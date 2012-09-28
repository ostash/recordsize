#ifndef FIELDINFO_H
#define FIELDINDO_H

#include <gcc-plugin.h>
#include <tree.h>

struct FieldInfo
{
  char* name;
  size_t size;
  size_t offset;
  size_t align;
  bool isBase;
  bool isBitField;
};

struct FieldInfo* createFieldInfo(const tree field);
void deleteFieldInfo(struct FieldInfo* fi);

#endif
