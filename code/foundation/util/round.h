#pragma once
#include "core/types.h"
namespace Util
{
class Round
{
public:
    static uint RoundUp2(uint val);
    static uint RoundUp4(uint val);
    static uint RoundUp8(uint val);
    static uint RoundUp16(uint val);
    static uint RoundUp32(uint val);
    static uint RoundUp(uint val, uint boundary);
};
__forceinline uint
Round::RoundUp2(uint val)
{
    return ((uint)val + (2 - 1)) & ~(2 - 1);
}
__forceinline uint
Round::RoundUp4(uint val)
{
    return ((uint)val + (4 - 1)) & ~(4 - 1);
}
__forceinline uint
Round::RoundUp8(uint val)
{
    return ((uint)val + (8 - 1)) & ~(8 - 1);
}
__forceinline uint
Round::RoundUp16(uint val)
{
    return ((uint)val + (16 - 1)) & ~(16 - 1);
}
__forceinline uint
Round::RoundUp32(uint val)
{
    return ((uint)val + (32 - 1)) & ~(32 - 1);
}
__forceinline uint
Round::RoundUp(uint val, uint boundary)
{
    n_assert(0 != boundary);
    return (((val - 1) / boundary) + 1) * boundary;
}
}