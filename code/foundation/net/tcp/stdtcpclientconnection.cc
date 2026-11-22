//------------------------------------------------------------------------------
//  stdtcpclientconnection.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "net/tcp/stdtcpclientconnection.h"
#include "io/memorystream.h"

namespace Net
{
__ImplementClass(Net::StdTcpClientConnection, 'STCC', Core::RefCounted);

using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
StdTcpClientConnection::StdTcpClientConnection()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
StdTcpClientConnection::~StdTcpClientConnection()
{
    this->Shutdown();
}

//------------------------------------------------------------------------------
/**
*/
bool
StdTcpClientConnection::Connect(const Ptr<Socket>& sock)
{
    n_assert(!this->socket.isvalid());
    n_assert(sock.isvalid());
    n_assert(sock->IsOpen());
    
    // проверьте, действительно ли сокет подключен
    if (sock->IsConnected())
    {
        this->socket = sock;
        this->socket->SetBlocking(true);
        this->socket->SetNoDelay(true);
        this->sendStream = MemoryStream::Create();
        this->recvStream = MemoryStream::Create();
        n_printf("Клиент с адреса addr=%s подключен!\n", 
            this->socket->GetAddress().GetHostAddr().AsCharPtr());
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
bool
StdTcpClientConnection::IsConnected() const
{
    if (this->socket.isvalid())
    {
        return this->socket->IsConnected();
    }
    else
    {
        return false;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
StdTcpClientConnection::Shutdown()
{
    if (this->socket.isvalid())
    {
        this->socket->Close();
        this->socket = nullptr;
    }
    this->sendStream = nullptr;
    this->recvStream = nullptr;
}

//------------------------------------------------------------------------------
/**
*/
const IpAddress&
StdTcpClientConnection::GetClientAddress() const
{
    n_assert(this->socket.isvalid());
    return this->socket->GetAddress();
}

//------------------------------------------------------------------------------
/**
*/
Socket::Result
StdTcpClientConnection::Send(const Ptr<Stream>& stream)
{
    n_assert(stream.isvalid());
    if (stream->GetSize() == 0)
    {
        // нечего отправлять
        return Socket::Success;
    }
    
    Socket::Result res = Socket::Success;
    stream->SetAccessMode(Stream::ReadAccess);
    if (stream->Open())
    {
        // мы не можем превышать максимальный размер сообщения...
        // поэтому нам, возможно, придется разделить отправляемые данные на
        // несколько пакетов
        SizeT maxMsgSize = this->socket->GetMaxMsgSize();
        Stream::Size sendSize = stream->GetSize();
        n_assert(sendSize < INT_MAX);
        uchar* ptr = (uchar*) stream->Map();
        SizeT overallBytesSent = 0;
        while ((Socket::Success == res) && (overallBytesSent < sendSize))
        {
            SizeT bytesToSend = (SizeT)sendSize - overallBytesSent;
            if (bytesToSend > maxMsgSize)
            {
                bytesToSend = maxMsgSize;
            }
            SizeT bytesSent = 0;
            res = this->socket->Send(ptr, bytesToSend, bytesSent);
            ptr += bytesSent;
            overallBytesSent += bytesSent;
        }
        stream->Unmap();
        stream->Close();
    }
    return res;
}

//------------------------------------------------------------------------------
/**
*/
Socket::Result
StdTcpClientConnection::Send()
{
    n_assert(this->sendStream.isvalid());
    Socket::Result res = this->Send(this->sendStream);
    this->sendStream->SetSize(0);
    return res;
}

//------------------------------------------------------------------------------
/**
*/
Socket::Result
StdTcpClientConnection::Recv()
{
    n_assert(this->recvStream.isvalid());
    this->recvStream->SetAccessMode(Stream::WriteAccess);
    this->recvStream->SetSize(0);
    Socket::Result res = Socket::Success;
    if (this->recvStream->Open())
    {
        // ПРИМЕЧАНИЕ. Следующий цикл гарантирует, что Recv()
        // никогда не блокирует
        uchar buf[1024];
        while ((Socket::Success == res) && (this->socket->HasRecvData()))
        {
            SizeT bytesReceived = 0;
            res = this->socket->Recv(&buf, sizeof(buf), bytesReceived);
            if ((bytesReceived > 0) && (Socket::Success == res))
            {
                this->recvStream->Write(buf, bytesReceived);
            }
        }
        this->recvStream->Close();
    }
    this->recvStream->SetAccessMode(Stream::ReadAccess);
    if (Socket::Success == res)
    {
        if (this->recvStream->GetSize() > 0)
        {
            return Socket::Success;
        }
        else
        {
            return Socket::WouldBlock;
        }
    }
    else
    {
        return res;
    }
}

//------------------------------------------------------------------------------
/**
*/
const Ptr<Stream>&
StdTcpClientConnection::GetSendStream()
{
    return this->sendStream;
}

//------------------------------------------------------------------------------
/**
*/
const Ptr<Stream>&
StdTcpClientConnection::GetRecvStream()
{
    return this->recvStream;
}

} // Сеть пространства имен
