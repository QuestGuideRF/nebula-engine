//------------------------------------------------------------------------------
//  drawfullscreenquad.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "renderutil/drawfullscreenquad.h"

namespace RenderUtil
{
using namespace Util;
using namespace CoreGraphics;
using namespace Resources;

CoreGraphics::BufferId DrawFullScreenQuad::vertexBuffer = CoreGraphics::InvalidBufferId;
CoreGraphics::VertexLayoutId DrawFullScreenQuad::vertexLayout = CoreGraphics::InvalidVertexLayoutId;
CoreGraphics::PrimitiveGroup DrawFullScreenQuad::primGroup;
bool DrawFullScreenQuad::isValid = false;

//------------------------------------------------------------------------------
/**
*/
void
DrawFullScreenQuad::Setup()
{
    n_assert(!DrawFullScreenQuad::IsValid());
    DrawFullScreenQuad::isValid = true;

    if (DrawFullScreenQuad::vertexBuffer == CoreGraphics::InvalidBufferId)
    {
        // настройка вершинных компонентов
        Array<VertexComponent> vertexComponents;
        vertexComponents.Append(VertexComponent(VertexComponent::IndexName::Position, VertexComponent::Float3));
        vertexComponents.Append(VertexComponent(VertexComponent::IndexName::TexCoord1, VertexComponent::Float2));

        // создать углы и ультрафиолетовые лучи
        float left = -1.0f;
        float right = +3.0f;
        float top = +3.0f;
        float bottom = -1.0f;

        float u0 = 0.0f;
        float u1 = 2.0f;
        float v0 = 0.0f;
        float v1 = 2.0f;

        // настроить буфер вершин с двумя треугольниками
        float v[3][5];

#if (__VULKAN__ || __DX12__)
        // первый треугольник
        v[0][0] = left;     v[0][1] = bottom;   v[0][2] = 0.0f; v[0][3] = u0; v[0][4] = v0;
        v[1][0] = left;     v[1][1] = top;      v[1][2] = 0.0f; v[1][3] = u0; v[1][4] = v1;
        v[2][0] = right;    v[2][1] = bottom;   v[2][2] = 0.0f; v[2][3] = u1; v[2][4] = v0;
#else
        // первый треугольник
        v[0][0] = left;     v[0][1] = top;      v[0][2] = 0.0f; v[0][3] = u0; v[0][4] = v0;
        v[1][0] = left;     v[1][1] = bottom;   v[1][2] = 0.0f; v[1][3] = u0; v[1][4] = v1;
        v[2][0] = right;    v[2][1] = top;      v[2][2] = 0.0f; v[2][3] = u1; v[2][4] = v0;
#endif

        DrawFullScreenQuad::vertexLayout = CoreGraphics::CreateVertexLayout({ .name = "Полноэкранный четырехэкранный режим"_atm, .comps = vertexComponents });

        // загрузить буфер вершин
        BufferCreateInfo info;
        info.name = "Полноэкранный Quad VBO"_atm;
        info.size = 3;
        info.elementSize = CoreGraphics::VertexLayoutGetSize(DrawFullScreenQuad::vertexLayout);
        info.mode = CoreGraphics::DeviceLocal;
        info.usageFlags = CoreGraphics::VertexBuffer;
        info.data = v;
        info.dataSize = sizeof(v);
        DrawFullScreenQuad::vertexBuffer = CreateBuffer(info);

        // настроить примитивный групповой объект
        DrawFullScreenQuad::primGroup.SetBaseVertex(0);
        DrawFullScreenQuad::primGroup.SetNumVertices(3);
        DrawFullScreenQuad::primGroup.SetBaseIndex(0);
        DrawFullScreenQuad::primGroup.SetNumIndices(0);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
DrawFullScreenQuad::Discard()
{
    n_assert(DrawFullScreenQuad::IsValid());
    DrawFullScreenQuad::isValid = false;
    DestroyBuffer(DrawFullScreenQuad::vertexBuffer);
    DestroyVertexLayout(DrawFullScreenQuad::vertexLayout);
}

//------------------------------------------------------------------------------
/**
*/
void
DrawFullScreenQuad::ApplyMesh(const CoreGraphics::CmdBufferId id)
{
    CoreGraphics::CmdSetVertexLayout(id, DrawFullScreenQuad::vertexLayout);
    CoreGraphics::CmdSetPrimitiveTopology(id, PrimitiveTopology::TriangleList);
    CoreGraphics::CmdSetVertexBuffer(id, 0, DrawFullScreenQuad::vertexBuffer, 0);
}

//------------------------------------------------------------------------------
/**
*/
const CoreGraphics::PrimitiveGroup&
DrawFullScreenQuad::GetPrimitiveGroup()
{
    return DrawFullScreenQuad::primGroup;
}

} // пространство имен RenderUtil
