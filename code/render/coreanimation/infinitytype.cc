//------------------------------------------------------------------------------
//  coreanimation/infinitytype.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coreanimation/infinitytype.h"

namespace CoreAnimation
{
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
InfinityType::Code
InfinityType::FromString(const String& str)
{
    if (str == "Constant")       return Constant;
    else if (str == "Cycle")     return Cycle;
    else
    {
        return InvalidInfinityType;
    }
}

//------------------------------------------------------------------------------
/**
*/
String
InfinityType::ToString(Code c)
{
    switch (c)
    {
        case Constant:  return "Constant";
        case Cycle:     return "Cycle";
        default:
            n_error("InfinityType::ToString(): неверный код типа бесконечности!");
            return "";
    }
}

} // пространство имен CoreAnimation
