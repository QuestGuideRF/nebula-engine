//------------------------------------------------------------------------------
//  mouserayutil.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "renderutil/mouserayutil.h"

namespace RenderUtil
{
using namespace Math;

//------------------------------------------------------------------------------
/**
    Utility function which computes a ray in world space between the eye
    and the current mouse position on the near plane of a perspective view frustum.
    Mouse position is expected in the range 0..1.
*/
line
MouseRayUtil::ComputeWorldMouseRay(const vec2& mousePos, float length, const mat4& invViewMatrix, const mat4& invProjMatrix, float nearPlane)
{
    // Вычислить положение мыши в мировых координатах.
    vec4 screenCoord3D((mousePos.x - 0.5f) * 2.0f, (mousePos.y - 0.5f) * 2.0f, 1.0f, 1.0f);
    vec4 viewCoord = invProjMatrix * screenCoord3D;
    viewCoord.w = 0.0f;
    vec4 localMousePos = viewCoord * nearPlane * 1.1f;;
    localMousePos.w = 1;
    localMousePos.y = -localMousePos.y;
    vec4 worldMousePos = invViewMatrix * localMousePos;
    vec4 worldMouseDir = worldMousePos - invViewMatrix.position;
    worldMouseDir = normalize(worldMouseDir);
    worldMouseDir *= length;
    return line(worldMousePos.vec, (worldMousePos + worldMouseDir).vec);
}
//------------------------------------------------------------------------------
/**
    Вспомогательная функция, которая вычисляет луч, простирающийся в мировом пространстве между курсором мыши и
    положение на ближней плоскости и длина по оси z камеры
    Положение мыши ожидается в диапазоне 0..1.

    @todo Есть лучшие способы сделать это!
*/
line
MouseRayUtil::ComputeWorldMouseRayOrtho(const vec2& mousePos, float length, const mat4& invViewMatrix, const mat4& invProjMatrix, float nearPlane)
{
    vec4 screenCoord3D((mousePos.x - 0.5f) * 2.0f, (mousePos.y - 0.5f) * 2.0f, 1.0f, 1.0f);
    vec4 viewCoord = invProjMatrix * screenCoord3D;
    viewCoord.w = 0.0f;
    vec4 localMousePos = viewCoord;
    localMousePos.w = 1;
    localMousePos.y = -localMousePos.y;
    vec4 worldMousePos = invViewMatrix * localMousePos;
    vec4 worldMouseDir = invViewMatrix.z_axis;
    worldMouseDir *= length;
    return line(worldMousePos.vec, (worldMousePos + worldMouseDir).vec);
}

} // пространство имен RenderUtil
