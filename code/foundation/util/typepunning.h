#pragma once
#include "core/types.h"
namespace Util
{
template<typename A, typename B>
A& TypePunning(B &v)
{
    union
    {
        A *a;
        B *b;
    } pun;
    pun.b = &v;
    return *pun.a;
}
template<typename A, typename B>
const A& TypePunning(const B &v)
{
    union
    {
        const A *a;
        const B *b;
    } pun;
    pun.b = &v;
    return *pun.a;
}
}