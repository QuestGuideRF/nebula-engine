#pragma once
#include "coregraphics/config.h"
#include "resources/resourceid.h"
#include "ids/id.h"
#include "ids/idallocator.h"
#include "coregraphics/buffer.h"
#include "coregraphics/primitivetopology.h"
#include "coregraphics/primitivegroup.h"
#include "coregraphics/vertexlayout.h"
namespace CoreGraphics
{
struct VertexAlloc
{
    uint size, offset, node;
};
struct CmdBufferId;
ID_24_8_TYPE(MeshId);
struct VertexStream
{
    BufferId vertexBuffer;
    SizeT offset;
    IndexT index;
};
struct MeshCreateInfo
{
    Resources::ResourceName name;
    Util::StackArray<VertexStream, 4> streams;
    SizeT indexBufferOffset;
    BufferId indexBuffer;
    IndexType::Code indexType;
    VertexLayoutId vertexLayout;
    CoreGraphics::PrimitiveTopology::Code topology;
    Util::Array<CoreGraphics::PrimitiveGroup> primitiveGroups;
};
const MeshId CreateMesh(const MeshCreateInfo& info);
void DestroyMesh(const MeshId id);
const Util::Array<CoreGraphics::PrimitiveGroup>& MeshGetPrimitiveGroups(const MeshId id);
const CoreGraphics::PrimitiveGroup MeshGetPrimitiveGroup(const MeshId id, const IndexT group);
const BufferId MeshGetVertexBuffer(const MeshId id, const IndexT stream);
const void MeshSetVertexBuffer(const MeshId id, const BufferId buffer, const IndexT stream);
const uint64_t MeshGetVertexOffset(const MeshId id, const IndexT stream);
const BufferId MeshGetIndexBuffer(const MeshId id);
const uint64_t MeshGetIndexOffset(const MeshId id);
const IndexType::Code MeshGetIndexType(const MeshId id);
const CoreGraphics::PrimitiveTopology::Code MeshGetTopology(const MeshId id);
const CoreGraphics::VertexLayoutId MeshGetVertexLayout(const MeshId id);
const void MeshBind(const MeshId id, const CoreGraphics::CmdBufferId cmd);
enum
{
    Mesh_Name,
    Mesh_Internals,
};
struct __Mesh
{
    Util::StackArray<VertexStream, 4> streams;
    SizeT indexBufferOffset;
    BufferId indexBuffer;
    IndexType::Code indexType;
    VertexLayoutId vertexLayout;
    CoreGraphics::PrimitiveTopology::Code primitiveTopology;
    Util::Array<CoreGraphics::PrimitiveGroup> primitiveGroups;
};
typedef Ids::IdAllocatorSafe<
    0xFFFF,
    Resources::ResourceName,
    __Mesh
> MeshAllocator;
extern MeshAllocator meshAllocator;
_DECL_ACQUIRE_RELEASE(MeshId);
extern MeshId RectangleMesh;
extern MeshId DiskMesh;
}