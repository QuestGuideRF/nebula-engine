#pragma once
#include "core/types.h"
#include "util/string.h"
namespace Posix
{
class PosixEnvironment
{
public:
    static bool Exists(const Util::String& envVarName);
    static Util::String Read(const Util::String& envVarName);
};
}