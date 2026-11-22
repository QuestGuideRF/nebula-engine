#pragma once
#include "core/types.h"
namespace Input
{
class InputPriority
{
public:
    enum Code
    {
        DynUi = 0,
        Gui,
        Game,
        Other,
    };
};
}