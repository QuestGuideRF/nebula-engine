//------------------------------------------------------------------------------
//  mousebutton.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "input/mousebutton.h"

namespace Input
{

//------------------------------------------------------------------------------
/**
*/
Util::String
MouseButton::ToString(Code code)
{
    switch (code)
    {
        case LeftButton:    return "LeftButton";
        case RightButton:   return "RightButton";
        case MiddleButton:  return "MiddleButton";
        default:            break;
                
    }
    n_error("Неверный код кнопки мыши!\n");
    return "";
}

//------------------------------------------------------------------------------
/**
*/
MouseButton::Code
MouseButton::FromString(const Util::String& str)
{
    if ("LeftButton" == str) return LeftButton;
    else if ("RightButton" == str) return RightButton;
    else if ("MiddleButton" == str) return MiddleButton;
    else
    {
        n_error("Неверная строка кнопки мыши!\n");
        return InvalidMouseButton;
    }
}

} // пространство имен Ввод