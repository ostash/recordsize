#ifndef FIELDINFO_H
#define FIELDINDO_H

#include <gcc-plugin.h>
#include <tree.h>

#include <string>

class FieldInfo
{
public:
    FieldInfo(const tree& field);
    const std::string& name() const { return m_name; }
    size_t size() const { return m_size; }
    size_t offset() const { return m_offset; }
    size_t align() const { return m_align; }
    bool isBase() const { return m_isBase; }
private:
    std::string m_name;
    size_t m_size;
    size_t m_offset;
    size_t m_align;
    bool m_isBase;
};

#endif
