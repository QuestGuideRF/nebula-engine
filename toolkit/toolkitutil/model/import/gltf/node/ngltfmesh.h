#pragma once
#include "ngltfnode.h"
#include "math/bbox.h"
#include "model/import/base/scenenode.h"
#include "model/meshutil/meshbuilder.h"
#include "toolkit-common/base/exporttypes.h"
#include "meshprimitive.h"
namespace ToolkitUtil
{
class NglTFMesh : public NglTFNode
{
    __DeclareClass(NglTFMesh);
public:
    typedef uint MeshMask;
    NglTFMesh();
    virtual ~NglTFMesh();
    static void Setup(
        Util::Array<MeshBuilder>& meshes
        , const Gltf::Mesh* gltfMesh
        , const Gltf::Document* scene
        , const ToolkitUtil::ExportFlags flags
        , const uint basePrimitive
        , const uint meshIndex
        , SceneNode** nodes
    );
    void Setup(Gltf::Node const* node, const Ptr<NglTFScene>& scene);
    void Discard();
    void SetBoundingBox(const Math::bbox& bbox);
    const Math::bbox& GetBoundingBox() const;
    ToolkitUtil::MeshBuilder* GetMesh() const;
    void SetMesh(ToolkitUtil::MeshBuilder* newMesh);
    Util::Array<MeshPrimitive>& Primitives();
protected:
    friend class NglTFScene;
    void ExtractMesh();
    int32_t meshId;
    Gltf::Mesh const* gltfMesh;
    ToolkitUtil::MeshBuilder* meshBuilder;
    Util::Array<MeshPrimitive> primGroups;
    Math::bbox boundingBox;
};
inline void
NglTFMesh::SetBoundingBox(const Math::bbox& bbox)
{
    this->boundingBox = bbox;
}
inline const Math::bbox&
NglTFMesh::GetBoundingBox() const
{
    return this->boundingBox;
}
inline ToolkitUtil::MeshBuilder*
NglTFMesh::GetMesh() const
{
    return this->meshBuilder;
}
inline void
NglTFMesh::SetMesh(ToolkitUtil::MeshBuilder* newMesh)
{
    this->meshBuilder = newMesh;
}
inline Util::Array<MeshPrimitive>&
NglTFMesh::Primitives()
{
    return this->primGroups;
}
}