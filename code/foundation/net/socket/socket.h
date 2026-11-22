#pragma once
#if (__WIN32__)
#include "net/win32/win32socket.h"
namespace Net
{
class Socket : public Win32::Win32Socket
{
    __DeclareClass(Socket);
};
}
#elif __linux__
#include "net/posix/posixsocket.h"
namespace Net
{
class Socket : public Posix::PosixSocket
{
    __DeclareClass(Socket);
};
}
#else
#error "Класс сокета не реализован на этой платформе"
#endif