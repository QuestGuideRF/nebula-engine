//------------------------------------------------------------------------------
//  @file color.cc
//  @copyright (C) 2025 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "color.h"
namespace Util
{

// Для обеспечения единообразия все цвета должны быть цвета X11.

const Color Color::red = Color(0xFFFF0000);
const Color Color::green = Color(0xFF00FF00);
const Color Color::blue = Color(0xFF0000FF);
const Color Color::yellow = Color(0xFFFFFF00);
const Color Color::purple = Color(0xFFA020F0);
const Color Color::orange = Color(0xFFFFA500);
const Color Color::black = Color(0xFF000000);
const Color Color::white = Color(0xFFFFFFFF);
const Color Color::gray = Color(0xFFBEBEBE);

} // пространство имен Util