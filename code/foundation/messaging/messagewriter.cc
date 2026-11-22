//------------------------------------------------------------------------------
//  messagewriter.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "messaging/messagewriter.h"
#include "messaging/message.h"

namespace Messaging
{
__ImplementClass(Messaging::MessageWriter, 'MGWR', IO::StreamWriter);

using namespace IO;
using namespace Util;
using namespace Core;

//------------------------------------------------------------------------------
/**
*/
MessageWriter::MessageWriter()
{
    this->binaryWriter = BinaryWriter::Create();
}

//------------------------------------------------------------------------------
/**
*/
void
MessageWriter::SetStream(const Ptr<IO::Stream>& s)
{
    n_assert(s.isvalid());
    StreamWriter::SetStream(s);
    this->binaryWriter->SetStream(s);
}

//------------------------------------------------------------------------------
/**
    Записывает полное сообщение в поток. Сначала идентификатор класса FourCC
    сообщение будет написано, затем сообщение будет задано
    для записи собственных данных в поток.
*/
void
MessageWriter::WriteMessage(const Ptr<Message>& msg)
{
    n_assert(msg.isvalid());
    this->binaryWriter->WriteUInt(msg->GetClassFourCC().AsUInt());
    msg->Encode(this->binaryWriter);
}

} // пространство имен