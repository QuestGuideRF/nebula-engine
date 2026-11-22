#pragma once
#include "util/string.h"
#include "toolkitutil/texutil/textureattrtable.h"
#include "toolkit-common/logger.h"
namespace ToolkitUtil
{
class TextureConversionJob
{
public:
    TextureConversionJob();
    void SetTexAttrTable(const TextureAttrTable* table);
    void SetLogger(Logger* logger);
    void SetToolPath(const Util::String & path);
    void SetForceFlag(bool b);
    void SetQuietFlag(bool b);
    void SetTmpDir(const Util::String& dir);
    void SetSrcPath(const Util::String& path);
    void SetDstPath(const Util::String& path);
    virtual bool Convert();
    bool CopyResult();
protected:
    virtual bool PrepareConversion(const Util::String& srcPath, const Util::String& dstPath);
    virtual bool NeedsConversion(const Util::String& srcPath, const Util::String& dstPath);
    void SetDstFileExtension(const Util::String & ext);
    const TextureAttrTable* textureAttrTable;
    Logger* logger;
    TextureAttrs textureAttrs;
    Util::String toolPath;
    Util::String dstFileExt;
    Util::String tmpDir;
    Util::String srcPath;
    Util::String dstPath;
    Util::String tmpPath;
    bool force;
    bool quiet;
    bool neverCopy;
};
inline void
TextureConversionJob::SetLogger(Logger* l)
{
    this->logger = l;
}
inline void
TextureConversionJob::SetTexAttrTable(const TextureAttrTable* table)
{
    this->textureAttrTable = table;
}
inline void
TextureConversionJob::SetToolPath(const Util::String & path)
{
    this->toolPath = path;
}
inline void
TextureConversionJob::SetForceFlag(bool b)
{
    this->force = b;
}
inline void
TextureConversionJob::SetQuietFlag(bool b)
{
    this->quiet = b;
}
inline void
TextureConversionJob::SetTmpDir(const Util::String & dir)
{
    this->tmpDir = dir;
}
inline void
TextureConversionJob::SetSrcPath(const Util::String& path)
{
    this->srcPath = path;
}
inline void
TextureConversionJob::SetDstPath(const Util::String& path)
{
    this->dstPath = path;
    this->dstPath.StripFileExtension();
    this->dstPath.Append(".");
    this->dstPath.Append(this->dstFileExt);
}
inline void
TextureConversionJob::SetDstFileExtension(const Util::String& ext)
{
    this->dstFileExt = ext;
}
}