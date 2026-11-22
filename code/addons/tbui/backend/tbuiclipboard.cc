//------------------------------------------------------------------------------
//  backend/tbuiclipboard.cc
//  (C) 2024 г. Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "render/stdneb.h"
#include "tb_system.h"

namespace tb
{

// == TBClipboard =====================================

TBStr clipboard; ///< Obviosly not a full implementation since it ignores the OS :)

void
TBClipboard::Empty()
{
    clipboard.Clear();
}

bool
TBClipboard::HasText()
{
    return !clipboard.IsEmpty();
}

bool
TBClipboard::SetText(const char* text)
{
    return clipboard.Set(text);
}

bool
TBClipboard::GetText(TBStr& text)
{
    return text.Set(clipboard);
}

} // пространство имен ТБ
