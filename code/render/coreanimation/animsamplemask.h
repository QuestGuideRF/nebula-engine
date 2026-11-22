#pragma once
#include "util/fixedarray.h"
#include "util/stringatom.h"
namespace CoreAnimation
{
struct AnimSampleMask
{
    Util::StringAtom name;
    Util::FixedArray<Math::scalar> weights;
    bool operator==(const AnimSampleMask& rhs) const { return name == rhs.name;  };
};
}