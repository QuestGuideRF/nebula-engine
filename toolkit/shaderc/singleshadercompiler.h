#pragma once
#include "toolkit-common/platform.h"
#include "io/uri.h"
#include "util/string.h"
namespace ToolkitUtil
{
class SingleShaderCompiler
{
public:
    SingleShaderCompiler();
    ~SingleShaderCompiler();
    void SetPlatform(Platform::Code platform);
    void SetLanguage(const Util::String& lang);
    void AddIncludeDir(const Util::String& incDir);
    void SetDstBinary(const Util::String& dstBinary);
    void SetDstHeader(const Util::String& dstHeader);
    void SetDebugFlag(bool b);
    void SetAdditionalParams(const Util::String& params);
    void SetQuietFlag(bool b);
    bool CompileShader(const Util::String& src);
    bool CompileMaterial(const Util::String& src);
    bool CompileFrameShader(const Util::String& src);
    bool CreateDependencies(const Util::String& src);
private:
    bool CompileGLSL(const Util::String& src);
    bool CompileSPIRV(const Util::String& src);
    Platform::Code platform;
    Util::String dstBinary;
    Util::String dstHeader;
    Util::String language;
    bool quiet;
    bool debug;
    Util::String additionalParams;
    Util::Array<Util::String> includeDirs;
};
inline void
SingleShaderCompiler::SetPlatform(Platform::Code p)
{
    this->platform = p;
}
inline void
SingleShaderCompiler::SetLanguage( const Util::String& lang )
{
    this->language = lang;
}
inline void
SingleShaderCompiler::SetDstBinary(const Util::String& d)
{
    this->dstBinary = d;
}
inline void
SingleShaderCompiler::SetDstHeader(const Util::String& dstHeader)
{
    this->dstHeader = dstHeader;
}
inline void
SingleShaderCompiler::AddIncludeDir(const Util::String& d)
{
    this->includeDirs.Append(d);
}
inline void
SingleShaderCompiler::SetDebugFlag(bool b)
{
    this->debug = b;
}
inline void
SingleShaderCompiler::SetAdditionalParams(const Util::String& p)
{
    this->additionalParams = p;
}
inline void
SingleShaderCompiler::SetQuietFlag(bool b)
{
    this->quiet = b;
}
}