//------------------------------------------------------------------------------
//  asyncport.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "messaging/asyncport.h"

namespace Messaging
{
__ImplementClass(Messaging::AsyncPort, 'ASPT', Core::RefCounted);

using namespace Util;
using namespace Threading;

//------------------------------------------------------------------------------
/**
*/
AsyncPort::AsyncPort() :
    isOpen(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
AsyncPort::~AsyncPort()
{
    if (this->IsOpen())
    {
        this->Close();
    }
    this->thread = nullptr;
}

//------------------------------------------------------------------------------
/**
    Откройте асинхронный порт. Перед асинхронным портом необходимо указать допустимое имя.
    открыт. Сообщения можно отправлять только на открытый порт.
*/
void
AsyncPort::Open()
{
    n_assert(!this->IsOpen());
    n_assert(this->thread.isvalid());

    // запустите поток обработчика и подождите, пока обработчики не откроются
    this->thread->Start();
    this->thread->WaitForHandlersOpened();

    this->isOpen = true;
}

//------------------------------------------------------------------------------
/**
    Закрывает асинхронный порт.
*/
void
AsyncPort::Close()
{
    n_assert(this->IsOpen());
    this->thread->Stop();
    this->thread->ClearHandlers();
    this->thread = nullptr;
    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
    Добавьте обработчик сообщения, его можно вызвать перед обработчиком
    поток запускается или в любое время позже.
*/
void
AsyncPort::AttachHandler(const Ptr<Handler>& h)
{
    this->thread->AttachHandler(h);
}

//------------------------------------------------------------------------------
/**
    Динамическое удаление обработчика сообщений.
*/
void
AsyncPort::RemoveHandler(const Ptr<Handler>& h)
{
    this->thread->RemoveHandler(h);
}

//------------------------------------------------------------------------------
/**
    Handle an asynchronous message and return immediately. If the caller
    expects any results from the message he can poll with the AsyncPort::Peek()
    method, or he may wait for the message to be handled with the 
    AsyncPort::Wait() method.
*/
void
AsyncPort::SendInternal(const Ptr<Message>& msg)
{
    n_assert(msg.isvalid());
    n_assert(this->thread.isvalid());
    n_assert(!msg->Handled());
    this->thread->AddMessage(msg);
}

//------------------------------------------------------------------------------
/**
    Отправьте асинхронное сообщение и подождите, пока сообщение не будет отправлено.
    обработано.
*/
void
AsyncPort::SendWaitInternal(const Ptr<Message>& msg)
{
    this->Send(msg);
    this->Wait(msg);
}

//------------------------------------------------------------------------------
/**
    This method peeks whether a message has already been handled. If the
    caller expects any return arguments from the message handling it 
    can use this message to check whether the results are ready using
    this non-blocking method. The caller can also wait for the results
    to become ready using the Wait() method.
*/
bool
AsyncPort::PeekInternal(const Ptr<Message>& msg)
{
    n_assert(msg.isvalid());
    return msg->Handled();
}

//------------------------------------------------------------------------------
/**
    Этот метод отменит ожидающее сообщение.
*/
void
AsyncPort::CancelInternal(const Ptr<Message>& msg)
{
    n_assert(msg.isvalid());
    n_assert(this->thread.isvalid());
    this->thread->CancelMessage(msg);
}

//------------------------------------------------------------------------------
/**
    Этот метод будет ждать, пока сообщение не будет обработано. Если звонящий
    ожидает любых возвращаемых аргументов от обработки сообщения, которую он может использовать
    этот метод, чтобы дождаться результатов.
*/
void
AsyncPort::WaitInternal(const Ptr<Message>& msg)
{
    n_assert(msg.isvalid());
    n_assert(this->thread.isvalid());

    // подождите, только если сообщение еще не обработано
    if (!msg->Handled())
    {
        this->thread->WaitForMessage(msg);
        n_assert(msg->Handled());
    }
}

} // пространство имен
