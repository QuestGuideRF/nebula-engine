#pragma once
#include "core/types.h"
#include "util/string.h"
#include "system/byteorder.h"
namespace ToolkitUtil
{
class Platform
{
public:
    enum Code
    {
        Win32,
        Linux,
        InvalidPlatform,
    };
    static System::ByteOrder::Type GetPlatformByteOrder(Code c);
    static Code FromString(const Util::String& str);
    static Util::String ToString(Code c);
};
}