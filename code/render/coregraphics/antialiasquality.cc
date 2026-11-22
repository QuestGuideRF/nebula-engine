//------------------------------------------------------------------------------
//  antialiasquality.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coregraphics/antialiasquality.h"

namespace CoreGraphics
{

//------------------------------------------------------------------------------
/**
*/
AntiAliasQuality::Code
AntiAliasQuality::FromString(const Util::String& str)
{
    if ("None" == str) return None;
    else if ("Low" == str) return Low;
    else if ("Medium" == str) return Medium;
    else if ("High" == str) return High;
    else
    {
        n_error("Недопустимая строка качества сглаживания «%s»!", str.AsCharPtr());
        return None;
    }
}

//------------------------------------------------------------------------------
/**
*/
Util::String
AntiAliasQuality::ToString(Code code)
{
    switch (code)
    {
        case None:      return "None";
        case Low:       return "Low";
        case Medium:    return "Medium";
        case High:      return "High";
        default:
            n_error("Неверный код качества сглаживания!");
            return "";
    }
}

} // пространство имен CoreGraphics
