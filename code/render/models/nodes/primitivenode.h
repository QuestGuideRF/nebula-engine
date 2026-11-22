#pragma once
#include "modelnode.h"
#include "coregraphics/primitivegroup.h"
#include "resources/resourceid.h"
#include "shaderstatenode.h"
#include "coregraphics/mesh.h"
namespace Models
{
class PrimitiveNode : public ShaderStateNode
{
public:
    PrimitiveNode();
    virtual ~PrimitiveNode();
    uint GetPrimitiveGroupIndex() const { return this->loadContext.primIndex; }
    uint GetMeshIndex() const { return this->loadContext.meshIndex;}
    const CoreGraphics::PrimitiveGroup& GetPrimitiveGroup() const { return this->primGroup; }
    CoreGraphics::MeshId GetMesh() const { return this->mesh; }
    Resources::ResourceId GetMeshResource() const { return this->res; }
protected:
    friend class ModelLoader;
    virtual bool Load(const Util::FourCC& fourcc, const Util::StringAtom& tag, const Ptr<IO::BinaryReader>& reader, bool immediate) override;
    virtual void Unload() override;
    virtual void OnFinishedLoading(ModelStreamingData* streamingData) override;
    Resources::ResourceName meshResource;
    Resources::ResourceId res;
    CoreGraphics::MeshId mesh;
    struct LoadContext
    {
        uint16_t meshIndex;
        uint16_t primIndex;
    } loadContext;
    CoreGraphics::BufferId vbo, ibo;
    IndexT baseVboOffset, attributesVboOffset, iboOffset;
    CoreGraphics::IndexType::Code indexType;
    CoreGraphics::PrimitiveTopology::Code topology;
    CoreGraphics::PrimitiveGroup primGroup;
    CoreGraphics::VertexLayoutId vertexLayout;
};
}