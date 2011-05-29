#include <gmp.h>

#include <gcc-plugin.h>
#include <tree.h>
#include <diagnostic.h>
#include <toplev.h>
#include <defaults.h>
#include <langhooks.h>

// Print messages about nodes we don't know how to handel
static bool flag_print_unknown = 1;
// Print information about all types, even with optimal size
static bool flag_print_all = 0;
// Print records layout
static bool flag_print_layout = 1;
// Process all types, not only from main translation unit
static bool flag_process_all = 0;

int plugin_is_GPL_compatible;
static struct plugin_info recordsize_plugin_info = { "0.1", "Record size plugin" };

static void print_unknown_node(const tree node, const char* msg)
{
    int tc = TREE_CODE(node);
    fprintf(stderr, "%s; node class name: %s, code name: %s", msg, TREE_CODE_CLASS_STRING(TREE_CODE_CLASS(tc)), tree_code_name[tc]);
}

struct FieldInfo
{
    const char* name;
    size_t size;
    size_t offset;
    size_t align;
    bool isBase;
};

static bool getFieldInfo(const tree field, struct FieldInfo* fi)
{
    // Can't understand how this can happen and what it means
    if (!DECL_SIZE(field) || !DECL_FIELD_OFFSET(field) || !DECL_FIELD_BIT_OFFSET(field)) return false;

    fi->isBase = DECL_ARTIFICIAL(field);
    fi->name = fi->isBase ? "base class" : DECL_NAME(field) ? IDENTIFIER_POINTER(DECL_NAME(field)) : "unnamed";
    fi->size = TREE_INT_CST_LOW(DECL_SIZE(field)) / BITS_PER_UNIT;
    fi->offset = TREE_INT_CST_LOW(DECL_FIELD_OFFSET(field)) + TREE_INT_CST_LOW(DECL_FIELD_BIT_OFFSET(field)) / BITS_PER_UNIT;
    fi->align = DECL_ALIGN(field) / BITS_PER_UNIT;
    return true;
}

