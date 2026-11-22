//------------------------------------------------------------------------------
//  batchmessage.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "messaging/batchmessage.h"

namespace Messaging
{
__ImplementClass(Messaging::BatchMessage, 'MSGB', Messaging::Message);
__ImplementMsgId(BatchMessage);

using namespace Util;

//------------------------------------------------------------------------------
/**
*/
BatchMessage::BatchMessage() :
    messages(256, 256)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
BatchMessage::AddMessage(const Ptr<Message>& msg)
{
    this->messages.Append(msg);
}

//------------------------------------------------------------------------------
/**
*/
const Array<Ptr<Message> >&
BatchMessage::GetMessages() const
{
    return this->messages;
}

} // пространство имен
