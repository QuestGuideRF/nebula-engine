#pragma once
#include "coregraphics/base/shaperendererbase.h"
#include "coregraphics/vertexlayout.h"
#include "util/fixedarray.h"
#include "frame/framecode.h"
namespace Vulkan
{
class VkShapeRenderer : public Base::ShapeRendererBase
{
    __DeclareClass(VkShapeRenderer);
public:
    VkShapeRenderer();
    virtual ~VkShapeRenderer();
    void Open();
    void Close();
    void DrawShapes(const CoreGraphics::CmdBufferId cmdBuf);
    static const int MaxNumVertices = 262140;
    static const int MaxNumIndices = 262140;
    static const int MaxVertexWidth = 8 * sizeof(float);
    static const int MaxIndexWidth = sizeof(int);
private:
    void DrawBufferedPrimitives(const CoreGraphics::CmdBufferId cmdBuf);
    void DrawBufferedIndexedPrimitives(const CoreGraphics::CmdBufferId cmdBuf);
    void GrowIndexBuffer();
    void GrowVertexBuffer();
    void DrawSimpleShape(const CoreGraphics::CmdBufferId cmdBuf, const Math::mat4& modelTransform, CoreGraphics::RenderShape::Type shapeType, const Math::vec4& color, const float lineThickness);
    void DrawMesh(const CoreGraphics::CmdBufferId cmdBuf, const Math::mat4& modelTransform, const CoreGraphics::MeshId mesh, const Math::vec4& color, const float lineThickness);
    void DrawPrimitives(const Math::mat4& modelTransform, CoreGraphics::PrimitiveTopology::Code topology, SizeT numVertices, const void* vertices, const float lineThickness);
    void DrawIndexedPrimitives(const Math::mat4& modelTransform, CoreGraphics::PrimitiveTopology::Code topology, SizeT numVertices, const void* vertices, SizeT numIndices, const void* indices, CoreGraphics::IndexType::Code indexType, const float lineThickness);
    Util::FixedArray<Resources::ResourceId> shapeMeshResources;
    Util::FixedArray<CoreGraphics::MeshId> meshes;
    Util::Array<CoreGraphics::VertexComponent> comps;
    static const SizeT MaxVertexIndexBuffers = 2;
    CoreGraphics::BufferId ibos[MaxVertexIndexBuffers];
    CoreGraphics::BufferId vbos[MaxVertexIndexBuffers];
    byte* vertexBufferPtr;
    byte* indexBufferPtr;
    IndexT indexBufferActiveIndex;
    IndexT vertexBufferActiveIndex;
    SizeT vertexBufferCapacity;
    SizeT indexBufferCapacity;
    CoreGraphics::VertexLayoutId vertexLayout;
    IndexT model;
    IndexT diffuseColor;
    IndexT lineWidth;
    SizeT vertexBufferOffset;
    SizeT indexBufferOffset;
    struct IndexedDraws
    {
        Util::Array<CoreGraphics::PrimitiveGroup> primitives;
        Util::Array<Math::mat4> transforms;
        Util::Array<uint64_t> firstVertexOffset;
        Util::Array<uint64_t> firstIndexOffset;
        Util::Array<CoreGraphics::IndexType::Code> indexType;
        Util::Array<float> lineThicknesses;
    } indexed[CoreGraphics::PrimitiveTopology::NumTopologies];
    struct UnindexedDraws
    {
        Util::Array<CoreGraphics::PrimitiveGroup> primitives;
        Util::Array<Math::mat4> transforms;
        Util::Array<uint64_t> firstVertexOffset;
        Util::Array<float> lineThicknesses;
    } unindexed[CoreGraphics::PrimitiveTopology::NumTopologies];
};
}