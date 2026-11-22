#pragma once
namespace System
{
enum PlatformType
{
    Win32,
    Linux,
    UnknownPlatform,
};
enum CpuArchType
{
    X86_32,
    X86_64,
    UnknownCpuType,
};
inline const char*
PlatformTypeAsString(PlatformType p)
{
    switch (p)
    {
        case Win32:     return "win32";
        case Linux:     return "linux";
        default:        return "unknownplatform";
    }
}
inline const char*
CpuArchTypeAsString(CpuArchType cpu)
{
    switch (cpu)
    {
        case X86_32:            return "x86_32";
        case X86_64:            return "x86_64";
        default:                return "unknowncputype";
    }
}
extern CpuArchType CpuArch;
extern PlatformType Platform;
extern SizeT NumCpuCores;
extern SizeT PageSize;
}
#if __WIN32__
#include "system/win32/win32systeminfo.h"
namespace System
{
class SystemInfo : public Win32::Win32SystemInfo {};
}
#elif __linux__
#include "system/posix/posixsysteminfo.h"
namespace System
{
class SystemInfo : public Posix::PosixSystemInfo {};
}
#else
#error "System::SystemInfo не реализован на этой платформе!"
#endif