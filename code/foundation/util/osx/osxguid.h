#pragma once
#include "core/types.h"
#include "util/string.h"
#include "core/sysfunc.h"
namespace OSX
{
class OSXGuid
{
public:
    void* operator new(size_t s);
    void operator delete(void* ptr);
    OSXGuid();
    OSXGuid(const OSXGuid& rhs);
    OSXGuid(const unsigned char* ptr, SizeT size);
    void operator=(const OSXGuid& rhs);
    void operator=(const Util::String& rhs);
    bool operator==(const OSXGuid& rhs) const;
    bool operator!=(const OSXGuid& rhs) const;
    bool operator<(const OSXGuid& rhs) const;
    bool operator<=(const OSXGuid& rhs) const;
    bool operator>(const OSXGuid& rhs) const;
    bool operator>=(const OSXGuid& rhs) const;
    bool IsValid() const;
    void Generate();
    static OSXGuid FromString(const Util::String& str);
    static OSXGuid FromBinary(const unsigned char* ptr, SizeT numBytes);
    Util::String AsString() const;
    SizeT AsBinary(const unsigned char*& outPtr) const;
    static SizeT BinarySize();
    IndexT HashCode() const;
private:
    uuid_t uuid;
};
__forceinline void*
OSXGuid::operator new(size_t size)
{
    #if NEBULA_DEBUG
    n_assert(size == sizeof(OSXGuid));
    #endif
    return Memory::Alloc(Memory::ObjectHeap, size);
}
__forceinline void
OSXGuid::operator delete(void* ptr)
{
    return Memory::Free(Memory::ObjectHeap, ptr);
}
inline
OSXGuid::OSXGuid()
{
    uuid_clear(this->uuid);
}
inline
OSXGuid::OSXGuid(const OSXGuid& rhs)
{
    uuid_copy(this->uuid, rhs.uuid);
}
inline
OSXGuid::OSXGuid(const unsigned char* ptr, SizeT size)
{
    n_assert((0 != ptr) && (size == sizeof(uuid_t)));
    uuid_copy(this->uuid, *(uuid_t*)ptr);
}
inline SizeT
OSXGuid::BinarySize()
{
    return sizeof(uuid_t);
}
}