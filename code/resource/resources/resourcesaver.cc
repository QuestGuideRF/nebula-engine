//------------------------------------------------------------------------------
// resourcesaver.cc
// (C) 2017-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "resourcesaver.h"

namespace Resources
{

__ImplementAbstractClass(Resources::ResourceSaver, 'RESA', Core::RefCounted);
//------------------------------------------------------------------------------
/**
*/
ResourceSaver::ResourceSaver()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ResourceSaver::~ResourceSaver()
{
    // empty
}

} // Ресурсы пространства имен