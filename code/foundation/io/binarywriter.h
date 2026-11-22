#pragma once
#include "io/streamwriter.h"
#include "util/blob.h"
#include "util/guid.h"
#include "system/byteorder.h"
#if !__OSX__
#include "math/vec2.h"
#endif
namespace IO
{
class BinaryWriter : public StreamWriter
{
    __DeclareClass(BinaryWriter);
public:
    BinaryWriter();
    virtual ~BinaryWriter();
    void SetMemoryMappingEnabled(bool b);
    bool IsMemoryMappingEnabled() const;
    void SetStreamByteOrder(System::ByteOrder::Type byteOrder);
    System::ByteOrder::Type GetStreamByteOrder() const;
    virtual bool Open();
    virtual void Close();
    void WriteChar(char c);
    void WriteUChar(unsigned char c);
    void WriteShort(short s);
    void WriteUShort(unsigned short s);
    void WriteInt(int i);
    void WriteUInt(unsigned int i);
    void WriteInt64(int64_t i);
    void WriteUInt64(uint64_t i);
    void WriteFloat(float f);
    void WriteFloatAsNormalizedUByte2(float f);
    void WriteFloatAsUnsignedNormalizedUByte2(float f);
    void WriteDouble(double d);
    void WriteBool(bool b);
    void WriteString(const Util::String& s);
    #if !__OSX__
    void WriteVec2(Math::vec2 f);
    void WriteVec3(const Math::vec3& v);
    void WriteVec4(const Math::vec4& v);
    void WriteMat4(const Math::mat4& m);
    #endif
    void WriteFloatArray(const Util::Array<float>& arr);
    void WriteIntArray(const Util::Array<int>& arr);
    void WriteUIntArray(const Util::Array<uint>& arr);
    void WriteBoolArray(const Util::Array<bool>& arr);
    void WriteGuid(const Util::Guid& guid);
    void WriteBlob(const Util::Blob& blob);
    void WriteRawData(const void* ptr, SizeT numBytes);
public:
    bool enableMapping;
    bool isMapped;
    System::ByteOrder byteOrder;
    unsigned char* mapCursor;
    unsigned char* mapEnd;
};
inline
void
BinaryWriter::SetMemoryMappingEnabled(bool b)
{
    this->enableMapping = b;
}
inline
bool
BinaryWriter::IsMemoryMappingEnabled() const
{
    return this->enableMapping;
}
inline void
BinaryWriter::SetStreamByteOrder(System::ByteOrder::Type order)
{
    this->byteOrder.SetToByteOrder(order);
}
inline System::ByteOrder::Type
BinaryWriter::GetStreamByteOrder() const
{
    return this->byteOrder.GetToByteOrder();
}
}