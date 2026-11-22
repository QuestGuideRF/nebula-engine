//------------------------------------------------------------------------------
//  stdtcperver.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "net/tcp/stdtcpserver.h"

namespace Net
{
__ImplementClass(Net::StdTcpServer, 'STSV', Core::RefCounted);
__ImplementClass(Net::StdTcpServer::ListenerThread, 'tclt', Threading::Thread);

using namespace Util;
using namespace Threading;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
StdTcpServer::StdTcpServer() :
    isOpen(false)
{
    this->connectionClassRtti = &TcpClientConnection::RTTI;
}

//------------------------------------------------------------------------------
/**
*/
StdTcpServer::~StdTcpServer()
{
    n_assert(!this->IsOpen());
}

//------------------------------------------------------------------------------
/**
*/
bool
StdTcpServer::Open()
{
    n_assert(!this->isOpen);
    n_assert(!this->listenerThread.isvalid());
    n_assert(this->clientConnections.IsEmpty());

    // создать поток слушателя
    this->listenerThread = ListenerThread::Create();
    this->listenerThread->SetName("StdTcpServer::ListenerThread");
    this->listenerThread->SetTcpServer(this);
    this->listenerThread->SetAddress(this->ipAddress);
    this->listenerThread->SetThreadAffinity(System::Cpu::Core4);
    this->listenerThread->SetClientConnectionClass(*this->connectionClassRtti);
    this->listenerThread->Start();
    
    this->isOpen = true;
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
StdTcpServer::Close()
{
    n_assert(this->isOpen);
    n_assert(this->listenerThread.isvalid());
    
    // остановить поток слушателя
    this->listenerThread->Stop();
    this->listenerThread = nullptr;

    // отключить клиентские соединения
    this->connectionCritSect.Enter();
    IndexT clientIndex;
    for (clientIndex = 0; clientIndex < this->clientConnections.Size(); clientIndex++)
    {
        const Ptr<TcpClientConnection>& curConnection = this->clientConnections[clientIndex];
        curConnection->Shutdown();
    }
    this->clientConnections.Clear();
    this->connectionCritSect.Leave();

    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
*/
void
StdTcpServer::AddClientConnection(const Ptr<TcpClientConnection>& conn)
{
    n_assert(conn.isvalid());
    this->connectionCritSect.Enter();
    this->clientConnections.Append(conn);
    this->connectionCritSect.Leave();
}

//------------------------------------------------------------------------------
/**
*/
Array<Ptr<TcpClientConnection> >
StdTcpServer::Recv()
{
    Array<Ptr<TcpClientConnection> > clientsWithData;

    // перебрать всех клиентов и проверить наличие новых данных,
    // if the client connection has been closed, remove
    // клиент из списка
    this->connectionCritSect.Enter();
    IndexT clientIndex;
    for (clientIndex = 0; clientIndex < this->clientConnections.Size();)
    {
        const Ptr<TcpClientConnection>& cur = this->clientConnections[clientIndex];
        bool dropClient = false;
        if (cur->IsConnected())
        {
            Socket::Result res = cur->Recv();
            if (res == Socket::Success)
            {
                clientsWithData.Append(cur);
            }
            else if ((res == Socket::Error) || (res == Socket::Closed))
            {
                // произошла какая-то ошибка, разорвите соединение
                dropClient = true;
            }
        }
        else
        {
            dropClient = true;
        }
        if (dropClient)
        {
            // соединение закрыто, удалите клиент
            cur->Shutdown();
            this->clientConnections.EraseIndex(clientIndex);
        }
        else
        {
            clientIndex++;
        }
    }
    this->connectionCritSect.Leave();
    return clientsWithData;
}

//------------------------------------------------------------------------------
/**
*/
bool
StdTcpServer::Broadcast(const Ptr<Stream>& msg)
{
    bool result = true;
    this->connectionCritSect.Enter();
    IndexT i;
    for (i = 0; i < this->clientConnections.Size(); i++)
    {
        if (Socket::Success != this->clientConnections[i]->Send(msg))
        {
            result = false;
        }
    }
    this->connectionCritSect.Leave();
    return result;
}

//------------------------------------------------------------------------------
/**
*/
void
StdTcpServer::ListenerThread::SetTcpServer(StdTcpServer* serv)
{
    n_assert(0 != serv);
    this->tcpServer = serv;
}

//------------------------------------------------------------------------------
/**
*/
void
StdTcpServer::ListenerThread::SetAddress(const IpAddress& a)
{
    this->ipAddress = a;
}

//------------------------------------------------------------------------------
/**
*/
void
StdTcpServer::ListenerThread::SetClientConnectionClass(const Core::Rtti& type)
{
    this->connectionClassRtti = &type;
}

//------------------------------------------------------------------------------
/**
    Это фактический метод прослушивания, который выполняется в отдельном потоке.
    Он создает сокет сервера и прослушивает входящие соединения. Когда
    клиент подключается, будет создан новый TcpClientConnection и
    добавлен в массив активных соединений.
*/
void
StdTcpServer::ListenerThread::DoWork()
{
    // создать сокет сервера
    Ptr<Socket> serverSocket = Socket::Create();
    if (serverSocket->Open(Socket::TCP))
    {
        serverSocket->SetAddress(this->ipAddress);
        serverSocket->SetReUseAddr(true);
        if (serverSocket->Bind())
        {
            while (!this->ThreadStopRequested() && serverSocket->Listen())
            {
                // проверьте, не эта ли функция пробуждения нас разбудила...
                if (!this->ThreadStopRequested())
                {
                    Ptr<Socket> newSocket;
                    if (serverSocket->Accept(newSocket))
                    {
                        // создайте новый объект соединения и добавьте в массив соединений
                        TcpClientConnection* newConnection = (TcpClientConnection*) this->connectionClassRtti->Create();
                        if (newConnection->Connect(newSocket))
                        {
                            this->tcpServer->AddClientConnection(newConnection);
                        }
                    }
                }
            }
        }
        else
        {
            n_warn2(false, "StdTcpServer::ListenerThread: Ошибка Socket::Bind()!");
        }
        serverSocket->Close();
    }
    
    // ПРИМЕЧАНИЕ. мы не закрываем консоль локали потока вручную, потому что
    // другим объектам может быть что сказать, пока они уничтожены
}

//------------------------------------------------------------------------------
/**
    Отправьте сигнал пробуждения в поток прослушивателя, это просто подключится
    в сокет, который активирует метод Socket::Listen().
*/
void
StdTcpServer::ListenerThread::EmitWakeupSignal()
{
    Ptr<Socket> socket = Socket::Create();
    if (socket->Open(Socket::TCP))
    {
        socket->SetAddress(IpAddress("127.0.0.1", this->ipAddress.GetPort()));
        socket->Connect();
        socket->Close();
    }
}

} // Сеть пространства имен
