#pragma once
#include "coregraphics/mesh.h"
#include "util/fixedarray.h"
namespace Particles
{
class EmitterMesh
{
public:
    EmitterMesh();
    ~EmitterMesh();
    void Setup(const CoreGraphics::MeshId mesh, IndexT primGroupIndex);
    void Discard();
    bool IsValid() const;
    struct EmitterPoint
    {
        Math::vec4 position;
        Math::vec4 normal;
        Math::vec4 tangent;
    };
    const EmitterPoint& GetEmitterPoint(IndexT key) const;
private:
    static const SizeT VertexWidth = sizeof(EmitterPoint) / sizeof(Math::vec4);
    SizeT numPoints;
    EmitterPoint* points;
};
inline bool
EmitterMesh::IsValid() const
{
    return (0 != this->points);
}
}