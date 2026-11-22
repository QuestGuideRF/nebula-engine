//------------------------------------------------------------------------------
//  win32guid.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "util/win32/win32guid.h"

namespace Win32
{
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
void
Win32Guid::operator=(const Win32Guid& rhs)
{
    if (this != &rhs)
    {
        this->uuid = rhs.uuid;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
Win32Guid::operator=(const String& rhs)
{
    n_assert(rhs.IsValid());
    RPC_STATUS result = UuidFromString((RPC_CSTR) rhs.AsCharPtr(), &(this->uuid));
    n_assert(RPC_S_INVALID_STRING_UUID != result);
}

//------------------------------------------------------------------------------
/**
*/
bool
Win32Guid::operator==(const Win32Guid& rhs) const
{
    RPC_STATUS status;
    int result = UuidCompare(const_cast<UUID*>(&this->uuid), const_cast<UUID*>(&rhs.uuid), &status);
    return (0 == result);
}

//------------------------------------------------------------------------------
/**
*/
bool
Win32Guid::operator!=(const Win32Guid& rhs) const
{
    RPC_STATUS status;
    int result = UuidCompare(const_cast<UUID*>(&this->uuid), const_cast<UUID*>(&rhs.uuid), &status);
    return (0 != result);
}

//------------------------------------------------------------------------------
/**
*/
bool
Win32Guid::operator<(const Win32Guid& rhs) const
{
    RPC_STATUS status;
    int result = UuidCompare(const_cast<UUID*>(&this->uuid), const_cast<UUID*>(&rhs.uuid), &status);
    return (-1 == result);
}

//------------------------------------------------------------------------------
/**
*/
bool
Win32Guid::operator<=(const Win32Guid& rhs) const
{
    RPC_STATUS status;
    int result = UuidCompare(const_cast<UUID*>(&this->uuid), const_cast<UUID*>(&rhs.uuid), &status);
    return ((-1 == result) || (0 == result));
}

//------------------------------------------------------------------------------
/**
*/
bool
Win32Guid::operator>(const Win32Guid& rhs) const
{
    RPC_STATUS status;
    int result = UuidCompare(const_cast<UUID*>(&this->uuid), const_cast<UUID*>(&rhs.uuid), &status);
    return (1 == result);
}

//------------------------------------------------------------------------------
/**
*/
bool
Win32Guid::operator>=(const Win32Guid& rhs) const
{
    RPC_STATUS status;
    int result = UuidCompare(const_cast<UUID*>(&this->uuid), const_cast<UUID*>(&rhs.uuid), &status);
    return ((1 == result) || (0 == result));
}

//------------------------------------------------------------------------------
/**
*/
bool
Win32Guid::IsValid() const
{
    RPC_STATUS status;
    int result = UuidIsNil(const_cast<UUID*>(&this->uuid), &status);
    return (TRUE != result);
}

//------------------------------------------------------------------------------
/**
*/
void
Win32Guid::Generate()
{
    RPC_STATUS status = UuidCreate(&this->uuid);
    n_assert(status == RPC_S_OK);
}

//------------------------------------------------------------------------------
/**
*/
String
Win32Guid::AsString() const
{
    const char* uuidStr;
    RPC_STATUS status = UuidToString((UUID*)&this->uuid, (RPC_CSTR*)&uuidStr);
    n_assert(status == RPC_S_OK);
    String result = uuidStr;
    RpcStringFree((RPC_CSTR*)&uuidStr);
    return result;
}

//------------------------------------------------------------------------------
/**
    Этот метод обеспечивает доступ для чтения к необработанным двоичным данным uuid.
    Он возвращает количество байтов в буфере и указатель на
    данные.
*/
SizeT
Win32Guid::AsBinary(const unsigned char*& outPtr) const
{
    outPtr = (const unsigned char*) &this->uuid;
    return sizeof(UUID);
}

//------------------------------------------------------------------------------
/**
*/
Win32Guid
Win32Guid::FromString(const Util::String& str)
{
    Win32Guid newGuid;
    RPC_STATUS success = UuidFromString((RPC_CSTR)str.AsCharPtr(), &(newGuid.uuid));
    n_assert(RPC_S_OK == success);
    return newGuid;
}

//------------------------------------------------------------------------------
/**
    Constructs the guid from binary data, as returned by the AsBinary().
*/
Win32Guid
Win32Guid::FromBinary(const unsigned char* ptr, SizeT numBytes)
{
    n_assert((0 != ptr) && (numBytes == sizeof(UUID)));
    Win32Guid newGuid;
    Memory::Copy(ptr, &(newGuid.uuid), sizeof(UUID));
    return newGuid;
}

//------------------------------------------------------------------------------
/**
    Этот метод возвращает хеш-код для uuid, совместимый с 
    Утил::ХэшТабле.
*/
uint32_t
Win32Guid::HashCode() const
{
    RPC_STATUS status;
    unsigned short hashCode = UuidHash((UUID*)&this->uuid, &status);
    return (uint32_t) hashCode;
}

}; // пространство имен Win32
