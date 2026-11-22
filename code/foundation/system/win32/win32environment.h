#pragma once
#include "core/types.h"
#include "util/string.h"
namespace Win32
{
class Win32Environment
{
public:
    static bool Exists(const Util::String& envVarName);
    static Util::String Read(const Util::String& envVarName);
};
}