#pragma once
#include "math/mat4.h"
#include "math/vec4.h"
#include "coregraphics/primitivetopology.h"
#include "coregraphics/indextype.h"
#include "io/memorystream.h"
#include "threading/threadid.h"
#include "mesh.h"
namespace CoreGraphics
{
class RenderShape
{
public:
    enum Type
    {
        Box,
        Sphere,
        Cylinder,
        Torus,
        Cone,
        Arrow,
        Primitives,
        IndexedPrimitives,
        RenderMesh,
        NumShapeTypes,
        InvalidShapeType,
    };
    enum RenderFlag
    {
        CheckDepth = 0,
        AlwaysOnTop,
        Wireframe,
        NumDepthFlags
    };
    struct RenderShapeVertex
    {
        Math::vec4 pos;
        Math::vec4 color;
        RenderShapeVertex() : color(Math::vec4(1)) {}
    };
    RenderShape();
    RenderShape(Type shapeType, RenderFlag depthFlag, const Math::mat4& modelTransform, const Math::vec4& color);
    bool IsValid() const;
    void SetupSimpleShape(
        Type shapeType
        , RenderFlag depthFlag
        , const Math::vec4& color
        , const Math::mat4& modelTransform = Math::mat4::identity
        , float lineThickness = 1.0f);
    void SetupPrimitives(
        const RenderShape::RenderShapeVertex* vertices
        , SizeT numVertices
        , PrimitiveTopology::Code topology
        , RenderFlag depthFlag
        , const Math::mat4 transform = Math::mat4::identity
        , float lineThickness = 1.0f
    );
    void SetupPrimitives(
        const Util::Array<RenderShape::RenderShapeVertex> vertices
        , PrimitiveTopology::Code topology
        , RenderFlag depthFlag
        , const Math::mat4 transform = Math::mat4::identity
        , float lineThickness = 1.0f
    );
    void SetupIndexPrimitives(
        const RenderShape::RenderShapeVertex* vertices
        , SizeT numVertices
        , const void* indices
        , SizeT numIndices
        , IndexType::Code indexType
        , PrimitiveTopology::Code topology
        , RenderFlag depthFlag
        , const Math::mat4 transform = Math::mat4::identity
        , float lineThickness = 1.0f
    );
    void SetupIndexPrimitives(
        const Util::Array<RenderShape::RenderShapeVertex> vertices
        , const Util::Array<uint16_t> indices
        , PrimitiveTopology::Code topology
        , RenderFlag depthFlag
        , const Math::mat4 transform = Math::mat4::identity
        , float lineThickness = 1.0f
    );
    void SetupIndexPrimitives(
        const Util::Array<RenderShape::RenderShapeVertex> vertices
        , const Util::Array<uint32_t> indices
        , PrimitiveTopology::Code topology
        , RenderFlag depthFlag
        , const Math::mat4 transform = Math::mat4::identity
        , float lineThickness = 1.0f
    );
    void SetupMesh(
        const MeshId mesh
        , const IndexT groupIndex
        , const Math::vec4& color
        , RenderFlag depthFlag
        , const Math::mat4& modelTransform = Math::mat4::identity
        , float lineThickness = 1.0f
    );
    Type GetShapeType() const;
    RenderFlag GetDepthFlag() const;
    const Math::mat4& GetModelTransform() const;
    PrimitiveTopology::Code GetTopology() const;
    SizeT GetNumVertices() const;
    SizeT GetNumIndices() const;
    const void* GetVertexData() const;
    const void* GetIndexData() const;
    IndexType::Code GetIndexType() const;
    const Math::vec4& GetColor() const;
    const VertexLayoutId GetVertexLayout() const;
    const MeshId GetMesh() const;
    const IndexT& GetPrimitiveGroupIndex() const;
    const float GetLineThickness() const;
private:
    Type shapeType;
    RenderFlag depthFlag;
    Math::mat4 modelTransform;
    PrimitiveTopology::Code topology;
    SizeT numIndices, numVertices;
    SizeT vertexDataOffset;
    IndexType::Code indexType;
    Math::vec4 color;
    float lineThickness;
    IndexT groupIndex;
    MeshId mesh;
    VertexLayoutId vertexLayout;
    Ptr<IO::MemoryStream> dataStream;
};
inline bool
RenderShape::IsValid() const
{
    return (InvalidShapeType != this->shapeType);
}
inline RenderShape::Type
RenderShape::GetShapeType() const
{
    return this->shapeType;
}
inline RenderShape::RenderFlag
RenderShape::GetDepthFlag() const
{
    return this->depthFlag;
}
inline const Math::mat4&
RenderShape::GetModelTransform() const
{
    return this->modelTransform;
}
inline PrimitiveTopology::Code
RenderShape::GetTopology() const
{
    n_assert((Primitives == this->shapeType) || (IndexedPrimitives == this->shapeType));
    return this->topology;
}
inline SizeT
RenderShape::GetNumVertices() const
{
    n_assert((Primitives == this->shapeType) || (IndexedPrimitives == this->shapeType));
    return this->numVertices;
}
inline SizeT
RenderShape::GetNumIndices() const
{
    n_assert(IndexedPrimitives == this->shapeType);
    return this->numIndices;
}
inline const void*
RenderShape::GetVertexData() const
{
    n_assert((Primitives == this->shapeType) || (IndexedPrimitives == this->shapeType));
    const void* ptr = ((uchar*)this->dataStream->GetRawPointer()) + this->vertexDataOffset;
    return ptr;
}
inline const void*
RenderShape::GetIndexData() const
{
    n_assert(IndexedPrimitives == this->shapeType);
    const void* ptr = this->dataStream->GetRawPointer();
    return ptr;
}
inline IndexType::Code
RenderShape::GetIndexType() const
{
    n_assert(IndexedPrimitives == this->shapeType);
    return this->indexType;
}
inline const Math::vec4&
RenderShape::GetColor() const
{
    return this->color;
}
inline const VertexLayoutId
RenderShape::GetVertexLayout() const
{
    return this->vertexLayout;
}
inline const MeshId
RenderShape::GetMesh() const
{
    return this->mesh;
}
inline const IndexT&
RenderShape::GetPrimitiveGroupIndex() const
{
    return this->groupIndex;
}
inline const float
RenderShape::GetLineThickness() const
{
    return this->lineThickness;
}
}