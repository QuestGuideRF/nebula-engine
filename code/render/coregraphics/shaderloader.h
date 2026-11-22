#pragma once
#include "resources/resourceloader.h"
namespace CoreGraphics
{
void CmdSetShaderProgram(const CoreGraphics::CmdBufferId, const CoreGraphics::ShaderProgramId, bool);
class ShaderLoader : public Resources::ResourceLoader
{
    __DeclareClass(ShaderLoader);
public:
    ShaderLoader();
    virtual ~ShaderLoader();
private:
    friend class VkVertexSignatureCache;
    friend class VkPipelineDatabase;
    friend void CoreGraphics::CmdSetShaderProgram(const CoreGraphics::CmdBufferId, const CoreGraphics::ShaderProgramId, bool);
    ResourceInitOutput InitializeResource(const ResourceLoadJob& job, const Ptr<IO::Stream>& stream) override;
    Resources::Resource::State ReloadFromStream(const Resources::ResourceId id, const Ptr<IO::Stream>& stream) override;
    void Unload(const Resources::ResourceId id) override;
};
}