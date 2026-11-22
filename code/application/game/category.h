#pragma once
#include "util/string.h"
#include "util/fixedarray.h"
#include "component.h"
namespace Game
{
ID_16_TYPE(BlueprintId);
ID_32_TYPE(TemplateId);
struct EntityTableCreateInfo
{
    Util::String name;
    Util::FixedArray<ComponentId> components;
};
}