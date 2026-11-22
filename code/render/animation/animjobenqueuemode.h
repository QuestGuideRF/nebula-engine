#pragma once
#include "core/types.h"
namespace Animation
{
class AnimJobEnqueueMode
{
public:
    enum Code
    {
        Append = 0,
        Intercept = 1,
        IgnoreIfSameClipActive = 2,
        IgnoreIfSameExclTagActive = 4,
    };
    static const char* ToString(Code c);
    static Code FromString(const char* s);
};
inline AnimJobEnqueueMode::Code
AnimJobEnqueueMode::FromString(const char* s)
{
    n_assert(0 != s);
    if (0 == Util::String::StrCmp(s, "Append")) return Append;
    else if (0 == Util::String::StrCmp(s, "Intercept")) return Intercept;
    else if (0 == Util::String::StrCmp(s, "IgnoreIfSameClipActive")) return IgnoreIfSameClipActive;
    else if (0 == Util::String::StrCmp(s, "IgnoreIfSameExclTagActive")) return IgnoreIfSameExclTagActive;
    else
    {
        n_error("AnimJobEnqueueMode: неверная строка режима '%s'!\n", s);
    }
    return Intercept;
}
inline const char*
AnimJobEnqueueMode::ToString(Code c)
{
    switch (c)
    {
        case Append: return "Append";
        case Intercept: return "Intercept";
        case IgnoreIfSameClipActive: return "IgnoreIfSameClipActive";
        case IgnoreIfSameExclTagActive: return "IgnoreIfSameExclTagActive";
        default:
            n_error("AnimJobEnqueueMode: неверный режим «%d»!\n", c);
            break;
    }
    return 0;
}
}