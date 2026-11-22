#pragma once
#include "core/refcounted.h"
#include "net/socket/ipaddress.h"
#include "net/socket/socket.h"
#include "io/stream.h"
namespace Net
{
class StdTcpClient : public Core::RefCounted
{
    __DeclareClass(StdTcpClient);
public:
    enum Result
    {
        Error,
        Success,
        Connecting,
    };
    StdTcpClient();
    virtual ~StdTcpClient();
    void SetBlocking(bool b);
    bool IsBlocking() const;
    void SetServerAddress(const IpAddress& addr);
    const IpAddress& GetServerAddress() const;
    Result Connect();
    void Disconnect();
    bool IsConnected();
    bool Send();
    const Ptr<IO::Stream>& GetSendStream();
    bool Recv();
    const Ptr<IO::Stream>& GetRecvStream();
protected:
    Ptr<Socket> socket;
    IpAddress serverAddr;
    bool blocking;
    Ptr<IO::Stream> sendStream;
    Ptr<IO::Stream> recvStream;
    bool inConnectionState;
};
inline void
StdTcpClient::SetBlocking(bool b)
{
    this->blocking = b;
    if (this->socket.isvalid())
    {
        this->socket->SetBlocking(b);
    }
}
inline bool
StdTcpClient::IsBlocking() const
{
    return this->blocking;
}
inline void
StdTcpClient::SetServerAddress(const IpAddress& addr)
{
    this->serverAddr = addr;
}
inline const IpAddress&
StdTcpClient::GetServerAddress() const
{
    return this->serverAddr;
}
}