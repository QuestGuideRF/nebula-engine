#pragma once
#include "coregraphics/mesh.h"
namespace CoreGraphics
{
RESOURCE_ID_TYPE(MeshResourceId);
struct MeshResource
{
    Util::Array<CoreGraphics::MeshId> meshes;
};
const MeshId MeshResourceGetMesh(const MeshResourceId id, const IndexT index);
const SizeT MeshResourceGetNumMeshes(const MeshResourceId id);
void DestroyMeshResource(const MeshResourceId id);
enum
{
    MeshResource_Meshes,
    MeshResource_VertexData,
    MeshResource_IndexData
};
typedef Ids::IdAllocator<
    Util::FixedArray<MeshId>,
    CoreGraphics::VertexAlloc,
    CoreGraphics::VertexAlloc
> MeshResourceAllocator;
extern MeshResourceAllocator meshResourceAllocator;
}