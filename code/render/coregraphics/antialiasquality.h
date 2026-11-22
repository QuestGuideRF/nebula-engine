#pragma once
#include "core/types.h"
#include "util/string.h"
namespace CoreGraphics
{
class AntiAliasQuality
{
public:
    enum Code
    {
        None,
        Low,
        Medium,
        High,
    };
    static Code FromString(const Util::String& str);
    static Util::String ToString(Code code);
};
}