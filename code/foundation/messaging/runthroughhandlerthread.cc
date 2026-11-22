//------------------------------------------------------------------------------
//  runthroughhandlerthread.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "messaging/runthroughhandlerthread.h"

namespace Messaging
{
__ImplementClass(Messaging::RunThroughHandlerThread, 'RTHT', Messaging::HandlerThreadBase);

//------------------------------------------------------------------------------
/**
*/
RunThroughHandlerThread::RunThroughHandlerThread()
{
    this->msgQueue.SetSignalOnEnqueueEnabled(false);
}

//------------------------------------------------------------------------------
/**
    Это добавляет новое сообщение в очередь сообщений потока.
*/
void
RunThroughHandlerThread::AddMessage(const Ptr<Message>& msg)
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
RunThroughHandlerThread::CancelMessage(const Ptr<Message>& msg)
{
    n_assert(msg.isvalid());
    this->msgQueue.EraseMatchingElements(msg);
}

//------------------------------------------------------------------------------
/**
    Цикл обработки сообщений.
*/
void
RunThroughHandlerThread::DoWork()
{
    this->ThreadOpenHandlers();
    while (!this->ThreadStopRequested())
    {
        bool msgHandled = false;

        // не ждать сообщения, а создать поток на некоторых платформах
        // (выход важен только на Wii и не имеет значения
        // что-нибудь на других платформах)
        Thread::YieldThread();

        // обновить состояние отложенных сообщений
        msgHandled = this->ThreadUpdateDeferredMessages();

        // обрабатывать сообщения
        if (!this->msgQueue.IsEmpty())
        {
            Util::Array<Ptr<Message> > msgArray;
            msgArray.Reserve(this->msgQueue.Size());
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