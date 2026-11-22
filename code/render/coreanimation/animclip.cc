//------------------------------------------------------------------------------
//  animclip.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coreanimation/animclip.h"

namespace CoreAnimation
{
using namespace Util;
using namespace Math;

//------------------------------------------------------------------------------
/**
*/
AnimClip::AnimClip()
    : numCurves(0)
    , firstCurve(0)
    , numEvents(0)
    , firstEvent(0)
    , numVelocityCurves(0)
    , firstVelocityCurve(0)
    , duration(0)
{
    // empty
}

} // пространство имен CoreAnimation
