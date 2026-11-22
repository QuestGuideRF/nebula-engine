//------------------------------------------------------------------------------
//  messageclientconnection.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "net/messageclientconnection.h"
#include "io/memorystream.h"
//------------------------------------------------------------------------------

using namespace IO;
using namespace Util;

namespace Net
{
__ImplementClass(Net::MessageClientConnection,'MGCC',Net::TcpClientConnection);

//------------------------------------------------------------------------------
/**
    Конструктор 
*/
MessageClientConnection::MessageClientConnection()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Деструктор  
*/
MessageClientConnection::~MessageClientConnection()
{
    n_assert(!this->IsConnected());
}

//------------------------------------------------------------------------------
/**
    Подключитесь с помощью предоставленного разъема. Подробности см. в разделе TcpClientConnection.
*/
bool
MessageClientConnection::Connect(const Ptr<Socket>& s)
{
    n_assert(!this->sendMessageStream.isvalid());
    n_assert(!this->recvMessageStream.isvalid());
    n_assert(this->msgQueue.IsEmpty());
    bool res = TcpClientConnection::Connect(s);
    if (res)
    {
        this->sendMessageStream = MemoryStream::Create();
    }
    return res;
}

//------------------------------------------------------------------------------
/**
    Это разрывает текущее соединение.
*/
void
MessageClientConnection::Shutdown()
{
    n_assert(this->msgQueue.IsEmpty());
    TcpClientConnection::Shutdown();
    this->sendMessageStream = nullptr;
    this->recvMessageStream = nullptr;
}

//------------------------------------------------------------------------------
/**
    Writes accumulated send stream into a message container and sends
    this container.
*/
Socket::Result
MessageClientConnection::Send()
{
    return this->Send(this->sendMessageStream);
}

//------------------------------------------------------------------------------
/**
    Writes given stream into a message container and sends this container.
*/
Socket::Result
MessageClientConnection::Send(const Ptr<IO::Stream> &stream)
{
    n_assert(stream.isvalid());
    if (stream->GetSize() == 0)
    {
        // нечего отправлять
        return Socket::Success;
    }

    stream->SetAccessMode(Stream::ReadAccess);
    this->codec.EncodeToMessage(stream, this->sendStream);

    n_printf("MessageClientConnection: отправка %ld байт\n", this->sendStream->GetSize());

    return TcpClientConnection::Send(sendStream);
}

//------------------------------------------------------------------------------
/**
    Возвращает поток, в который записываются данные для отправки.
*/
const Ptr<IO::Stream>&
MessageClientConnection::GetSendStream()
{
    return this->sendMessageStream;
}

//------------------------------------------------------------------------------
/**
    Receive data from the clients, but returns only Success, if a
    complete message was available. This message will either return one
    complete message, or none at all.
*/
Socket::Result
MessageClientConnection::Recv()
{
    // сначала проверьте, доступны ли новые данные из сокета...
    Socket::Result returnValue = TcpClientConnection::Recv();
    if (Socket::Success == returnValue)
    {
        // декодировать входящие данные в отдельные сообщения и ставить в очередь 
        // сообщения для дальнейшего использования
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
        else
        {
            // Соединение блокируется, если получены данные, но нет полного сообщения
            returnValue = Socket::WouldBlock;
        }
    }

    // теперь напишите самое старое сообщение из очереди сообщений
    // в поток приема, так что ровно один 
    if (!this->msgQueue.IsEmpty())
    {
        this->recvMessageStream = this->msgQueue.Dequeue();
        
        // пока сообщения доступны и не возникло ошибок,
        // установите результат на Успех
        if (Socket::Error != returnValue)
        {
            returnValue = Socket::Success;
        }
    }
    else
    {
        // нет доступных сообщений
        this->recvMessageStream = nullptr;
        if (Socket::Error != returnValue)
        {
            returnValue = Socket::WouldBlock;
        }
    }
    return returnValue;
}

//------------------------------------------------------------------------------
/**
    Возвращает поток с полученными данными   
*/
const Ptr<IO::Stream>&
MessageClientConnection::GetRecvStream()
{
    return this->recvMessageStream;
}

} // Сеть пространства имен
