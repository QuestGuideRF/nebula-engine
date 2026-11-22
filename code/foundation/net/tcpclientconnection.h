#pragma once
#include "core/config.h"
#if (__WIN32__ || __OSX__ || __APPLE__ || __linux__)
#include "net/tcp/stdtcpclientconnection.h"
namespace Net
{
class TcpClientConnection : public StdTcpClientConnection
{
    __DeclareClass(TcpClientConnection);
};
}
#else
#error "Net::TcpClientConnection не реализован на этой платформе!"
#endif