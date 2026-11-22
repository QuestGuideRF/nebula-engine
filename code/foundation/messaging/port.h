#pragma once
#include "core/refcounted.h"
#include "core/ptr.h"
#include "messaging/message.h"
#include "messaging/handler.h"
namespace Messaging
{
class Port : public Core::RefCounted
{
    __DeclareClass(Port);
public:
    virtual void SetupAcceptedMessages();
    void AttachHandler(const Ptr<Handler>& h);
    void RemoveHandler(const Ptr<Handler>& h);
    void RemoveAllHandlers();
    SizeT GetNumHandlers() const;
    const Ptr<Handler>& GetHandlerAtIndex(IndexT i) const;
    virtual void Send(const Ptr<Message>& msg);
    const Util::Array<const Id*>& GetAcceptedMessages() const;
    bool AcceptsMessage(const Id& msgId) const;
    virtual void HandleMessage(const Ptr<Messaging::Message>& msg);
protected:
    void RegisterMessage(const Id& msgId);
private:
    Util::Array<Ptr<Handler> > handlers;
    Util::Array<const Id*> acceptedMessageIds;
};
inline
SizeT
Port::GetNumHandlers() const
{
    return this->handlers.Size();
}
inline
void
Port::RegisterMessage(const Id& msgId)
{
    if (InvalidIndex == this->acceptedMessageIds.BinarySearchIndex(&msgId))
    {
        this->acceptedMessageIds.InsertSorted(&msgId);
    }
}
inline
const Util::Array<const Id*>&
Port::GetAcceptedMessages() const
{
    return this->acceptedMessageIds;
}
inline
const Ptr<Handler>&
Port::GetHandlerAtIndex(IndexT i) const
{
    return this->handlers[i];
}
inline
bool
Port::AcceptsMessage(const Id& msgId) const
{
    return (InvalidIndex != this->acceptedMessageIds.FindIndex(&msgId));
}
}