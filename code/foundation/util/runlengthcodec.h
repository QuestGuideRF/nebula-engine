#pragma once
#include "core/types.h"
namespace Util
{
class RunLengthCodec
{
public:
    static SizeT GetSafeRLEBufferSize(SizeT srcBufferSize);
    static SizeT ComputeDecodedSize(const uchar* srcPtr, SizeT srcNumBytes);
    static SizeT Encode(const uchar* srcPtr, SizeT srcNumBytes, uchar* dstPtr, SizeT dstNumBytes);
    static SizeT Decode(const uchar* srcPtr, SizeT srcNumBytes, uchar* dstPtr, SizeT dstNumBytes);
};
}