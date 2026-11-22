#pragma once
#include "core/types.h"
#include "util/string.h"
namespace CoreGraphics
{
class Adapter
{
public:
    enum Code
    {
        Primary = 0,
        Secondary,
        None,
    };
    static Code FromString(const Util::String& str);
    static Util::String ToString(Code code);
};
}