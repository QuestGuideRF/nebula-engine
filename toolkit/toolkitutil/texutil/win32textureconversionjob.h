#pragma once
#include "toolkitutil/texutil/textureconversionjob.h"
#include "toolkit-common/applauncher.h"
#include "io/uri.h"
namespace ToolkitUtil
{
class Win32TextureConversionJob : public TextureConversionJob
{
public:
    Win32TextureConversionJob();
    virtual bool Convert();
private:
    ToolkitUtil::AppLauncher appLauncher;
};
}