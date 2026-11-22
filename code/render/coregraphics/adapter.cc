//------------------------------------------------------------------------------
//  adapter.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coregraphics/adapter.h"

namespace CoreGraphics
{

//------------------------------------------------------------------------------
/**
*/
Adapter::Code
Adapter::FromString(const Util::String& str)
{
    if ("None" == str) return None;
    else if ("Primary" == str) return Primary;
    else if ("Secondary" == str) return Secondary;
    else
    {
        n_error("Недопустимая строка адаптера «%s»!", str.AsCharPtr());
        return Primary;
    }
}

//------------------------------------------------------------------------------
/**
*/
Util::String
Adapter::ToString(Code code)
{
    switch (code)
    {
        case None:      return "None";
        case Primary:   return "Primary";
        case Secondary: return "Secondary";
        default:
            n_error("Неверный код адаптера!");
            return "";
    }
}

} // пространство имен CoreGraphics
