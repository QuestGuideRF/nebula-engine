#pragma once
#include "messaging/handlerthreadbase.h"
#include "threading/safequeue.h"
namespace Messaging
{
class RunThroughHandlerThread : public HandlerThreadBase
{
    __DeclareClass(RunThroughHandlerThread);
public:
    RunThroughHandlerThread();
    virtual void AddMessage(const Ptr<Message>& msg);
    virtual void CancelMessage(const Ptr<Message>& msg);
    virtual void DoWork();
private:
    Threading::SafeQueue<Ptr<Message> > msgQueue;
};
}