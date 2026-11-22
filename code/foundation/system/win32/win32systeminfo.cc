//------------------------------------------------------------------------------
//  win32systeminfo.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "system/systeminfo.h"
#include "system/win32/win32systeminfo.h"

namespace System
{
    CpuArchType CpuArch;
    PlatformType Platform;
    SizeT NumCpuCores;
    SizeT PageSize;
}

namespace Win32
{

//------------------------------------------------------------------------------
/**
*/
Win32SystemInfo::Win32SystemInfo()
{
    // получить информацию о времени выполнения из Windows
    SYSTEM_INFO sysInfo;
    ::GetSystemInfo(&sysInfo);    
    switch (sysInfo.wProcessorArchitecture)
    {
        case PROCESSOR_ARCHITECTURE_AMD64:  System::CpuArch = System::X86_64; break;
        case PROCESSOR_ARCHITECTURE_INTEL:  System::CpuArch = System::X86_32; break;
        default:                            System::CpuArch = System::UnknownCpuType; break;
    }
    System::NumCpuCores = sysInfo.dwNumberOfProcessors;
    System::PageSize = sysInfo.dwPageSize;
    System::Platform = System::Win32;
}

} // пространство имен Win32
