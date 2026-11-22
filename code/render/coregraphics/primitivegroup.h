#pragma once
#include "coregraphics/primitivetopology.h"
#include "math/bbox.h"
namespace CoreGraphics
{
class PrimitiveGroup
{
public:
    PrimitiveGroup();
    void SetBaseVertex(IndexT i);
    IndexT GetBaseVertex() const;
    void SetNumVertices(SizeT n);
    SizeT GetNumVertices() const;
    void SetBaseIndex(IndexT i);
    IndexT GetBaseIndex() const;
    void SetNumIndices(SizeT n);
    SizeT GetNumIndices() const;
    SizeT GetNumPrimitives(const CoreGraphics::PrimitiveTopology::Code& topo) const;
private:
    IndexT baseVertex;
    SizeT numVertices;
    IndexT baseIndex;
    SizeT numIndices;
};
inline
PrimitiveGroup::PrimitiveGroup() :
    baseVertex(0),
    numVertices(0),
    baseIndex(0),
    numIndices(0)
{
}
inline void
PrimitiveGroup::SetBaseVertex(IndexT i)
{
    this->baseVertex = i;
}
inline IndexT
PrimitiveGroup::GetBaseVertex() const
{
    return this->baseVertex;
}
inline void
PrimitiveGroup::SetNumVertices(SizeT n)
{
    this->numVertices = n;
}
inline SizeT
PrimitiveGroup::GetNumVertices() const
{
    return this->numVertices;
}
inline void
PrimitiveGroup::SetBaseIndex(IndexT i)
{
    this->baseIndex = i;
}
inline IndexT
PrimitiveGroup::GetBaseIndex() const
{
    return this->baseIndex;
}
inline void
PrimitiveGroup::SetNumIndices(SizeT n)
{
    this->numIndices = n;
}
inline SizeT
PrimitiveGroup::GetNumIndices() const
{
    return this->numIndices;
}
inline SizeT
PrimitiveGroup::GetNumPrimitives(const CoreGraphics::PrimitiveTopology::Code& topo) const
{
    if (this->numIndices > 0)
    {
        return PrimitiveTopology::NumberOfPrimitives(topo, this->numIndices);
    }
    else
    {
        return PrimitiveTopology::NumberOfPrimitives(topo, this->numVertices);
    }
}
}