#pragma once
#include "core/types.h"
#if !__NEBULA_JOB__
#include "util/string.h"
#endif
namespace CoreAnimation
{
class CurveType
{
public:
    enum Code
    {
        Translation,
        Rotation,
        Scale,
        Color,
        Velocity,
        Float4,
        NumCurveTypes,
        InvalidCurveType,
    };
    #if !__NEBULA_JOB__
    static Code FromString(const Util::String& str);
    static Util::String ToString(Code c);
    #endif
};
}