static void recordsize_finish_type(void *gcc_data, void *plugin_data)
{
    tree record_type = (tree)gcc_data;
    // Check whether we really get record
    switch (TREE_CODE(record_type))
    {
	case RECORD_TYPE:
	    break;
	case UNION_TYPE:
	case ERROR_MARK:
	    return;
	default:
	    if (flag_print_unknown) print_unknown_node(record_type, "Don't know how to handle such node");
	    return;
    };
    // Check whether this record has TYPE_NAME
    tree type_decl = TYPE_NAME(record_type);
    if (TREE_CODE(type_decl) != TYPE_DECL)
    {
	if (flag_print_unknown) print_unknown_node(type_decl, "Don't know how to handle record_type with such name node");
    }

    // Ignoring types not from main translation unit
    if (!flag_process_all && !MAIN_FILE_P(linemap_lookup(line_table, DECL_SOURCE_LOCATION(type_decl)))) return;

    int lastBaseIdx = -1;
    int maxAlignFieldIdx = -1;
    // Up to 128-byte alignment
    size_t sizes[7] = {0, 0, 0, 0, 0, 0, 0};
    size_t maxFields = 64;
    size_t fieldCount = 0;
    struct FieldInfo* fields = (struct FieldInfo*) xmalloc(maxFields * sizeof(struct FieldInfo));

    for (tree field = TYPE_FIELDS(record_type); field; field = TREE_CHAIN(field))
    {
	switch(TREE_CODE(field))
	{
	case FIELD_DECL:
	    // Ingoring records with bit-fields
	    if (DECL_BIT_FIELD(field) || !getFieldInfo(field, &fields[fieldCount])) { free(fields); return; }

	    if (fields[fieldCount].isBase && (lastBaseIdx == -1 || fields[fieldCount].offset >= fields[lastBaseIdx].offset))
		lastBaseIdx = fieldCount;
	    else
	    {
		sizes[exact_log2(fields[fieldCount].align)] += fields[fieldCount].size;
		if (maxAlignFieldIdx == -1 || fields[fieldCount].align > fields[maxAlignFieldIdx].align)
		    maxAlignFieldIdx = fieldCount;
	    }
	    fieldCount++;
	    if (fieldCount == maxFields)
	    {
		maxFields *= 2;
		fields = xrealloc(fields, maxFields);
	    }
	    break;
	case VAR_DECL:
	case CONST_DECL:
	case TYPE_DECL:
	case TEMPLATE_DECL:
	case USING_DECL:
	    break;
	default:
	    if (flag_print_unknown) print_unknown_node(field, "Don't know how to handle field with such name node");
	    free(fields);
	    return;
	}
    }

    if (fieldCount == 0) { free(fields); return; }

    size_t minFieldsSize = 0;
    for (size_t i = 0; i < sizeof(sizes)/sizeof(size_t); ++i) minFieldsSize += sizes[i];

    size_t recordAlign = TYPE_ALIGN(record_type) / BITS_PER_UNIT;
    if (minFieldsSize % recordAlign)
	minFieldsSize = (minFieldsSize / recordAlign + 1) * recordAlign;

    size_t endOfBases = 0;
    if (lastBaseIdx != -1)
    {
	endOfBases = fields[lastBaseIdx].offset + (fields[lastBaseIdx].size / fields[lastBaseIdx].align) * fields[lastBaseIdx].align;
	if (fields[lastBaseIdx].size % fields[lastBaseIdx].align) endOfBases += fields[lastBaseIdx].align;
    }

    size_t prePadding = 0;
    if (maxAlignFieldIdx != -1)
    {
	size_t modulus = endOfBases % fields[maxAlignFieldIdx].align;
	if (modulus) prePadding = fields[maxAlignFieldIdx].align - modulus;
    }

    size_t recordSize = TREE_INT_CST_LOW(TYPE_SIZE(record_type)) / BITS_PER_UNIT;
    size_t recordSizeWOBases = recordSize - endOfBases - prePadding;

    if (minFieldsSize < recordSizeWOBases || flag_print_all)
    {
	if (minFieldsSize < recordSizeWOBases)
	    fprintf(stderr, "Warning: ");
	fprintf(stderr, "Record %s at %s:%d; current size %zu byte(s), minimal size %zu byte(s)\n", IDENTIFIER_POINTER(DECL_NAME(type_decl)),
	    DECL_SOURCE_FILE(type_decl), DECL_SOURCE_LINE(type_decl), recordSize, endOfBases + prePadding + minFieldsSize);

	if (flag_print_layout)
	{
	    fprintf(stderr, "# %-32s %-7s %-7s %-7s\n", "Name", "Offset", "Size", "Align");
	    for (size_t i = 0; i < fieldCount; ++i)
		fprintf(stderr, "%zu %-32s %7zu %7zu %7zu\n", i, fields[i].name, fields[i].offset, fields[i].size, fields[i].align);
	}
    }
    free(fields);
}

int plugin_init(struct plugin_name_args* info, struct plugin_gcc_version* ver)
{
    if (info->argc)
    {
	for (int i = 0; i < info->argc; ++i)
	{
	    if (strcmp(info->argv[i].key, "print-unknown") == 0)
		flag_print_unknown = 1;
	    if (strcmp(info->argv[i].key, "print-all") == 0)
		flag_print_all = 1;
	    if (strcmp(info->argv[i].key, "print-layout") == 0)
		flag_print_layout = 1;
	    if (strcmp(info->argv[i].key, "process-all") == 0)
		flag_process_all = 1;
	}
    }

    if (strcmp(lang_hooks.name, "GNU C++") != 0)
    {
	fprintf(stderr, "Plugin supports only GNU C++ frontend");
	return 1;
    }

    register_callback(info->base_name, PLUGIN_INFO, NULL, &recordsize_plugin_info);
    register_callback(info->base_name, PLUGIN_FINISH_TYPE, &recordsize_finish_type, NULL);

    return 0;
}
