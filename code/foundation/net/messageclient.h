#pragma once
#include "net/tcpclient.h"
#include "net/tcpmessagecodec.h"
#include "util/queue.h"
namespace Net
{
class MessageClient : public TcpClient
{
    __DeclareClass(MessageClient);
public:
    MessageClient();
    virtual ~MessageClient();
    virtual Result Connect();
    virtual void Disconnect();
    virtual bool Send();
    virtual const Ptr<IO::Stream>& GetSendStream();
    virtual bool Recv();
    virtual const Ptr<IO::Stream>& GetRecvStream();
private:
    TcpMessageCodec codec;
    Ptr<IO::Stream> sendMessageStream;
    Ptr<IO::Stream> recvMessageStream;
    Util::Queue<Ptr<IO::Stream> > msgQueue;
};
}