#pragma once
#ifndef POSIX_POSIXIPADDRESS_H
#define POSIX_POSIXIPADDRESS_H
#include "core/types.h"
#include "io/uri.h"
#include <sys/socket.h>
#include <netinet/in.h>
namespace Posix
{
class PosixIpAddress
{
public:
    PosixIpAddress();
    PosixIpAddress(const PosixIpAddress& rhs);
    PosixIpAddress(const IO::URI& uri);
    PosixIpAddress(const Util::String& hostName, ushort portNumber);
    bool operator==(const PosixIpAddress& rhs) const;
    bool operator<(const PosixIpAddress& rhs) const;
    bool operator>(const PosixIpAddress& rhs) const;
    void ExtractFromUri(const IO::URI& uri);
    void SetHostName(const Util::String& hostName);
    const Util::String& GetHostName() const;
    void SetPort(ushort port);
    ushort GetPort() const;
    const Util::String& GetHostAddr() const;
protected:
    friend class PosixSocket;
    void SetSockAddr(const sockaddr& addr);
    const sockaddr& GetSockAddr() const;
    static bool GetHostByName(const Util::String& hostName, sockaddr& outAddr);
    static bool IsInetAddr(const in_addr* addr);
    Util::String hostName;
    Util::String addrAsString;
    sockaddr addr;
};
inline bool
PosixIpAddress::operator==(const PosixIpAddress& rhs) const
{
    n_assert(this->addr.sa_family == AF_INET);
    n_assert(this->addr.sa_family == rhs.addr.sa_family);
    sockaddr_in * myAddr = (sockaddr_in *)(&(this->addr));
    sockaddr_in * otherAddr = (sockaddr_in *)(&(rhs.addr));
    return ((myAddr->sin_addr.s_addr == otherAddr->sin_addr.s_addr) &&
            (myAddr->sin_port == otherAddr->sin_port));
}
inline bool
PosixIpAddress::operator<(const PosixIpAddress& rhs) const
{
    n_assert(this->addr.sa_family == AF_INET);
    n_assert(this->addr.sa_family == rhs.addr.sa_family);
    sockaddr_in * myAddr = (sockaddr_in *)(&(this->addr));
    sockaddr_in * otherAddr = (sockaddr_in *)(&(rhs.addr));
    if (myAddr->sin_addr.s_addr == otherAddr->sin_addr.s_addr)
    {
        return myAddr->sin_port < otherAddr->sin_port;
    }
    else
    {
        return myAddr->sin_addr.s_addr < otherAddr->sin_addr.s_addr;
    }
}
inline bool
PosixIpAddress::operator>(const PosixIpAddress& rhs) const
{
    n_assert(this->addr.sa_family == AF_INET);
    n_assert(this->addr.sa_family == rhs.addr.sa_family);
    sockaddr_in * myAddr = (sockaddr_in *)(&(this->addr));
    sockaddr_in * otherAddr = (sockaddr_in *)(&(rhs.addr));
    if (myAddr->sin_addr.s_addr == otherAddr->sin_addr.s_addr)
    {
        return myAddr->sin_port > otherAddr->sin_port;
    }
    else
    {
        return myAddr->sin_addr.s_addr > otherAddr->sin_addr.s_addr;
    }
}
}
#endif