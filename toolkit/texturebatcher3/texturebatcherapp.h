#pragma once
#include "toolkit-common/distributedtools/distributedtoolkitapp.h"
#include "toolkitutil/texutil/textureconverter.h"
namespace Toolkit
{
class TextureBatcherApp : public DistributedTools::DistributedToolkitApp
{
public:
    virtual void DoWork();
private:
    virtual bool ParseCmdLineArgs();
    virtual bool SetupProjectInfo();
    virtual void ShowHelp();
    virtual Util::Array<Util::String> CreateFileList();
    Util::Array<Util::String> GetFileListFromDirectory(const Util::String& directory);
    Util::String GetDstTextureFileExtension();
    bool NeedsConversion(const Util::String& srcPath);
    bool NeedsConversion(const Util::String& srcPath, const Util::String& dstPath);
    ToolkitUtil::TextureConverter textureConverter;
    Util::String category;
    Util::String texture;
};
}