#pragma once
#include "core/refcounted.h"
#include "threading/thread.h"
#include "util/array.h"
#include "net/tcpclientconnection.h"
#include "net/socket/socket.h"
#include "threading/criticalsection.h"
namespace Net
{
class StdTcpServer : public Core::RefCounted
{
    __DeclareClass(StdTcpServer);
public:
    StdTcpServer();
    virtual ~StdTcpServer();
    void SetAddress(const IpAddress& addr);
    const IpAddress& GetAddress() const;
    void SetClientConnectionClass(const Core::Rtti& type);
    const Core::Rtti& GetClientConnectionClass();
    bool Open();
    void Close();
    bool IsOpen() const;
    Util::Array<Ptr<TcpClientConnection> > Recv();
    bool Broadcast(const Ptr<IO::Stream>& msg);
private:
    class ListenerThread : public Threading::Thread
    {
        __DeclareClass(ListenerThread);
    public:
        void SetTcpServer(StdTcpServer* tcpServer);
        void SetAddress(const IpAddress& a);
        void SetClientConnectionClass(const Core::Rtti& type);
    private:
        virtual void DoWork();
        virtual void EmitWakeupSignal();
        Ptr<StdTcpServer> tcpServer;
        IpAddress ipAddress;
        Ptr<Socket> socket;
        const Core::Rtti* connectionClassRtti;
    };
    friend class ListenerThread;
    void AddClientConnection(const Ptr<TcpClientConnection>& connection);
    IpAddress ipAddress;
    Ptr<ListenerThread> listenerThread;
    bool isOpen;
    Util::Array<Ptr<TcpClientConnection> > clientConnections;
    Threading::CriticalSection connectionCritSect;
    const Core::Rtti* connectionClassRtti;
};
inline bool
StdTcpServer::IsOpen() const
{
    return this->isOpen;
}
inline void
StdTcpServer::SetAddress(const IpAddress& addr)
{
    this->ipAddress = addr;
}
inline const IpAddress&
StdTcpServer::GetAddress() const
{
    return this->ipAddress;
}
inline void
StdTcpServer::SetClientConnectionClass(const Core::Rtti& type)
{
    this->connectionClassRtti = &type;
}
inline const Core::Rtti&
StdTcpServer::GetClientConnectionClass()
{
    return *this->connectionClassRtti;
}
}