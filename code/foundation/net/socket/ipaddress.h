#pragma once
#if (__WIN32__)
#include "net/win32/win32ipaddress.h"
namespace Net
{
typedef Win32::Win32IpAddress IpAddress;
}
#elif __linux__
#include "net/posix/posixipaddress.h"
namespace Net
{
typedef Posix::PosixIpAddress IpAddress;
}
#else
#error "Класс IpAddress не реализован на этой платформе."
#endif