#pragma once
#include "core/types.h"
#include "core/factory.h"
#include "core/sysfunc.h"
#include "util/string.h"
#include "util/fourcc.h"
namespace Core
{
class RefCounted;
class Rtti
{
public:
    typedef void* (*Creator)();
    typedef void* (*ArrayCreator)(SizeT);
    Rtti(const char* className, Util::FourCC fcc, Creator creatorFunc, ArrayCreator arrayCreatorFunc, const Core::Rtti* parentClass, SizeT instSize);
    Rtti(const char* className, Creator creatorFunc, ArrayCreator arrayCreatorFunc, const Core::Rtti* parentClass, SizeT instSize);
    bool operator==(const Rtti& rhs) const;
    bool operator!=(const Rtti& rhs) const;
    const Util::String& GetName() const;
    Util::FourCC GetFourCC() const;
    const Core::Rtti* GetParent() const;
    SizeT GetInstanceSize() const;
    void* Create() const;
    void* CreateArray(SizeT num) const;
    bool IsDerivedFrom(const Rtti& other) const;
    bool IsDerivedFrom(const Util::String& otherClassName) const;
    bool IsDerivedFrom(const Util::FourCC& otherClassFourCC) const;
    void* AllocInstanceMemory();
    void* AllocInstanceMemoryArray(size_t num);
    void FreeInstanceMemory(void* ptr);
private:
    void Construct(const char* className, Util::FourCC fcc, Creator creatorFunc, ArrayCreator arrayCreatorFunc, const Core::Rtti* parentClass, SizeT instSize);
    Util::String name;
    const Core::Rtti* parent;
    Util::FourCC fourCC;
    SizeT instanceSize;
    Creator creator;
    ArrayCreator arrayCreator;
};
inline bool
Rtti::operator==(const Rtti& rhs) const
{
    return this == &rhs;
}
inline bool
Rtti::operator!=(const Rtti& rhs) const
{
    return this != &rhs;
}
inline const Util::String&
Rtti::GetName() const
{
    return this->name;
}
inline Util::FourCC
Rtti::GetFourCC() const
{
    return this->fourCC;
}
inline const Core::Rtti*
Rtti::GetParent() const
{
    return this->parent;
}
inline SizeT
Rtti::GetInstanceSize() const
{
    return this->instanceSize;
}
#include "core/rttimacros.h"
}