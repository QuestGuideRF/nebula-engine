//------------------------------------------------------------------------------
//  linuxthreadlocalptr.cc
//  (C) 2011 А.Вайсфлог
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "linuxthreadlocalptr.h"
#include "core/sysfunc.h"

namespace Linux
{
//------------------------------------------------------------------------------
/**
 */
LinuxThreadLocalPtr::LinuxThreadLocalPtr()
{
    SysFunc::Setup();
    this->slot = LinuxThreadLocalData::RegisterSlot();
}

} // пространство имен Linux
