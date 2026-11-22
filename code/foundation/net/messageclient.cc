//------------------------------------------------------------------------------
// messageclient.cc
// (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "net/messageclient.h"

namespace Net
{
__ImplementClass(Net::MessageClient,'MSGC',Net::TcpClient);

using namespace IO;
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
MessageClient::MessageClient()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
MessageClient::~MessageClient()
{
    if (this->IsConnected())
    {
        this->Disconnect();
    }
}

//------------------------------------------------------------------------------
/**
    Установите соединение с сервером. Подробности смотрите в TcpClient.
*/
TcpClient::Result
MessageClient::Connect()
{
    n_assert(!this->sendMessageStream.isvalid());
    n_assert(!this->recvMessageStream.isvalid());
    TcpClient::Result res = TcpClient::Connect();
    if (TcpClient::Success == res)
    {
        this->sendMessageStream = MemoryStream::Create();
    }
    return res;
}

//------------------------------------------------------------------------------
/**
*/
void
MessageClient::Disconnect()
{
    TcpClient::Disconnect();
    this->sendMessageStream = nullptr;
    this->recvMessageStream = nullptr;
}

//------------------------------------------------------------------------------
/**
*/
bool
MessageClient::Send()
{
    n_assert(this->sendMessageStream.isvalid());
    if (this->sendMessageStream->GetSize() == 0)
    {
        // нечего отправлять
        return Socket::Success;
    }
    this->sendMessageStream->SetAccessMode(Stream::ReadAccess);
    this->codec.EncodeToMessage(this->sendMessageStream,TcpClient::GetSendStream());
    this->sendMessageStream->SetSize(0);
    return TcpClient::Send();
}

//------------------------------------------------------------------------------
/**
*/
const Ptr<IO::Stream>&
MessageClient::GetSendStream()
{
    return this->sendMessageStream;
}

//------------------------------------------------------------------------------
/**
*/
bool
MessageClient::Recv()
{
    // сначала проверьте, доступны ли новые данные из сокета
    if (TcpClient::Recv())
    {
        // декодировать входящие данные в отдельное сообщение и ставить их в очередь
        // сообщение для дальнейшего использования
        this->codec.DecodeStream(this->recvStream);
        if (this->codec.HasMessages())
        {
            Array<Ptr<Stream> > msgArray = this->codec.DequeueMessages();
            IndexT i;
            for (i = 0; i < msgArray.Size(); i++)
            {
                this->msgQueue.Enqueue(msgArray[i]);
            }
        }
    }

    // установить самое старое полученное сообщение из очереди сообщений
    // как поток приема (мы либо возвращаем 1 полное сообщение,
    // или вообще ничего за вызов Recv()
    if (!this->msgQueue.IsEmpty())
    {
        this->recvMessageStream = this->msgQueue.Dequeue();
        return true;
    }
    else
    {
        // нет сообщений для возврата
        return false;
    }
}

//------------------------------------------------------------------------------
/**
*/
const Ptr<IO::Stream>&
MessageClient::GetRecvStream()
{
    return this->recvMessageStream;
}

} // Сеть пространства имен
