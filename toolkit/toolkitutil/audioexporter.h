#pragma once
#include "toolkit-common/platform.h"
#include "util/string.h"
namespace ToolkitUtil
{
class AudioExporter
{
public:
    AudioExporter();
    ~AudioExporter();
    void SetPlatform(Platform::Code platform);
    void SetToolPath(const Util::String& toolPath);
    void SetForceFlag(bool b);
    void SetProjectFile(const Util::String& projectFile);
    void SetDstDir(const Util::String& dstDir);
    void SetSrcDir(const Util::String& srcDir);
    bool Export();
private:
    bool ExportXact();
    bool ExportWii();
    bool ExportFmod();
    void FindRspjFiles(Util::String folder);
    void CopyDirectory(Util::String src, Util::String dst);
    void FmodFindProjectFiles(const Util::String &folder, Util::Array<Util::String> &projectFiles, const Util::String & pattern) const;
    Platform::Code platform;
    Util::String toolPath;
    Util::String projectFile;
    Util::String dstDir;
    Util::String srcDir;
    Util::Array<Util::String> srcFolders;
    Util::Array<Util::String> rspjFiles;
    bool force;
};
inline void
AudioExporter::SetPlatform(Platform::Code p)
{
    this->platform = p;
}
inline void
AudioExporter::SetForceFlag(bool b)
{
    this->force = b;
}
inline void
AudioExporter::SetToolPath(const Util::String& p)
{
    this->toolPath = p;
}
inline void
AudioExporter::SetProjectFile(const Util::String& f)
{
    this->projectFile = f;
}
inline void
AudioExporter::SetDstDir(const Util::String& d)
{
    this->dstDir = d;
}
inline void
AudioExporter::SetSrcDir(const Util::String& s)
{
    this->srcDir = s;
}
}