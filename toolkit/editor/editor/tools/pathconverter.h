#pragma once
#include "util/string.h"
#include "io/uri.h"
namespace Editor
{
struct PathConverterCreateInfo
{
};
void CreatePathConverter(const PathConverterCreateInfo& info);
struct PathConverter
{
    static Util::String MapToCompactPath(const Util::String& path);
    static Util::String StripAssetName(const Util::String& path);
    static Util::String MapToWork(const Util::String& path);
};
}