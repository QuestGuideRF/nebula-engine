#pragma once
#include "toolkit-common/platform.h"
#include "io/uri.h"
#include "util/string.h"
namespace ToolkitUtil
{
class ShaderCompiler
{
public:
    ShaderCompiler();
    ~ShaderCompiler();
    void SetPlatform(Platform::Code platform);
    void SetLanguage(const Util::String& lang);
    void SetSrcShaderBaseDir(const Util::String& srcDir);
    void SetSrcShaderCustomDir(const Util::String& srcDir);
    void SetDstShaderDir(const Util::String& dstDir);
    void SetSrcFrameShaderBaseDir(const Util::String& srcDir);
    void SetSrcFrameShaderCustomDir(const Util::String& srcDir);
    void SetDstFrameShaderDir(const Util::String& dstDir);
    void SetSrcMaterialBaseDir(const Util::String& srcDir);
    void SetSrcMaterialCustomDir(const Util::String& srcDir);
    void SetDstMaterialsDir(const Util::String& dstDir);
    void SetForceFlag(bool b);
    void SetDebugFlag(bool b);
    void SetAdditionalParams(const Util::String& params);
    void SetQuietFlag(bool b);
    bool CompileShaders();
    bool CompileFrameShaders();
    bool CompileMaterials();
private:
    bool CheckRecompile(const Util::String& srcPath, const Util::String& dstPath);
    bool CompileHLSL(const Util::String& srcPath);
    bool CompileGLSL(const Util::String& srcPath);
    bool CompileSPIRV(const Util::String& srcPath);
    bool WriteShaderDictionary();
    Platform::Code platform;
    Util::String srcShaderBaseDir;
    Util::String srcShaderCustomDir;
    Util::String dstShaderDir;
    Util::String srcFrameShaderBaseDir;
    Util::String srcFrameShaderCustomDir;
    Util::String dstFrameShaderDir;
    Util::String srcMaterialBaseDir;
    Util::String srcMaterialCustomDir;
    Util::String dstMaterialDir;
    Util::String language;
    bool force;
    bool quiet;
    bool debug;
    Util::String additionalParams;
    Util::Array<Util::String> shaderNames;
};
inline void
ShaderCompiler::SetPlatform(Platform::Code p)
{
    this->platform = p;
}
inline void
ShaderCompiler::SetLanguage( const Util::String& lang )
{
    this->language = lang;
}
inline void
ShaderCompiler::SetSrcShaderBaseDir(const Util::String& d)
{
    this->srcShaderBaseDir = d;
}
inline void
ShaderCompiler::SetSrcShaderCustomDir( const Util::String& d )
{
    this->srcShaderCustomDir = d;
}
inline void
ShaderCompiler::SetDstShaderDir(const Util::String& d)
{
    this->dstShaderDir = d;
}
inline void
ShaderCompiler::SetSrcFrameShaderBaseDir(const Util::String& d)
{
    this->srcFrameShaderBaseDir = d;
}
inline void
ShaderCompiler::SetSrcFrameShaderCustomDir( const Util::String& d )
{
    this->srcFrameShaderCustomDir = d;
}
inline void
ShaderCompiler::SetDstFrameShaderDir(const Util::String& d)
{
    this->dstFrameShaderDir = d;
}
inline void
ShaderCompiler::SetSrcMaterialBaseDir( const Util::String& d )
{
    this->srcMaterialBaseDir = d;
}
inline void
ShaderCompiler::SetSrcMaterialCustomDir( const Util::String& d )
{
    this->srcMaterialCustomDir = d;
}
inline void
ShaderCompiler::SetDstMaterialsDir( const Util::String& dstDir )
{
    this->dstMaterialDir = dstDir;
}
inline void
ShaderCompiler::SetForceFlag(bool b)
{
    this->force = b;
}
inline void
ShaderCompiler::SetDebugFlag(bool b)
{
    this->debug = b;
}
inline void
ShaderCompiler::SetAdditionalParams(const Util::String& p)
{
    this->additionalParams = p;
}
inline void
ShaderCompiler::SetQuietFlag(bool b)
{
    this->quiet = b;
}
}