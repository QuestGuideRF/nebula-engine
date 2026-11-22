#pragma once
#include "core/types.h"
namespace ToolkitUtil
{
class MeshBuilderTriangle
{
public:
    MeshBuilderTriangle();
    MeshBuilderTriangle(IndexT i0, IndexT i1, IndexT i2);
    bool operator==(const MeshBuilderTriangle& rhs) const;
    bool operator!=(const MeshBuilderTriangle& rhs) const;
    bool operator<(const MeshBuilderTriangle& rhs) const;
    bool operator>(const MeshBuilderTriangle& rhs) const;
    int Compare(const MeshBuilderTriangle& rhs) const;
    void SetVertexIndices(IndexT i0, IndexT i1, IndexT i2);
    void GetVertexIndices(IndexT& i0, IndexT& i1, IndexT& i2) const;
    void SetVertexIndex(IndexT cornerIndex, IndexT vertexIndex);
    IndexT GetVertexIndex(IndexT cornerIndex) const;
private:
    friend class MeshBuilder;
    IndexT vertexIndex[3];
};
}