#pragma once
#include "core/types.h"
#include "util/string.h"
#include "core/sysfunc.h"
namespace Win32
{
class Win32Guid
{
public:
    Win32Guid();
    Win32Guid(const Win32Guid& rhs);
    Win32Guid(const unsigned char* ptr, SizeT size);
    void operator=(const Win32Guid& rhs);
    void operator=(const Util::String& rhs);
    bool operator==(const Win32Guid& rhs) const;
    bool operator!=(const Win32Guid& rhs) const;
    bool operator<(const Win32Guid& rhs) const;
    bool operator<=(const Win32Guid& rhs) const;
    bool operator>(const Win32Guid& rhs) const;
    bool operator>=(const Win32Guid& rhs) const;
    bool IsValid() const;
    void Generate();
    static Win32Guid FromString(const Util::String& str);
    static Win32Guid FromBinary(const unsigned char* ptr, SizeT numBytes);
    Util::String AsString() const;
    SizeT AsBinary(const unsigned char*& outPtr) const;
    static SizeT BinarySize();
    uint32_t HashCode() const;
private:
    UUID uuid;
};
inline
Win32Guid::Win32Guid()
{
    Memory::Clear(&(this->uuid), sizeof(this->uuid));
}
inline
Win32Guid::Win32Guid(const Win32Guid& rhs)
{
    this->uuid = rhs.uuid;
}
inline
Win32Guid::Win32Guid(const unsigned char* ptr, SizeT size)
{
    n_assert((0 != ptr) && (size == sizeof(UUID)));
    Memory::Copy(ptr, &this->uuid, sizeof(UUID));
}
inline SizeT
Win32Guid::BinarySize()
{
    return sizeof(UUID);
}
}