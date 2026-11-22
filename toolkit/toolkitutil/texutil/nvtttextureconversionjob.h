#pragma once
#include "toolkitutil/texutil/textureconversionjob.h"
#include "toolkit-common/applauncher.h"
#include "io/uri.h"
namespace ToolkitUtil
{
class NVTTTextureConversionJob : public TextureConversionJob
{
public:
    NVTTTextureConversionJob();
    virtual bool Convert();
};
}