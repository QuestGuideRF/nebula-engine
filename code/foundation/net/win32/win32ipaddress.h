#pragma once
#include "core/types.h"
#include "io/uri.h"
namespace Win32
{
class Win32IpAddress
{
public:
    Win32IpAddress();
    Win32IpAddress(const Win32IpAddress& rhs);
    Win32IpAddress(const IO::URI& uri);
    Win32IpAddress(const Util::String& hostName, ushort portNumber);
    bool operator==(const Win32IpAddress& rhs) const;
    bool operator<(const Win32IpAddress& rhs) const;
    bool operator>(const Win32IpAddress& rhs) const;
    void ExtractFromUri(const IO::URI& uri);
    void SetHostName(const Util::String& hostName);
    const Util::String& GetHostName() const;
    void SetPort(ushort port);
    ushort GetPort() const;
    const Util::String& GetHostAddr() const;
private:
    friend class Win32Socket;
    Win32IpAddress(const sockaddr_in& addr);
    void SetSockAddr(const sockaddr_in& addr);
    const sockaddr_in& GetSockAddr() const;
    static bool GetHostByName(const Util::String& hostName, in_addr& outAddr);
    static bool IsInetAddr(const in_addr* addr);
    Util::String hostName;
    Util::String addrAsString;
    sockaddr_in addr;
};
inline bool
Win32IpAddress::operator==(const Win32IpAddress& rhs) const
{
    return ((this->addr.sin_addr.S_un.S_addr == rhs.addr.sin_addr.S_un.S_addr) &&
            (this->addr.sin_port == rhs.addr.sin_port));
}
inline bool
Win32IpAddress::operator<(const Win32IpAddress& rhs) const
{
    if (this->addr.sin_addr.S_un.S_addr == rhs.addr.sin_addr.S_un.S_addr)
    {
        return this->addr.sin_port < rhs.addr.sin_port;
    }
    else
    {
        return this->addr.sin_addr.S_un.S_addr < rhs.addr.sin_addr.S_un.S_addr;
    }
}
inline bool
Win32IpAddress::operator>(const Win32IpAddress& rhs) const
{
    if (this->addr.sin_addr.S_un.S_addr == rhs.addr.sin_addr.S_un.S_addr)
    {
        return this->addr.sin_port > rhs.addr.sin_port;
    }
    else
    {
        return this->addr.sin_addr.S_un.S_addr > rhs.addr.sin_addr.S_un.S_addr;
    }
}
}