#pragma once
#include "messaging/message.h"
#include "util/array.h"
namespace Messaging
{
class BatchMessage : public Message
{
    __DeclareClass(BatchMessage);
    __DeclareMsgId;
public:
    BatchMessage();
    void AddMessage(const Ptr<Message>& msg);
    const Util::Array<Ptr<Message> >& GetMessages() const;
private:
    Util::Array<Ptr<Message> > messages;
};
}