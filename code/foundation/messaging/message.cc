//------------------------------------------------------------------------------
//  message.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "messaging/message.h"

namespace Messaging
{
__ImplementClass(Messaging::Message, 'MSG_', Core::RefCounted);
__ImplementMsgId(Message);

//------------------------------------------------------------------------------
/**
*/
Message::Message() :
    handled(0),
    deferred(false),
    deferredHandled(false),
    distribute(true)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
Message::Encode(const Ptr<IO::BinaryWriter>& writer)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
Message::Decode(const Ptr<IO::BinaryReader>& reader)
{
    // empty
}

} // Сообщение пространства имен
