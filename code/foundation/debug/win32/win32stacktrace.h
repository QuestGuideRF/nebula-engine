#pragma once
#include "core/types.h"
#include "util/string.h"
namespace Win32
{
class Win32StackTrace
{
public:
    static const Util::Array<Util::String> GenerateStackTrace();
    static void PrintStackTrace(int skip = 3);
};
}