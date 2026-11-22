#pragma once
#ifndef POSIX_POSIXGUID_H
#define POSIX_POSIXGUID_H
#include "core/types.h"
#include "util/string.h"
#include <uuid/uuid.h>
namespace Posix
{
class PosixGuid
{
public:
    PosixGuid();
    PosixGuid(const PosixGuid& rhs);
    PosixGuid(const unsigned char* ptr, SizeT size);
    void operator=(const PosixGuid& rhs);
    void operator=(const Util::String& rhs);
    bool operator==(const PosixGuid& rhs) const;
    bool operator!=(const PosixGuid& rhs) const;
    bool operator<(const PosixGuid& rhs) const;
    bool operator<=(const PosixGuid& rhs) const;
    bool operator>(const PosixGuid& rhs) const;
    bool operator>=(const PosixGuid& rhs) const;
    bool IsValid() const;
    void Generate();
    static PosixGuid FromString(const Util::String& str);
    static PosixGuid FromBinary(const unsigned char* ptr, SizeT numBytes);
    Util::String AsString() const;
    SizeT AsBinary(const unsigned char*& outPtr) const;
    IndexT HashCode() const;
private:
    uuid_t uuid;
};
inline
PosixGuid::PosixGuid()
{
    uuid_clear(this->uuid);
}
inline
PosixGuid::PosixGuid(const PosixGuid& rhs)
{
    uuid_copy(this->uuid, rhs.uuid);
}
inline
PosixGuid::PosixGuid(const unsigned char* ptr, SizeT size)
{
    n_assert((0 != ptr) && (size == sizeof(uuid_t)));
    Memory::Copy(ptr, &this->uuid, sizeof(uuid_t));
}
};
#endif