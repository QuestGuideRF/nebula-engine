#pragma once
#include "messaging/handlerthreadbase.h"
#include "threading/safequeue.h"
namespace Messaging
{
class BlockingHandlerThread : public HandlerThreadBase
{
    __DeclareClass(BlockingHandlerThread);
public:
    BlockingHandlerThread();
    void SetWaitTimeout(int milliSec);
    int GetWaitTimeout() const;
    virtual void AddMessage(const Ptr<Message>& msg);
    virtual void CancelMessage(const Ptr<Message>& msg);
    virtual void EmitWakeupSignal();
    virtual void DoWork();
private:
    int waitTimeout;
    Threading::SafeQueue<Ptr<Message> > msgQueue;
};
inline void
BlockingHandlerThread::SetWaitTimeout(int milliSec)
{
    n_assert(!this->IsRunning());
    this->waitTimeout = milliSec;
}
inline int
BlockingHandlerThread::GetWaitTimeout() const
{
    return this->waitTimeout;
}
}