#pragma once
#include "util/string.h"
namespace Presentation
{
namespace Dialogs
{
enum FileResult
{
    NOINPUT = -1,
    CANCEL = 0,
    OKAY = 1,
    ERR = 2,
};
FileResult OpenFileDialog(const Util::String& title, Util::String& outpath, const char* pattern, bool& open);
}
}