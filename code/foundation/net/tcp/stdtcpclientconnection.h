#pragma once
#include "core/refcounted.h"
#include "net/socket/ipaddress.h"
#include "io/stream.h"
#include "net/socket/socket.h"
namespace Net
{
class StdTcpClientConnection : public Core::RefCounted
{
    __DeclareClass(StdTcpClientConnection);
public:
    StdTcpClientConnection();
    virtual ~StdTcpClientConnection();
    virtual bool Connect(const Ptr<Socket>& s);
    bool IsConnected() const;
    virtual void Shutdown();
    const IpAddress& GetClientAddress() const;
    virtual Socket::Result Send();
    virtual Socket::Result Send(const Ptr<IO::Stream>& stream);
    virtual const Ptr<IO::Stream>& GetSendStream();
    virtual Socket::Result Recv();
    virtual const Ptr<IO::Stream>& GetRecvStream();
protected:
    Ptr<Socket> socket;
    Ptr<IO::Stream> sendStream;
    Ptr<IO::Stream> recvStream;
};
}