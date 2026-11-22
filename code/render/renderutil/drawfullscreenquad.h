#pragma once
#include "coregraphics/shader.h"
#include "coregraphics/buffer.h"
#include "coregraphics/vertexlayout.h"
#include "coregraphics/primitivegroup.h"
namespace RenderUtil
{
class DrawFullScreenQuad
{
public:
    static void Setup();
    static void Discard();
    static bool IsValid();
    static void ApplyMesh(const CoreGraphics::CmdBufferId id);
    static const CoreGraphics::PrimitiveGroup& GetPrimitiveGroup();
private:
    static CoreGraphics::BufferId vertexBuffer;
    static CoreGraphics::VertexLayoutId vertexLayout;
    static CoreGraphics::PrimitiveGroup primGroup;
    static bool isValid;
};
inline bool
DrawFullScreenQuad::IsValid()
{
    return DrawFullScreenQuad::isValid;
}
}