#pragma once
#ifndef UTIL_RANDOMNUMBERTABLE_H
#define UTIL_RANDOMNUMNERTABLE_H
#include "core/types.h"
namespace Util
{
class RandomNumberTable
{
public:
    static float Rand(IndexT key);
    static float Rand(IndexT key, float minVal, float maxVal);
private:
    static const SizeT tableSize = 2048;
    static const float randTable[tableSize];
};
inline float
RandomNumberTable::Rand(IndexT key)
{
    return randTable[key % tableSize];
}
inline float
RandomNumberTable::Rand(IndexT key, float minVal, float maxVal)
{
    return minVal + (randTable[key % tableSize] * (maxVal - minVal));
}
}
#endif