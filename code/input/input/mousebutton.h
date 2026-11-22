#pragma once
#include "core/types.h"
#include "util/string.h"
namespace Input
{
class MouseButton
{
public:
    enum Code
    {
        LeftButton   = 0,
        RightButton  = 1,
        MiddleButton = 2,
        NumMouseButtons,
        InvalidMouseButton,
    };
    static Util::String ToString(Code code);
    static Code FromString(const Util::String& str);
};
}