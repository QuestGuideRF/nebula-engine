#pragma once
#include "util/string.h"
#include "util/dictionary.h"
namespace ToolkitUtil
{
class UniqueString
{
public:
    UniqueString() = delete;
    static Util::String New(const Util::String& str);
    static Util::String New(const char* str);
    static void Reset();
private:
    static Util::Dictionary<IndexT, SizeT> Lookup;
};
}