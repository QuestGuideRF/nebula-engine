#pragma once
#include "io/memorystream.h"
#include "io/binaryreader.h"
namespace Net
{
class TcpMessageCodec
{
public:
    TcpMessageCodec();
    virtual ~TcpMessageCodec();
    void EncodeToMessage(const Ptr<IO::Stream> & stream, const Ptr<IO::Stream> &output);
    void DecodeStream(const Ptr<IO::Stream> & stream);
    bool HasMessages();
    Util::Array<Ptr<IO::Stream> > DequeueMessages();
private:
    enum ReceiveState
    {
        HeaderData,
        MessageData
    };
    ReceiveState receiveState;
    Ptr<IO::Stream> headerStream;
    Ptr<IO::Stream> messageStream;
    SizeT messageSize;
    IndexT headerPosition;
    IndexT messagePostition;
    Util::Array<Ptr<IO::Stream> > completedMessages;
};
}