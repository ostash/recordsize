#include "FieldInfo.h"

enum
{
  FIELD_BASE = 0,
  FIELD_NONAME
};
static const char* fieldNames[] = {"base class", "unnamed"};

struct FieldInfo* createFieldInfo(const tree field)
{
  struct FieldInfo* fi = (struct FieldInfo*) xcalloc(1, sizeof(struct FieldInfo));
  fi->size = 0;

  fi->isBase = DECL_ARTIFICIAL(field);
  fi->isBitField = DECL_BIT_FIELD(field);

  const char* fieldName;
  if (fi->isBase)
    fieldName = fieldNames[FIELD_BASE];
  else if (DECL_NAME(field))
    fieldName = IDENTIFIER_POINTER(DECL_NAME(field));
  else
    fieldName = fieldNames[FIELD_NONAME];

  fi->name = (char*)xmalloc(strlen(fieldName + 1));
  strcpy(fi->name, fieldName);

  fi->size = TREE_INT_CST_LOW(DECL_SIZE(field));
  fi->offset = TREE_INT_CST_LOW(DECL_FIELD_OFFSET(field)) + TREE_INT_CST_LOW(DECL_FIELD_BIT_OFFSET(field));
  fi->align = DECL_ALIGN(field);

  return fi;
}

void deleteFieldInfo(struct FieldInfo* fi)
{
  free(fi->name);
  free(fi);
}
