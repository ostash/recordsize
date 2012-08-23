#include "FieldInfo.h"

#include <defaults.h>

FieldInfo::FieldInfo(const tree& field)
  : m_size(0)
{
  // Bit-fields aren't supported
  if (DECL_BIT_FIELD(field))
    return;
  // Can't understand how this can happen and what it means
  if (!DECL_SIZE(field) || !DECL_FIELD_OFFSET(field) || !DECL_FIELD_BIT_OFFSET(field))
    return;

  m_isBase = DECL_ARTIFICIAL(field);
  m_name = m_isBase ? "base class" : DECL_NAME(field) ? IDENTIFIER_POINTER(DECL_NAME(field)) : "unnamed";
  m_size = TREE_INT_CST_LOW(DECL_SIZE(field)) / BITS_PER_UNIT;
  m_offset = TREE_INT_CST_LOW(DECL_FIELD_OFFSET(field)) + TREE_INT_CST_LOW(DECL_FIELD_BIT_OFFSET(field)) / BITS_PER_UNIT;
  m_align = DECL_ALIGN(field) / BITS_PER_UNIT;
}
