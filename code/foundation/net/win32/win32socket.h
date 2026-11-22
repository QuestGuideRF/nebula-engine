#pragma once
#include "core/refcounted.h"
#include "net/socket/ipaddress.h"
namespace Net
{
class Socket;
}
namespace Win32
{
class Win32Socket : public Core::RefCounted
{
    __DeclareClass(Win32Socket);
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
        ErrorTooManyProcesses,
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
    Win32Socket();
    virtual ~Win32Socket();
    static void InitNetwork();
    static bool IsNetworkInitialized();
    bool Open(Protocol p);
    void Close();
    bool IsOpen() const;
    ErrorCode GetErrorCode() const;
    Util::String GetErrorString() const;
    void SetAddress(const Net::IpAddress& a);
    const Net::IpAddress& GetAddress() const;
    void SetBroadcast(bool b);
    bool GetBroadcast();
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
    friend class Win32IpAddress;
    void OpenWithExistingSocket(SOCKET s);
    void ClearError();
    void SetToLastWSAError();
    void SetWSAError(int wsaError);
    static ErrorCode WSAErrorToErrorCode(int wsaError);
    static Util::String ErrorAsString(ErrorCode err);
    static Util::String WSAErrorToString(int wsaError);
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
Win32Socket::IsOpen() const
{
    return (0 != this->sock);
}
inline bool
Win32Socket::IsBound() const
{
    return this->isBound;
}
inline void
Win32Socket::SetAddress(const Net::IpAddress& a)
{
    this->addr = a;
}
inline const Net::IpAddress&
Win32Socket::GetAddress() const
{
    return this->addr;
}
inline Win32Socket::ErrorCode
Win32Socket::GetErrorCode() const
{
    return this->error;
}
inline void
Win32Socket::SetBroadcast(bool b)
{
    this->SetBoolOption(SO_BROADCAST, b);
}
inline bool
Win32Socket::GetBroadcast()
{
    return this->GetBoolOption(SO_BROADCAST);
}
inline void
Win32Socket::SetKeepAlive(bool b)
{
    this->SetBoolOption(SO_KEEPALIVE, b);
}
inline bool
Win32Socket::GetKeepAlive()
{
    return this->GetBoolOption(SO_KEEPALIVE);
}
inline void
Win32Socket::SetReUseAddr(bool b)
{
    this->SetBoolOption(SO_REUSEADDR, b);
}
inline bool
Win32Socket::GetReUseAddr()
{
    return this->GetBoolOption(SO_REUSEADDR);
}
inline void
Win32Socket::SetNoDelay(bool b)
{
    this->SetBoolOption(TCP_NODELAY, b);
}
inline bool
Win32Socket::GetNoDelay()
{
    return this->GetBoolOption(TCP_NODELAY);
}
inline void
Win32Socket::SetRecvBufSize(SizeT s)
{
    this->SetIntOption(SO_RCVBUF, s);
}
inline SizeT
Win32Socket::GetRecvBufSize()
{
    return this->GetIntOption(SO_RCVBUF);
}
inline void
Win32Socket::SetSendBufSize(SizeT s)
{
    this->SetIntOption(SO_SNDBUF, s);
}
inline SizeT
Win32Socket::GetSendBufSize()
{
    return this->GetIntOption(SO_SNDBUF);
}
inline SizeT
Win32Socket::GetMaxMsgSize()
{
    return 8192;
}
inline bool
Win32Socket::GetBlocking() const
{
    return this->isBlocking;
}
}