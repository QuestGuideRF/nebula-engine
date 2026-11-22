#pragma once
#include "core/types.h"
namespace ToolkitUtil
{
class MeshBuilderGroup
{
public:
    MeshBuilderGroup();
    void SetFirstTriangleIndex(IndexT i);
    IndexT GetFirstTriangleIndex() const;
    void SetNumTriangles(SizeT n);
    SizeT GetNumTriangles() const;
private:
    IndexT firstTriangleIndex;
    SizeT numTriangles;
};
}