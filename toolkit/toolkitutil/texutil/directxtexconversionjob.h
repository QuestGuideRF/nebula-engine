#pragma once
#include "toolkitutil/texutil/textureconversionjob.h"
#include "toolkit-common/applauncher.h"
#include "io/uri.h"
namespace ToolkitUtil
{
class DirectXTexConversionJob : public TextureConversionJob
{
public:
    DirectXTexConversionJob();
    virtual bool Convert();
    virtual bool ConvertCube();
private:
    ToolkitUtil::AppLauncher appLauncher;
    Util::String cubeToolPath;
};
}