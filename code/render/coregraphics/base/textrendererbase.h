#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "util/string.h"
#include "math/vec4.h"
#include "math/vec2.h"
#include "threading/threadid.h"
#include "coregraphics/textelement.h"
namespace Base
{
class TextRendererBase : public Core::RefCounted
{
    __DeclareClass(TextRendererBase);
    __DeclareSingleton(TextRendererBase);
public:
    TextRendererBase();
    virtual ~TextRendererBase();
    void Open();
    void Close();
    bool IsOpen() const;
    void DrawTextElements();
    void DeleteTextElementsByThreadId(Threading::ThreadId threadId);
    void AddTextElement(const CoreGraphics::TextElement& textElement);
    void AddTextElements(const Util::Array<CoreGraphics::TextElement>& textElement);
protected:
    Util::Array<CoreGraphics::TextElement> textElements;
    bool isOpen;
};
inline bool
TextRendererBase::IsOpen() const
{
    return this->isOpen;
}
}