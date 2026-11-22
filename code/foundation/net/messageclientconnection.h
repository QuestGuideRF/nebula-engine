#pragma once
#include "net/tcpclientconnection.h"
#include "net/tcpmessagecodec.h"
#include "util/queue.h"
namespace Net
{
class MessageClientConnection : public TcpClientConnection
{
    __DeclareClass(MessageClientConnection);
public:
    MessageClientConnection();
    virtual ~MessageClientConnection();
    virtual bool Connect(const Ptr<Socket>& s);
    virtual void Shutdown();
    virtual Socket::Result Send();
    virtual Socket::Result Send(const Ptr<IO::Stream>& stream);
    virtual const Ptr<IO::Stream>& GetSendStream();
    virtual Socket::Result Recv();
    virtual const Ptr<IO::Stream>& GetRecvStream();
private:
    Ptr<IO::Stream> sendMessageStream;
    Ptr<IO::Stream> recvMessageStream;
    Util::Queue<Ptr<IO::Stream> > msgQueue;
    TcpMessageCodec codec;
};
}