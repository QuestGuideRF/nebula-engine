#pragma once
#include "toolkitutil/texutil/textureconversionjob.h"
#include "toolkit-common/applauncher.h"
#include "io/uri.h"
namespace ToolkitUtil
{
class CompressonatorConversionJob : public TextureConversionJob
{
public:
    CompressonatorConversionJob();
    virtual bool Convert();
};
}