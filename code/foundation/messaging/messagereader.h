#pragma once
#include "io/binaryreader.h"
namespace Messaging
{
class Message;
class MessageReader : public IO::StreamReader
{
    __DeclareClass(MessageReader);
public:
    MessageReader();
    virtual void SetStream(const Ptr<IO::Stream>& s);
    Message* ReadMessage();
private:
    Ptr<IO::BinaryReader> binaryReader;
};
}