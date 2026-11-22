#pragma once
#include "io/streamwriter.h"
#include "io/binarywriter.h"
namespace Messaging
{
class Message;
class MessageWriter : public IO::StreamWriter
{
    __DeclareClass(MessageWriter);
public:
    MessageWriter();
    virtual void SetStream(const Ptr<IO::Stream>& s);
    void WriteMessage(const Ptr<Message>& msg);
private:
    Ptr<IO::BinaryWriter> binaryWriter;
};
}