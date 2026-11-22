#pragma once
#include "ids/id.h"
#include "util/fourcc.h"
#include "util/stringatom.h"
#include "util/hashtable.h"
#include "table.h"
#include "attributeid.h"
namespace MemDb
{
class Attribute
{
public:
    template<typename T>
    explicit Attribute(Util::StringAtom name, T const& defaultValue, uint32_t flags);
    explicit Attribute(Util::StringAtom name, SizeT typeSizeBytes, void const* defaultValue, uint32_t flags);
    Attribute() = default;
    Attribute(Attribute&& desc) noexcept;
    ~Attribute();
    void operator=(Attribute&& rhs) noexcept;
    void operator=(Attribute& rhs);
    Util::StringAtom name;
    SizeT typeSize = 0;
    void* defVal = nullptr;
    uint32_t externalFlags;
};
template<typename T> inline
Attribute::Attribute(Util::StringAtom name, T const& defaultValue, uint32_t flags) :
    name(name),
    typeSize(sizeof(T)),
    externalFlags(flags)
{
    this->defVal = Memory::Alloc(Memory::HeapType::ObjectHeap, sizeof(T));
    Memory::Copy(&defaultValue, this->defVal, sizeof(T));
}
inline
Attribute::Attribute(Util::StringAtom name, SizeT typeSizeBytes, void const* defaultValue, uint32_t flags) :
    name(name),
    typeSize(typeSizeBytes),
    externalFlags(flags)
{
    if (typeSizeBytes > 0)
    {
        this->defVal = Memory::Alloc(Memory::HeapType::ObjectHeap, typeSizeBytes);
        if (defaultValue != nullptr)
        {
            Memory::Copy(defaultValue, this->defVal, typeSizeBytes);
        }
        else
        {
            Memory::Clear(this->defVal, typeSizeBytes);
        }
    }
}
inline
Attribute::Attribute(Attribute&& desc) noexcept :
    name(desc.name), typeSize(desc.typeSize), defVal(desc.defVal)
{
    desc.defVal = nullptr;
    desc.name = nullptr;
    desc.typeSize = 0;
}
inline
Attribute::~Attribute()
{
    if (this->defVal != nullptr)
    {
        Memory::Free(Memory::HeapType::ObjectHeap, this->defVal);
    }
}
inline void
Attribute::operator=(Attribute&& rhs) noexcept
{
    this->typeSize = rhs.typeSize;
    this->defVal = rhs.defVal;
    this->name = rhs.name;
    rhs.typeSize = 0;
    rhs.defVal = nullptr;
    rhs.name = nullptr;
}
inline void
Attribute::operator=(Attribute& rhs)
{
    this->typeSize = rhs.typeSize;
    this->defVal = rhs.defVal;
    this->name = rhs.name;
    rhs.typeSize = 0;
    rhs.defVal = nullptr;
    rhs.name = nullptr;
}
}