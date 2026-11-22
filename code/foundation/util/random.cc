//------------------------------------------------------------------------------
//  random.cc
//  (C) 2018-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "random.h"

namespace Util
{

union RandomUnion
{
    unsigned int i;
    float f;
};

//------------------------------------------------------------------------------
/**
    Реализация XorShift128.
*/
uint
FastRandom()
{
    // Они предопределены, чтобы дать нам самый большой
    // возможная последовательность случайных чисел
    static uint x = 123456789;
    static uint y = 362436069;
    static uint z = 521288629;
    static uint w = 88675123;
    uint t;
    t = x ^ (x << 11);
    x = y;
    y = z;
    z = w;
    return w = w ^ (w >> 19) ^ (t ^ (t >> 8));
}

//------------------------------------------------------------------------------
/**
    Thanks to Nic Werneck (https://xor0110.wordpress.com/2010/09/24/how-to-generate-floating-point-random-numbers-efficiently/)
*/
float
RandomFloat()
{
    static RandomUnion r;
    r.i = (FastRandom() & 0x007fffff) | 0x3f800000;
    return r.f - 1.0f;
}

//------------------------------------------------------------------------------
/**
*/
float
RandomFloatNTP()
{
    static RandomUnion r;
    r.i = (FastRandom() & 0x007fffff) | 0x40000000;
    return r.f - 3.0f;
}

} // пространство имен Util



