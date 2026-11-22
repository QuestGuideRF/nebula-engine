//------------------------------------------------------------------------------
//  handler.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "messaging/handler.h"

namespace Messaging
{
__ImplementClass(Messaging::Handler, 'MHDL', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
Handler::Handler() :
    isOpen(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
Handler::~Handler()
{
    n_assert(!this->IsOpen());
}

//------------------------------------------------------------------------------
/**
    Откройте обработчик. Этот метод вызывается один раз после того, как обработчик был
    подключен к порту и перед первым вызовом HandleMessage().
*/
void
Handler::Open()
{
    n_assert(!this->IsOpen());
    this->isOpen = true;
}

//------------------------------------------------------------------------------
/**
    Закройте обработчик. Этот метод вызывается один раз перед обработчиком
    оторван от порта.
*/
void
Handler::Close()
{
    n_assert(this->IsOpen());
    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
    Выведите этот метод в подклассе для обработки определенных сообщений. Метод
    должен возвращать true только в том случае, если сообщение было обработано.
*/
bool
Handler::HandleMessage(const Ptr<Message>& msg)
{
    n_assert(msg.isvalid());
    n_assert(this->isOpen);
    return false;
}

//------------------------------------------------------------------------------
/**
    This is an optional method for handlers which need to do continuous
    work (like a render thread message handler). This message will
    be called after messages have been handled.
*/
void
Handler::DoWork()
{
    // пусто, переопределить в подклассе
    n_assert2(this->isOpen, this->GetRtti()->GetName().AsCharPtr());
}

} // пространство имен
