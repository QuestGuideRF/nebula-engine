#pragma once
#include "messaging/message.h"
#include "messaging/handler.h"
#include "messaging/handlerthreadbase.h"
namespace Messaging
{
class AsyncPort : public Core::RefCounted
{
    __DeclareClass(AsyncPort);
public:
    AsyncPort();
    virtual ~AsyncPort();
    void SetHandlerThread(const Ptr<HandlerThreadBase>& handlerThread);
    const Ptr<HandlerThreadBase>& GetHandlerThread() const;
    virtual void AttachHandler(const Ptr<Handler>& h);
    virtual void RemoveHandler(const Ptr<Handler>& h);
    virtual void Open();
    virtual void Close();
    bool IsOpen() const;
    template<class MESSAGETYPE> void Send(const Ptr<MESSAGETYPE>& msg);
    template<class MESSAGETYPE> void SendWait(const Ptr<MESSAGETYPE>& msg);
    template<class MESSAGETYPE> void Wait(const Ptr<MESSAGETYPE>& msg);
    template<class MESSAGETYPE> bool Peek(const Ptr<MESSAGETYPE>& msg);
    template<class MESSAGETYPE> void Cancel(const Ptr<MESSAGETYPE>& msg);
private:
    void SendInternal(const Ptr<Message>& msg);
    void SendWaitInternal(const Ptr<Message>& msg);
    void WaitInternal(const Ptr<Message>& msg);
    bool PeekInternal(const Ptr<Message>& msg);
    void CancelInternal(const Ptr<Message>& msg);
private:
    void ClearHandlers();
    Ptr<HandlerThreadBase> thread;
    bool isOpen;
};
inline void
AsyncPort::SetHandlerThread(const Ptr<HandlerThreadBase>& handlerThread)
{
    n_assert(!this->IsOpen());
    this->thread = handlerThread;
}
inline const Ptr<HandlerThreadBase>&
AsyncPort::GetHandlerThread() const
{
    return this->thread;
}
inline bool
AsyncPort::IsOpen() const
{
    return this->isOpen;
}
template<class MESSAGETYPE> inline void
AsyncPort::Send(const Ptr<MESSAGETYPE>& msg)
{
    this->SendInternal((const Ptr<Messaging::Message>&)msg);
}
template<class MESSAGETYPE> inline void
AsyncPort::SendWait(const Ptr<MESSAGETYPE>& msg)
{
    this->SendWaitInternal((const Ptr<Messaging::Message>&)msg);
}
template<class MESSAGETYPE> inline void
AsyncPort::Wait(const Ptr<MESSAGETYPE>& msg)
{
    this->WaitInternal((const Ptr<Messaging::Message>&)msg);
}
template<class MESSAGETYPE> inline bool
AsyncPort::Peek(const Ptr<MESSAGETYPE>& msg)
{
    return this->PeekInternal((const Ptr<Messaging::Message>&)msg);
}
template<class MESSAGETYPE> inline void
AsyncPort::Cancel(const Ptr<MESSAGETYPE>& msg)
{
    this->CancelInternal((const Ptr<Messaging::Message>&)msg);
}
}