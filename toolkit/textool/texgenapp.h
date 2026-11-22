#pragma once
#include "toolkit-common/toolkitapp.h"
namespace Toolkit
{
class TexGenApp : public ToolkitUtil::ToolkitApp
{
public:
    TexGenApp();
    virtual void Run() override;
    virtual bool ParseCmdLineArgs() override;
private:
    virtual void ShowHelp() override;
    Util::String outputFile;
    Util::String inputFile;
    Util::String outputBGRA[4];
    float BGRA[4];
    int outputWidth;
    int outputHeight;
    int outputChannels;
};
}