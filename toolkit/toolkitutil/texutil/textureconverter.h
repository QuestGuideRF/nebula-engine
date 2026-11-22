#pragma once
#include "toolkit-common/platform.h"
#include "io/uri.h"
#include "util/string.h"
#include "toolkitutil/texutil/textureattrtable.h"
#include "toolkit-common/applauncher.h"
#include "toolkit-common/logger.h"
namespace ToolkitUtil
{
class TextureConverter
{
public:
    TextureConverter();
    ~TextureConverter();
    void SetPlatform(Platform::Code platform);
    void SetTexAttrTablePath(const Util::String& path);
    void SetSrcDir(const Util::String& srcDir);
    const Util::String& GetSrcDir() const;
    void SetDstDir(const Util::String& dstDir);
    const Util::String& GetDstDir() const;
    Util::String& GetDstDir();
    void SetToolPath(const Util::String& toolPath);
    void SetForceFlag(bool b);
    void SetQuietFlag(bool b);
    void SetTextureAttrTable(TextureAttrTable&& extTexAttrTable);
    void SetMaxParallelJobs(int count);
    int GetMaxParallelJobs();
    void AddAttributeEntry(const Util::String& file, const TextureAttrs& attrs);
    bool Setup();
    void SetLogger(Logger* logger);
    void Discard();
    bool IsValid() const;
    bool ConvertFiles(const Util::Array<Util::String>& files);
    bool ConvertTexture(const Util::String& srcTexPath, const Util::String& dstTexPath, const Util::String& tmpDir);
    bool ConvertCubemap(const Util::String& srcTexPath, const Util::String& dstTexPath, const Util::String& tmpDir);
private:
    Logger* logger;
    Platform::Code platform;
    Util::String texAttrTablePath;
    Util::String toolPath;
    bool force;
    bool quiet;
    bool valid;
    TextureAttrTable textureAttrTable;
    int maxParallelJobs;
};
inline void
TextureConverter::SetTextureAttrTable(TextureAttrTable&& extTexAttrTable)
{
    this->textureAttrTable = extTexAttrTable;
}
inline bool
TextureConverter::IsValid() const
{
    return this->valid;
}
inline void
TextureConverter::SetPlatform(Platform::Code p)
{
    this->platform = p;
}
inline void
TextureConverter::SetTexAttrTablePath(const Util::String& path)
{
    this->texAttrTablePath = path;
}
inline void
TextureConverter::SetToolPath(const Util::String& p)
{
    this->toolPath = p;
}
inline void
TextureConverter::SetForceFlag(bool b)
{
    this->force = b;
}
inline void
TextureConverter::SetQuietFlag(bool b)
{
    this->quiet = b;
}
inline void
TextureConverter::SetMaxParallelJobs(int count)
{
    n_assert(count > 0);
    this->maxParallelJobs = count;
}
inline int
TextureConverter::GetMaxParallelJobs()
{
    return this->maxParallelJobs;
}
}