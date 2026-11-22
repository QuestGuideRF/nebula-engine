#pragma once
#include "util/string.h"
namespace Win32
{
class Win32StringConverter
{
public:
    static SizeT UTF8ToWide(const Util::String& src, ushort* dst, SizeT dstMaxBytes);
    static SizeT UTF8ToWide(const char* src, ushort* dst, SizeT dstMaxBytes);
    static Util::String WideToUTF8(const ushort* src, SizeT length = -1);
};
}