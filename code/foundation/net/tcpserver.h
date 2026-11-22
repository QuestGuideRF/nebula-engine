#pragma once
#include "core/config.h"
#if (__WIN32__ || __OSX__ || __APPLE__ || __linux__)
#include "net/tcp/stdtcpserver.h"
namespace Net
{
class TcpServer : public StdTcpServer
{
    __DeclareClass(TcpServer);
};
}
#else
#error "Net::TcpServer не реализован на этой платформе!"
#endif