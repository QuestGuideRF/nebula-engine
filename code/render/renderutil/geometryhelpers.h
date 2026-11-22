#pragma once
#include "coregraphics/buffer.h"
#include "coregraphics/primitivegroup.h"
#include "coregraphics/primitivetopology.h"
namespace RenderUtil
{
struct Geometry
{
    CoreGraphics::BufferId vertexBuffer;
    CoreGraphics::BufferId indexBuffer;
    CoreGraphics::PrimitiveGroup primitiveGroup;
    CoreGraphics::PrimitiveTopology::Code topology;
};
class GeometryHelpers
{
public:
    static CoreGraphics::MeshId CreateRectangle();
    static CoreGraphics::MeshId CreateDisk(SizeT numPoints);
};
}