#pragma once
#ifndef POSIX_POSIXSOCKET_H
#define POSIX_POSIXSOCKET_H
#include "core/refcounted.h"
#include "net/socket/ipaddress.h"
#include <sys/socket.h>
#include <netinet/tcp.h>
namespace Net
{
class Socket;
}
namespace Posix
{
typedef int SOCKET;
class PosixSocket : public Core::RefCounted
{
    __DeclareClass(PosixSocket);
public:
    enum Protocol
    {
        TCP,
        UDP,
    };
    enum Result
    {
        Error = 0,
        Success,
        WouldBlock,
        Closed,
    };
    enum ErrorCode
    {
        ErrorNone,
        ErrorUnknown,
        ErrorInterrupted,
    ErrorBrokenPipe,
        ErrorPermissionDenied,
        ErrorBadAddress,
        ErrorInvalidArgument,
        ErrorTooManyOpenFiles,
        ErrorWouldBlock,
        ErrorInProgress,
        ErrorAlreadyInProgress,
        ErrorNotASocket,
        ErrorDestAddrRequired,
        ErrorMsgTooLong,
        ErrorInvalidProtocol,
        ErrorBadProtocolOption,
        ErrorProtocolNotSupported,
        ErrorSocketTypeNotSupported,
        ErrorOperationNotSupported,
        ErrorProtFamilyNotSupported,
        ErrorAddrFamilyNotSupported,
        ErrorAddrInUse,
        ErrorAddrNotAvailable,
        ErrorNetDown,
        ErrorNetUnreachable,
        ErrorNetReset,
        ErrorConnectionAborted,
        ErrorConnectionReset,
        ErrorNoBufferSpace,
        ErrorIsConnected,
        ErrorNotConnected,
        ErrorIsShutdown,
        ErrorIsTimedOut,
        ErrorConnectionRefused,
        ErrorHostDown,
        ErrorHostUnreachable,
        ErrorSystemNotReady,
        ErrorVersionNotSupported,
        ErrorNotInitialized,
        ErrorDisconnecting,
        ErrorTypeNotFound,
        ErrorHostNotFound,
        ErrorTryAgain,
        ErrorNoRecovery,
        ErrorNoData,
    };
    PosixSocket();
    virtual ~PosixSocket();
    bool Open(Protocol p);
    void Close();
    bool IsOpen() const;
    ErrorCode GetErrorCode() const;
    Util::String GetErrorString() const;
    void SetAddress(const Net::IpAddress& a);
    const Net::IpAddress& GetAddress() const;
    void SetBroadcast(bool b);
    bool GetBroadcast();
    void SetDontLinger(bool b);
    bool GetDontLinger();
    void SetKeepAlive(bool b);
    bool GetKeepAlive();
    void SetReUseAddr(bool b);
    bool GetReUseAddr();
    void SetNoDelay(bool b);
    bool GetNoDelay();
    void SetRecvBufSize(SizeT s);
    SizeT GetRecvBufSize();
    void SetSendBufSize(SizeT s);
    SizeT GetSendBufSize();
    void SetBlocking(bool b);
    bool GetBlocking() const;
    SizeT GetMaxMsgSize();
    bool Bind();
    bool IsBound() const;
    bool Listen();
    bool Accept(Ptr<Net::Socket>& outSocket);
    Result Connect();
    bool IsConnected();
    Result Send(const void* buf, SizeT numBytes, SizeT& bytesSent);
    bool HasRecvData();
    Result Recv(void* buf, SizeT bufSize, SizeT& bytesReceived);
    Result SendTo(const void* buf, SizeT numBytes, uint addr, ushort port, SizeT& bytesSent);
    Result RecvFrom(void* buf, SizeT bufSize, uint addr, ushort port, SizeT& bytesReceived);
private:
    friend class PosixIpAddress;
    friend class SysFunc;
    static void InitNetwork();
    void OpenWithExistingSocket(int s);
    void ClearError();
    void SetToLastSocketError();
    void SetSocketError(int wsaError);
    static ErrorCode SocketErrorToErrorCode(int wsaError);
    static ErrorCode HostErrorToErrorCode(int wsaError);
    static Util::String ErrorAsString(ErrorCode err);
    static Util::String SocketErrorToString(int wsaError);
    static Util::String HostErrorToString(int wsaError);
    void SetBoolOption(int optName, bool val);
    bool GetBoolOption(int optName);
    void SetIntOption(int optName, int val);
    int GetIntOption(int optName);
    static bool NetworkInitialized;
    ErrorCode error;
    SOCKET sock;
    Net::IpAddress addr;
    bool isBlocking;
    bool isBound;
};
inline bool
PosixSocket::IsOpen() const
{
    return (0 != this->sock);
}
inline bool
PosixSocket::IsBound() const
{
    return this->isBound;
}
inline void
PosixSocket::SetAddress(const Net::IpAddress& a)
{
    this->addr = a;
}
inline const Net::IpAddress&
PosixSocket::GetAddress() const
{
    return this->addr;
}
inline PosixSocket::ErrorCode
PosixSocket::GetErrorCode() const
{
  return this->error;
}
inline void
PosixSocket::SetBroadcast(bool b)
{
    this->SetBoolOption(SO_BROADCAST, b);
}
inline bool
PosixSocket::GetBroadcast()
{
    return this->GetBoolOption(SO_BROADCAST);
}
inline void
PosixSocket::SetDontLinger(bool b)
{
    this->SetBoolOption(SO_LINGER, !b);
}
inline bool
PosixSocket::GetDontLinger()
{
    return !this->GetBoolOption(SO_LINGER);
}
inline void
PosixSocket::SetKeepAlive(bool b)
{
    this->SetBoolOption(SO_KEEPALIVE, b);
}
inline bool
PosixSocket::GetKeepAlive()
{
    return this->GetBoolOption(SO_KEEPALIVE);
}
inline void
PosixSocket::SetReUseAddr(bool b)
{
    this->SetBoolOption(SO_REUSEADDR, b);
}
inline bool
PosixSocket::GetReUseAddr()
{
    return this->GetBoolOption(SO_REUSEADDR);
}
inline void
PosixSocket::SetNoDelay(bool b)
{
    this->SetBoolOption(TCP_NODELAY, b);
}
inline bool
PosixSocket::GetNoDelay()
{
    return this->GetBoolOption(TCP_NODELAY);
}
inline void
PosixSocket::SetRecvBufSize(SizeT s)
{
    this->SetIntOption(SO_RCVBUF, s);
}
inline SizeT
PosixSocket::GetRecvBufSize()
{
    return this->GetIntOption(SO_RCVBUF);
}
inline void
PosixSocket::SetSendBufSize(SizeT s)
{
    this->SetIntOption(SO_SNDBUF, s);
}
inline SizeT
PosixSocket::GetSendBufSize()
{
    return this->GetIntOption(SO_SNDBUF);
}
inline SizeT
PosixSocket::GetMaxMsgSize()
{
    return 1500;
}
inline bool
PosixSocket::GetBlocking() const
{
    return this->isBlocking;
}
}
#endif