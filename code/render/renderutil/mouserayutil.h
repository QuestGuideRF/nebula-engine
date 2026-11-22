#pragma once
#include "math/vec2.h"
#include "math/mat4.h"
#include "math/line.h"
namespace RenderUtil
{
class MouseRayUtil
{
public:
    static Math::line ComputeWorldMouseRay(const Math::vec2& mousePos, float length, const Math::mat4& invViewMatrix, const Math::mat4& invProjMatrix, float nearPlane);
    static Math::line ComputeWorldMouseRayOrtho(const Math::vec2& mousePos, float length, const Math::mat4& invViewMatrix, const Math::mat4& invProjMatrix, float nearPlane);
};
}