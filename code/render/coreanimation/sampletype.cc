//------------------------------------------------------------------------------
//  coreanimation/sampletype.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coreanimation/sampletype.h"

namespace CoreAnimation
{
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
SampleType::Code
SampleType::FromString(const String& str)
{
    if (str == "Step")            return Step;
    else if (str == "Linear")     return Linear;
    else if (str == "Hermite")    return Hermite;
    else if (str == "CatmullRom") return CatmullRom;
    else
    {
        return InvalidSampleType;
    }
}

//------------------------------------------------------------------------------
/**
*/
String
SampleType::ToString(Code c)
{
    switch (c)
    {
        case Step:          return "Step";
        case Linear:        return "Linear";
        case Hermite:       return "Hermite";
        case CatmullRom:    return "CatmullRom";
        default:
            n_error("SampleType::ToString(): неверный код типа образца!");
            return "";
    }
}

} // пространство имен CoreAnimation

