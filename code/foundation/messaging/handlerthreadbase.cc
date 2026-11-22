//------------------------------------------------------------------------------
//  handlerthreadbase.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "messaging/handlerthreadbase.h"
#include "messaging/batchmessage.h"

namespace Messaging
{
__ImplementClass(Messaging::HandlerThreadBase, 'HTBS', Threading::Thread);

using namespace Util;
using namespace Threading;

//------------------------------------------------------------------------------
/**
*/
HandlerThreadBase::HandlerThreadBase()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Прикрепите обработчик сообщений к порту. Этот метод можно вызвать из
    любая нить.
*/
void
HandlerThreadBase::AttachHandler(const Ptr<Handler>& h)
{
    n_assert(h.isvalid());    
    this->handlersCritSect.Enter();    
    n_assert(InvalidIndex == this->handlers.FindIndex(h));
    this->handlers.Append(h);
    this->handlersCritSect.Leave();
}

//------------------------------------------------------------------------------
/**
    Удалить обработчик сообщений. Этот метод можно вызвать из любого потока.
*/
void
HandlerThreadBase::RemoveHandler(const Ptr<Handler>& h)
{
    n_assert(h.isvalid());

    this->handlersCritSect.Enter();    
    IndexT index = this->handlers.FindIndex(h);
    this->handlers.EraseIndex(index);
    this->handlersCritSect.Leave();
}

//------------------------------------------------------------------------------
/**
    Это очищает все прикрепленные обработчики сообщений.
*/
void
HandlerThreadBase::ClearHandlers()
{
    this->handlersCritSect.Enter();
    this->handlers.Clear();
    this->handlersCritSect.Leave();
}

//------------------------------------------------------------------------------
/**
    Дождитесь события открытия обработчиков (о котором будет сигнализировать 
    Метод ThreadOpenHandlers.
*/
void
HandlerThreadBase::WaitForHandlersOpened()
{
    n_assert(this->IsRunning());
    this->handlersOpenedEvent.Wait();
}

//------------------------------------------------------------------------------
/**
    Это добавляет новое сообщение в очередь сообщений потока. В базовом классе
    это сообщение пусто и должно быть реализовано в подклассе.
*/
void
HandlerThreadBase::AddMessage(const Ptr<Message>& msg)
{
    // пусто, переопределить в подклассе!
}

//------------------------------------------------------------------------------
/**
    Это удаляет сообщение из очереди сообщений потока, независимо от того,
    своего государства. Переопределить этот метод в подклассе.
*/
void
HandlerThreadBase::CancelMessage(const Ptr<Message>& msg)
{
    // пусто, переопределить в подклассе!
}

//------------------------------------------------------------------------------
/**
    This waits until the message given as argument has been handled. In 
    order for this message to work, the ThreadSignalMessageHandled() must
    be called from within the handler thread context. Note that 
    subclasses may override this method if they need to.
*/
void
HandlerThreadBase::WaitForMessage(const Ptr<Message>& msg)
{
    while (!msg->Handled())
    {
        this->msgHandledEvent.Wait();
    }
}

//------------------------------------------------------------------------------
/**
    Откройте обработчики прикрепленных сообщений. Этот метод должен быть вызван в
    начало потока обработчика.
*/
void
HandlerThreadBase::ThreadOpenHandlers()
{
    this->handlersCritSect.Enter();
    Array<Ptr<Handler> >::Iterator iter;
    for (iter = this->handlers.Begin(); iter < this->handlers.End(); iter++)
    {
        (*iter)->Open();
    }
    this->handlersCritSect.Leave();
    this->handlersOpenedEvent.Signal();
}

//------------------------------------------------------------------------------
/**
    Закройте обработчики прикрепленных сообщений. Этот метод должен называться правильно
    прежде чем поток обработчика закроется.
*/
void
HandlerThreadBase::ThreadCloseHandlers()
{
    this->handlersCritSect.Enter();
    Array<Ptr<Handler> >::Iterator iter;
    for (iter = this->handlers.Begin(); iter < this->handlers.End(); iter++)
    {
        (*iter)->Close();
    }
    this->handlersCritSect.Leave();
}

//------------------------------------------------------------------------------
/**
    Do per-frame update of attached handlers. This will open handlers
    which have been added late, and call the DoWork() method on 
    handlers from within the thread context.
*/
void
HandlerThreadBase::ThreadUpdateHandlers()
{
    this->handlersCritSect.Enter();
    Array<Ptr<Handler> >::Iterator iter;
    for (iter = this->handlers.Begin(); iter < this->handlers.End(); iter++)
    {
        // убедитесь, что обработчик открыт, может случиться так, что обработчики
        // добавляются в результате сообщения, и такой обработчик
        // возможно еще не открыли
        if (!(*iter)->IsOpen())
        {
            (*iter)->Open();
        }
        (*iter)->DoWork();
    }
    this->handlersCritSect.Leave();
}

//------------------------------------------------------------------------------
/**
    This checks every message in the deferred message array whether it
    has been handled yet, if yes, the message's actual handled flag
    will be set, and the message will be removed from the deferred
    handled array. If at least one message has been handled, the method
    will return true, if no message has been handled, the method returns
    false. If message have been handled, don't forget to call
    ThreadSignalMessageHandled() later!
*/
bool
HandlerThreadBase::ThreadUpdateDeferredMessages()
{
    bool msgHandled = false;
    IndexT deferredIndex;
    for (deferredIndex = this->deferredMessages.Size() - 1; deferredIndex != InvalidIndex; deferredIndex--)
    {
        Ptr<Message> msg = this->deferredMessages[deferredIndex];
        if (msg->DeferredHandled())
        {
            this->deferredMessages.EraseIndex(deferredIndex);
            msg->SetHandled(true);
            msgHandled = true;
        }
    }
    return msgHandled;
}

//------------------------------------------------------------------------------
/**
    При этом будут удалены все оставшиеся отложенные сообщения. Позвоните непосредственно перед отключением
    потока-обработчика.
*/
void
HandlerThreadBase::ThreadDiscardDeferredMessages()
{
    this->deferredMessages.Clear();
}

//------------------------------------------------------------------------------
/**
    Handle all message in the provided message array. Supports batched
    and deferred messages. Calls ThreadHandleSingleMessage(). If at least
    one message has been handled, the method returns true.
*/
bool
HandlerThreadBase::ThreadHandleMessages(const Array<Ptr<Message> >& msgArray)
{
    this->handlersCritSect.Enter();

    bool msgHandled = false;
    IndexT msgIdx;
    for (msgIdx = 0; msgIdx < msgArray.Size(); ++msgIdx)
    {
        const Ptr<Message>& msg = msgArray[msgIdx];
        // особый случай: пакетное сообщение?
        if (msg->CheckId(BatchMessage::Id))
        {
            // обрабатывать пакетные сообщения
            const Array<Ptr<Message> >& msgBatch = msg.cast<BatchMessage>()->GetMessages();
            IndexT msgBatchIndex;
            for (msgBatchIndex = 0; msgBatchIndex < msgBatch.Size(); msgBatchIndex++)
            {
                msgHandled |= this->ThreadHandleSingleMessage(msgBatch[msgBatchIndex]);
            }
            msg->SetHandled(true);
            msgHandled = true;
        }
        else
        {
            // нормальное сообщение
            msgHandled |= this->ThreadHandleSingleMessage(msg);
        }
    }
    this->handlersCritSect.Leave();
    return msgHandled;
}

//------------------------------------------------------------------------------
/**
    Handle a single message, called by ThreadHandleMessages(). Return true
    if message has been handled.
    This method MUST be called from ThreadHandleMessages(), since this
    method will not explicitely take the handlers array critical section.
*/
bool
HandlerThreadBase::ThreadHandleSingleMessage(const Ptr<Message>& msg)
{
    bool msgHandled = false;

    // пусть каждый обработчик просмотрит сообщение
    IndexT i;
    for (i = 0; i < this->handlers.Size(); i++)
    {    
        const Ptr<Handler>& curHandler = this->handlers[i];
        
        // открыть обработчик по требованию (обработчик может иметь
        // был добавлен в результате вызова сообщения,
        // так что давайте убедимся, что он сможет справиться 
        // сообщение как можно скорее
        if (!curHandler->IsOpen())
        {
            curHandler->Open();
        }
    
        // пусть обработчик обработает текущее сообщение
        if (curHandler->HandleMessage(msg))
        {
            // сообщение было принято этим обработчиком
            if (msg->IsDeferred())
            {
                // сообщение было отложено обработчиком
                // и будет обработано позже,
                // поместить сообщение в отложенную очередь
                // и обработать позже
                this->deferredMessages.Append(msg);
            }
            else
            {
                // сообщение обработано нормально
                msg->SetHandled(true);
                msgHandled = true;
            }
            
            // прерваться после того, как первый обработчик примет сообщение
            break;
        }
    }
    return msgHandled;
}

//------------------------------------------------------------------------------
/**
    Сигнализировать флаг обработки сообщения. Вызовите этот метод один раз в
    цикл-обработчик, если либо ThreadUpdateDeferredMessages, либо 
    ThreadHandleMessages возвращает true!
*/
void
HandlerThreadBase::ThreadSignalMessageHandled()
{
    this->msgHandledEvent.Signal();    
}

} // пространство имен