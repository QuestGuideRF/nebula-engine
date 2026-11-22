#pragma once
#include "util/string.h"
#include "math/vec4.h"
#include "math/vec2.h"
#include "threading/threadid.h"
namespace CoreGraphics
{
class TextElement
{
public:
    TextElement();
    TextElement(Threading::ThreadId threadId, const Util::String& text, const Math::vec4& color, const Math::vec2& pos, const float size);
    Threading::ThreadId GetThreadId() const;
    const Util::String& GetText() const;
    const Math::vec4& GetColor() const;
    const Math::vec2& GetPosition() const;
    const float GetSize() const;
private:
    Threading::ThreadId threadId;
    Util::String text;
    Math::vec4 color;
    Math::vec2 pos;
    float size;
};
inline Threading::ThreadId
TextElement::GetThreadId() const
{
    return this->threadId;
}
inline const Util::String&
TextElement::GetText() const
{
    return this->text;
}
inline const Math::vec4&
TextElement::GetColor() const
{
    return this->color;
}
inline const Math::vec2&
TextElement::GetPosition() const
{
    return this->pos;
}
inline const float
TextElement::GetSize() const
{
    return this->size;
}
}