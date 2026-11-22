//------------------------------------------------------------------------------
//  stdtcpclient.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "net/tcp/stdtcpclient.h"
#include "io/memorystream.h"

namespace Net
{
__ImplementClass(Net::StdTcpClient, 'STCL', Core::RefCounted);

using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
StdTcpClient::StdTcpClient() :
    blocking(true),
    inConnectionState(false)
{
    // создавать потоки отправки и получения
    this->sendStream = MemoryStream::Create();
    this->recvStream = MemoryStream::Create();
}

//------------------------------------------------------------------------------
/**
*/
StdTcpClient::~StdTcpClient()
{
    if (this->IsConnected())
    {
        this->Disconnect();
    }
    this->sendStream = nullptr;
    this->recvStream = nullptr;
}

//------------------------------------------------------------------------------
/**
    Establish a connection with the server. If the client is set to
    non-blocking at the time this method is called, it will return immediately
    with the result Connecting. To check if the connection is standing, just
    call Connect() again in intervals which will eventually return Success.
    On a blocking client, connect returns after a connection has been established,
    or with a time out when no connection could be established.
*/
StdTcpClient::Result
StdTcpClient::Connect()
{
    n_assert(!this->socket.isvalid());
    n_assert(this->sendStream.isvalid());
    n_assert(this->recvStream.isvalid());

    // создайте новый сокет и попробуйте подключиться к серверу
    this->socket = Socket::Create();
    if (this->socket->Open(Socket::TCP))
    {
        this->socket->SetAddress(this->serverAddr);
        this->socket->SetBlocking(this->blocking);
        this->socket->SetReUseAddr(true);
        this->socket->SetNoDelay(true);

        /*
        n_printf("StdTcpClient: подключение к порту хоста '%s(%s)' '%d'...\n", 
            this->serverAddr.GetHostName().AsCharPtr(),
            this->serverAddr.GetHostAddr().AsCharPtr(),
            this->serverAddr.GetPort());
        */

        Socket::Result res = this->socket->Connect();
        if (Socket::Error == res)
        {
            n_printf("StdTcpClient: не удалось подключиться к порту хоста «%s(%s)» «%d»!.\n",
                this->serverAddr.GetHostName().AsCharPtr(),
                this->serverAddr.GetHostAddr().AsCharPtr(),
                this->serverAddr.GetPort());
            this->socket = nullptr;
            return Error;
        }
        else if (Socket::Success == res)
        {
            // n_printf("StdTcpClient: соединение установлено\n");
            this->inConnectionState = true;
            return Success;
        }
        else
        {
            // n_printf("StdTcpClient: подключение...\n");
            return Connecting;
        }
    }
    n_printf("StdTcpClient: не удалось открыть сокет!\n");
    this->socket = nullptr;
    return Error;
}

//------------------------------------------------------------------------------
/**
    Return true if the socket is currently connected. This will actually
    probe the connection using a select().
*/
bool
StdTcpClient::IsConnected()
{
    if (this->socket.isvalid())
    {
        if (this->socket->IsConnected())
        {
            return true;
        }
        else
        {
            this->Disconnect();
            return false;
        }
    }
    else
    {
        return false;
    }
}

//------------------------------------------------------------------------------
/**
    Это разрывает текущее соединение.
*/
void
StdTcpClient::Disconnect()
{
    if (this->inConnectionState)
    {
        n_assert(this->socket.isvalid());
        this->socket->Close();
        this->socket = nullptr;
    }
    this->inConnectionState = false;
}

//------------------------------------------------------------------------------
/**
*/
bool
StdTcpClient::Send()
{
    n_assert(this->sendStream.isvalid());
    if (this->sendStream->GetSize() == 0)
    {
        // нечего отправлять
        return true;
    }

    this->sendStream->SetAccessMode(Stream::ReadAccess);
    if (this->sendStream->Open())
    {
        // переведите сокет в режим блокировки, чтобы, если
        // исходящие данные не помещаются в буфер отправки
        // сокет будет блокироваться до тех пор, пока не будет записан следующий блок данных
        bool wasBlocking = this->socket->GetBlocking();
        if (!wasBlocking)
        {
            this->socket->SetBlocking(true);
        }

        // мы не можем превышать максимальный размер сообщения...
        // поэтому нам, возможно, придется разделить отправляемые данные на
        // несколько пакетов
        SizeT maxMsgSize = this->socket->GetMaxMsgSize();
        Stream::Size sendSize = this->sendStream->GetSize();
        n_assert(sendSize < INT_MAX);
        uchar* ptr = (uchar*) this->sendStream->Map();
        SizeT overallBytesSent = 0;
        Socket::Result socketResult = Socket::Success;
        while ((Socket::Success == socketResult) && (overallBytesSent < sendSize))
        {
            SizeT bytesToSend = (SizeT)sendSize - overallBytesSent;
            if (bytesToSend > maxMsgSize)
            {
                bytesToSend = maxMsgSize;
            }
            SizeT bytesSent = 0;
            socketResult = this->socket->Send(ptr, bytesToSend, bytesSent);
            if (Socket::Success == socketResult)
            {
                ptr += bytesSent;
                overallBytesSent += bytesSent;
            }
            else
            {
                // отправка не удалась, попробуйте переподключиться и отправить еще раз
                bool clientWasBlocking = this->blocking;
                this->SetBlocking(true);
                this->Disconnect();
                Result connectResult = this->Connect();
                if (Success == connectResult)
                {
                    this->socket->SetBlocking(true);
                    n_printf("StdTcpClient повторно подключен!\n");
                    socketResult = Socket::Success;
                }
                else if (Connecting == connectResult)
                {
                    n_printf("Повторное подключение StdTcpClient не удалось выполнить при «подключении» (невозможно)\n");
                }
                else
                {
                    n_printf("Повторное подключение StdTcpClient не удалось с ошибкой\n");
                }
                this->SetBlocking(clientWasBlocking);
            }
        }
        this->sendStream->Unmap();
        if (!wasBlocking)
        {
            this->socket->SetBlocking(false);
        }
        this->sendStream->Close();
        this->sendStream->SetSize(0);
        if ((Socket::Success == socketResult) && (overallBytesSent == sendSize))
        {
            return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
bool
StdTcpClient::Recv()
{
    n_assert(this->recvStream.isvalid());
    this->recvStream->SetAccessMode(Stream::WriteAccess);
    this->recvStream->SetSize(0);
    if (this->recvStream->Open())
    {
        uchar buf[1024];
        Socket::Result res = Socket::Success;
        do
        {
            // ПРИМЕЧАНИЕ. Если это блокирующий клиент, первый вызов
            // to Recv() будет блокироваться до тех пор, пока данные не станут доступны.
            SizeT bytesReceived = 0;
            res = this->socket->Recv(&buf, sizeof(buf), bytesReceived);
            if ((bytesReceived > 0) && (Socket::Success == res))
            {
                this->recvStream->Write(buf, bytesReceived);
            }
        }
        while ((Socket::Success == res) && this->socket->HasRecvData());
        this->recvStream->Close();
    }
    this->recvStream->SetAccessMode(Stream::ReadAccess);
    return (this->recvStream->GetSize() > 0);
}

//------------------------------------------------------------------------------
/**
*/
const Ptr<Stream>&
StdTcpClient::GetSendStream()
{
    return this->sendStream;
}

//------------------------------------------------------------------------------
/**
*/
const Ptr<Stream>&
StdTcpClient::GetRecvStream()
{
    return this->recvStream;
}

} // Сеть пространства имен