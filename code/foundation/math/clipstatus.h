#pragma once
#include "core/types.h"
namespace Math
{
class ClipStatus
{
public:
    enum Type
    {
        Inside = 0,
        Outside = 1,
        Clipped = 2,
        Invalid,
    };
};
}