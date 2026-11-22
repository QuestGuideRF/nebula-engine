#pragma once
#include "core/types.h"
#if !__NEBULA_JOB__
#include "util/string.h"
#endif
namespace CoreAnimation
{
class SampleType
{
public:
    enum Code
    {
        Step,
        Linear,
        Hermite,
        CatmullRom,
        NumSampleTypes,
        InvalidSampleType,
    };
    #if !__NEBULA_JOB__
    static Code FromString(const Util::String& str);
    static Util::String ToString(Code c);
    #endif
};
}