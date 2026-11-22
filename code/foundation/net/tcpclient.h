#pragma once
#include "core/config.h"
#if (__WIN32__ || __OSX__ || __APPLE__ || __linux__)
#include "net/tcp/stdtcpclient.h"
namespace Net
{
class TcpClient : public StdTcpClient
{
    __DeclareClass(TcpClient);
};
}
#else
#error "Net::TcpClient не реализован на этой платформе!"
#endif