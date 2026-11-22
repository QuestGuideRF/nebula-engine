//------------------------------------------------------------------------------
//  animbuildercurve.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "model/animutil/animbuildercurve.h"

namespace ToolkitUtil
{

using namespace CoreAnimation;

//------------------------------------------------------------------------------
/**
*/
AnimBuilderCurve::AnimBuilderCurve() 
    : firstKeyOffset(0)
    , numKeys(0)
    , preInfinityType(InfinityType::Code::InvalidInfinityType)
    , postInfinityType(InfinityType::Code::InvalidInfinityType)
    , curveType(CurveType::Code::InvalidCurveType)
{
    // empty
}
 

} // пространство имен ToolkitUtil