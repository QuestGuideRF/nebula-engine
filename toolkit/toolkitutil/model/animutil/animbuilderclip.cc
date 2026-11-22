//------------------------------------------------------------------------------
//  animbuilderclip.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "model/animutil/animbuilderclip.h"

namespace ToolkitUtil
{
using namespace CoreAnimation;

//------------------------------------------------------------------------------
/**
*/
AnimBuilderClip::AnimBuilderClip()
    : duration(0)
    , firstCurveOffset(0)
    , numCurves(0)
    , firstEventOffset(0)
    , numEvents(0)
    , firstVelocityCurveOffset(0)
    , numVelocityCurves(0)

{
    // empty
}

} // пространство имен ToolkitUtil