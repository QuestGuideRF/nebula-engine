#pragma once
#include "core/types.h"
#include "math/vec4.h"
namespace Util
{
class Color : public Math::vec4
{
public:
    Color() = default;
    Color(const Color& c) = default;
    Color(float r, float g, float b, float a);
    Color(const Math::vec4& v);
    Color(const Math::vec3& v);
    Color(uint32_t argb);
    explicit Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    static Color RGBA(uint32_t rgba);
    static Color ARGB(uint32_t argb);
    static const Color red;
    static const Color green;
    static const Color blue;
    static const Color yellow;
    static const Color purple;
    static const Color orange;
    static const Color black;
    static const Color white;
    static const Color gray;
};
__forceinline
Color::Color(float r, float g, float b, float a)
    : Math::vec4(r, g, b, a)
{
}
__forceinline
Color::Color(const Math::vec4& v)
    : Math::vec4(v)
{
}
__forceinline
Color::Color(const Math::vec3& v)
    : Math::vec4(v, 1.0f)
{
}
__forceinline
Color::Color(uint32_t argb)
{
    const float scale = 1.0f / 255.0f;
    this->vec = _mm_setr_ps(
        scale * ((argb >> 16) & 0xFF), scale * ((argb >> 8) & 0xFF), scale * (argb & 0xFF), scale * ((argb >> 24) & 0xFF)
    );
}
__forceinline
Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    const float scale = 1.0f / 255.0f;
    this->vec = _mm_setr_ps(scale * r, scale * g, scale * b, scale * a);
}
__forceinline Color
Color::RGBA(uint32_t rgba)
{
    return Color((uint8_t)((rgba >> 24) & 0xFF), (uint8_t)((rgba >> 16) & 0xFF), (uint8_t)((rgba >> 8) & 0xFF), (uint8_t)(rgba & 0xFF));
}
__forceinline Color
Color::ARGB(uint32_t argb)
{
    return Color(argb);
}
}