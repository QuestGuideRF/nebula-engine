//------------------------------------------------------------------------------
//  staticmessagehandler.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "messaging/staticmessagehandler.h"

namespace Messaging
{
//------------------------------------------------------------------------------
/**
*/
template<> void
StaticMessageHandler::Dispatch(const Ptr<Core::RefCounted>& obj, const Ptr<Message>& msg)
{
    n_error("StaticMessageHandler: необработанное сообщение (objclass=%s, msgclass=%s)!\n", 
        obj->GetClassName().AsCharPtr(), msg->GetClassName().AsCharPtr());
}

} // пространство имен
