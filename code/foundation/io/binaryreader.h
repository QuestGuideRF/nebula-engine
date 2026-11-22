#pragma once
#include "io/streamreader.h"
#if !__OSX__
#include "math/vec2.h"
#include "math/vec4.h"
#include "math/vec3.h"
#include "math/mat4.h"
#endif
#include "util/guid.h"
#include "util/blob.h"
#include "system/byteorder.h"
namespace IO
{
class BinaryReader : public StreamReader
{
    __DeclareClass(BinaryReader);
public:
    BinaryReader();
    virtual ~BinaryReader();
    void SetMemoryMappingEnabled(bool b);
    bool IsMemoryMappingEnabled() const;
    void SetStreamByteOrder(System::ByteOrder::Type byteOrder);
    System::ByteOrder::Type GetStreamByteOrder() const;
    virtual bool Open();
    virtual void Close();
    char ReadChar();
    unsigned char ReadUChar();
    short ReadShort();
    unsigned short ReadUShort();
    int ReadInt();
    unsigned int ReadUInt();
    long long ReadInt64();
    unsigned long long ReadUInt64();
    float ReadFloat();
    float ReadFloatFromNormalizedUByte2();
    float ReadFloatFromUnsignedNormalizedUByte2();
    double ReadDouble();
    bool ReadBool();
    Util::String ReadString();
    #if !__OSX__
    Math::vec2 ReadFloat2();
    Math::vec3 ReadVec3();
    Math::vec4 ReadVec4();
    Math::mat4 ReadMat4();
    #endif
    Util::Array<float> ReadFloatArray();
    Util::Array<int> ReadIntArray();
    Util::Array<uint> ReadUIntArray();
    Util::Array<bool> ReadBoolArray();
    Util::Guid ReadGuid();
    Util::Blob ReadBlob();
    void ReadRawData(void* ptr, SizeT numBytes);
public:
    System::ByteOrder byteOrder;
    unsigned char* mapCursor;
    unsigned char* mapEnd;
    bool enableMapping;
    bool isMapped;
};
inline void
BinaryReader::SetMemoryMappingEnabled(bool b)
{
    this->enableMapping = b;
}
inline bool
BinaryReader::IsMemoryMappingEnabled() const
{
    return this->enableMapping;
}
inline void
BinaryReader::SetStreamByteOrder(System::ByteOrder::Type order)
{
    this->byteOrder.SetFromByteOrder(order);
}
inline System::ByteOrder::Type
BinaryReader::GetStreamByteOrder() const
{
    return this->byteOrder.GetFromByteOrder();
}
}