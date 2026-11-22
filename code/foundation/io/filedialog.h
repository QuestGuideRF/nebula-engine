#pragma once
#include "core/types.h"
#include "util/string.h"
#include "io/uri.h"
namespace IO
{
namespace FileDialog
{
    bool OpenFile(Util::String const& title, const IO::URI & start, std::initializer_list<const char*>const& filters, Util::String& path);
    bool OpenFolder(Util::String const& title, const IO::URI & start, Util::String& path);
    bool SaveFile(Util::String const& title, const IO::URI & start, std::initializer_list<const char*>const& filters, Util::String& path);
}
}