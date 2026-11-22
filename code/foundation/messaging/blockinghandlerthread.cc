//------------------------------------------------------------------------------
//  blockinghandlerthread.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "messaging/blockinghandlerthread.h"

namespace Messaging
{
__ImplementClass(Messaging::BlockingHandlerThread, 'BLHT', Messaging::HandlerThreadBase);

//------------------------------------------------------------------------------
/**
*/
BlockingHandlerThread::BlockingHandlerThread() :
    waitTimeout(0)
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Это добавляет новое сообщение в очередь сообщений потока.
*/
void
BlockingHandlerThread::AddMessage(const Ptr<Message>& msg)
{
    n_assert(msg.isvalid());
    this->msgQueue.Enqueue(msg);
}

//------------------------------------------------------------------------------
/**
    Это удаляет сообщение из очереди сообщений потока, независимо от того,
    своего государства.
*/
void
BlockingHandlerThread::CancelMessage(const Ptr<Message>& msg)
{
    n_assert(msg.isvalid());
    this->msgQueue.EraseMatchingElements(msg);
}

//------------------------------------------------------------------------------
/**
    This method is called by Thread::Stop() after setting the 
    stopRequest event and before waiting for the thread to stop.
*/
void
BlockingHandlerThread::EmitWakeupSignal()
{
    // разбудите нашу ветку, если она ждет сообщений
    this->msgQueue.Signal();
}

//------------------------------------------------------------------------------
/**
    Цикл обработки сообщений.
*/
void
BlockingHandlerThread::DoWork()
{
    this->ThreadOpenHandlers();
    while (!this->ThreadStopRequested())
    {
        // дождаться следующего сообщения или таймаута
        bool msgHandled = false;
        if (this->waitTimeout > 0)
        {
            this->msgQueue.WaitTimeout(this->waitTimeout);
        }
        else
        {
            this->msgQueue.Wait();
        }
        
        // обновить состояние отложенных сообщений
        msgHandled = this->ThreadUpdateDeferredMessages();

        // обрабатывать сообщения
        if (!this->msgQueue.IsEmpty())
        {
            Util::Array<Ptr<Message> > msgArray;
            this->msgQueue.DequeueAll(msgArray);
            msgHandled |= this->ThreadHandleMessages(msgArray);
        }

        // сигнал, если хотя бы одно сообщение было обработано
        if (msgHandled)
        {
            this->ThreadSignalMessageHandled();
        }

        // выполнять покадровое обновление прикрепленных обработчиков
        this->ThreadUpdateHandlers();
    }

    // очистка и выход из потока
    this->ThreadDiscardDeferredMessages();
    this->ThreadCloseHandlers();
}

} // пространство имен