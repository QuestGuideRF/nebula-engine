#pragma once
#include "threading/thread.h"
#include "threading/event.h"
#include "threading/criticalsection.h"
#include "messaging/handler.h"
#include "messaging/message.h"
namespace Messaging
{
class HandlerThreadBase : public Threading::Thread
{
    __DeclareClass(HandlerThreadBase);
public:
    HandlerThreadBase();
    void AttachHandler(const Ptr<Handler>& h);
    void RemoveHandler(const Ptr<Handler>& h);
    void ClearHandlers();
    void WaitForHandlersOpened();
    virtual void AddMessage(const Ptr<Message>& msg);
    virtual void CancelMessage(const Ptr<Message>& msg);
    virtual void WaitForMessage(const Ptr<Message>& msg);
protected:
    void ThreadOpenHandlers();
    void ThreadCloseHandlers();
    void ThreadUpdateHandlers();
    bool ThreadUpdateDeferredMessages();
    void ThreadDiscardDeferredMessages();
    bool ThreadHandleMessages(const Util::Array<Ptr<Message> >& msgArray);
    bool ThreadHandleSingleMessage(const Ptr<Message>& msg);
    void ThreadSignalMessageHandled();
    Threading::Event msgHandledEvent;
    Threading::Event handlersOpenedEvent;
    Threading::CriticalSection handlersCritSect;
    Util::Array<Ptr<Handler> > handlers;
    Util::Array<Ptr<Message> > deferredMessages;
};
}