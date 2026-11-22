#pragma once
#include "model/meshutil/meshbuildervertex.h"
#include "model/meshutil/meshbuildertriangle.h"
#include "model/meshutil/meshbuildergroup.h"
#include "util/fixedarray.h"
#include "util/array.h"
#include "math/bbox.h"
#include "coregraphics/primitivetopology.h"
#include "coregraphics/vertexlayout.h"
namespace ToolkitUtil
{
class MeshBuilder
{
    struct Mesh;
public:
    MeshBuilder();
    void NewMesh(SizeT numVertices, SizeT numTriangles);
    void SetComponents(const MeshBuilderVertex::ComponentMask mask);
    const MeshBuilderVertex::ComponentMask GetComponents();
    void AddVertex(const MeshBuilderVertex& v);
    SizeT GetNumVertices() const;
    MeshBuilderVertex& VertexAt(IndexT i) const;
    void AddTriangle(const MeshBuilderTriangle& t);
    SizeT GetNumTriangles() const;
    MeshBuilderTriangle& TriangleAt(IndexT i) const;
    void SetPrimitiveTopology(const CoreGraphics::PrimitiveTopology::Code& p);
    const CoreGraphics::PrimitiveTopology::Code& GetPrimitiveTopology();
    void SetPrimitiveGroups(const Util::Array<MeshBuilderGroup>& groups);
    const Util::Array<MeshBuilderGroup>& GetPrimitiveGroups();
    void ClearPrimitiveGroups();
    void CopyTriangle(const MeshBuilder& srcMesh, IndexT triIndex, Util::FixedArray<IndexT>& indexMap);
    Math::bbox ComputeBoundingBox() const;
    void SortTriangles();
    void Clear();
    void Transform(const Math::mat4& m);
    void Deflate(Util::FixedArray<Util::Array<IndexT> >* collapseMap);
    void Inflate();
    void FlipUvs();
    void Cleanup(Util::Array<Util::Array<int> > *collapseMap);
    void Merge(MeshBuilder const& srcMesh);
    void CalculateNormals();
    void CalculateTangents();
private:
    static int __cdecl VertexSorter(const void* elm0, const void* elm1);
    static MeshBuilder* qsortData;
    friend class MeshBuilderSaver;
    friend class SkinPartitioner;
    friend class SkinFragment;
    friend class NFbxScene;
    Util::Array<MeshBuilderTriangle> triangles;
    CoreGraphics::PrimitiveTopology::Code topology;
    MeshBuilderVertex::ComponentMask componentMask;
    Util::Array<MeshBuilderVertex> vertices;
    Util::Array<MeshBuilderGroup> groups;
};
inline void
MeshBuilder::AddVertex(const MeshBuilderVertex& v)
{
    this->vertices.Append(v);
}
inline SizeT
MeshBuilder::GetNumVertices() const
{
    return this->vertices.Size();
}
inline MeshBuilderVertex&
MeshBuilder::VertexAt(IndexT i) const
{
    return this->vertices[i];
}
inline void
MeshBuilder::AddTriangle(const MeshBuilderTriangle& t)
{
    this->triangles.Append(t);
}
inline SizeT
MeshBuilder::GetNumTriangles() const
{
    return this->triangles.Size();
}
inline MeshBuilderTriangle&
MeshBuilder::TriangleAt(IndexT i) const
{
    return this->triangles[i];
}
}