#pragma once
#include "core/types.h"
#if !__NEBULA_JOB__
#include "util/string.h"
#endif
namespace CoreAnimation
{
class InfinityType
{
public:
    enum Code
    {
        Constant,
        Cycle,
        NumInfinityTypes,
        InvalidInfinityType,
    };
    #if !__NEBULA_JOB__
    static Code FromString(const Util::String& str);
    static Util::String ToString(Code c);
    #endif
};
}