#pragma once
#include "resources/resourceloader.h"
#include "materials/shaderconfig.h"
#include "coregraphics/config.h"
#include "materials/materialtemplates.h"
namespace IO
{
class BXmlReader;
}
namespace Materials
{
class MaterialLoader : public Resources::ResourceLoader
{
    __DeclareClass(MaterialLoader);
public:
    virtual void Setup() override;
    ResourceInitOutput InitializeResource(const ResourceLoadJob& job, const Ptr<IO::Stream>& stream) override;
    static void* AllocateConstantMemory(SizeT size);
    static void FlushMaterialBuffers(const CoreGraphics::CmdBufferId id, const CoreGraphics::QueueType queue);
    static CoreGraphics::BufferId GetMaterialBindingBuffer();
    static CoreGraphics::BufferId GetMaterialBuffer(const MaterialTemplates::MaterialProperties type);
    static IndexT RegisterTerrainMaterial(const MaterialInterfaces::TerrainMaterial& terrain);
private:
    void Unload(const Resources::ResourceId id) override;
};
}