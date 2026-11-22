//------------------------------------------------------------------------------
//  darwinsysteminfo.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "system/darwin/darwinsysteminfo.h"

namespace Darwin
{

//------------------------------------------------------------------------------
/**
*/
DarwinSystemInfo::DarwinSystemInfo()
{
    this->platform = MacOSX;
    this->cpuType = X86_32;
    // XXX: это->numCpuCores = sysInfo.dwNumberOfProcessors;
    // XXX: this->pageSize = sysInfo.dwPageSize;
}

} // пространство имен Дарвина
