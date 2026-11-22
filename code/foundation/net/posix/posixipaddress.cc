//------------------------------------------------------------------------------
//  posixipaddress.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "net/posix/posixipaddress.h"
#include "net/posix/posixsocket.h"

#include <netdb.h>
#include <arpa/inet.h>

namespace Posix
{
using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
PosixIpAddress::PosixIpAddress()
{
    if (!PosixSocket::NetworkInitialized)
    {
        PosixSocket::InitNetwork();
    }
    Memory::Clear(&this->addr, sizeof(this->addr));
    this->addr.sa_family = AF_INET;
}

//------------------------------------------------------------------------------
/**
*/
PosixIpAddress::PosixIpAddress(const PosixIpAddress& rhs) :
    hostName(rhs.hostName),
    addrAsString(rhs.addrAsString),
    addr(rhs.addr)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
PosixIpAddress::PosixIpAddress(const String& hostName, ushort portNumber)
{
    Memory::Clear(&this->addr, sizeof(this->addr));
    this->addr.sa_family = AF_INET;
    this->SetHostName(hostName);
    this->SetPort(portNumber);
}

//------------------------------------------------------------------------------
/**
*/
PosixIpAddress::PosixIpAddress(const URI& uri)
{
    Memory::Clear(&this->addr, sizeof(this->addr));
    this->addr.sa_family = AF_INET;
    this->ExtractFromUri(uri);
}

//------------------------------------------------------------------------------
/**
    Set the address directly from a sockaddr_in struct. This will
    set the host name to the string representation of the 
    host address.
*/
void
PosixIpAddress::SetSockAddr(const sockaddr& sa)
{
    this->addr = sa;
    this->hostName = inet_ntoa(((sockaddr_in &)sa).sin_addr);
    this->addrAsString = this->hostName;
}

//------------------------------------------------------------------------------
/**
    Get the sockaddr_in struct, which has either been set directly
    with SetSockAddr() or indirectly through host name, port number
    or from an URI.
*/
const sockaddr&
PosixIpAddress::GetSockAddr() const
{
    return this->addr;
}

//------------------------------------------------------------------------------
/**
    Extract the host name and optionally the port number from the provided
    URI. If no port number is set in the URI, the current port number
    will be left as is. If the host name is empty, it will be set to
    "localhost".
*/
void
PosixIpAddress::ExtractFromUri(const URI& uri)
{
    if (uri.Host().IsValid())
    {
        this->SetHostName(uri.Host());
    }
    else
    {
        this->SetHostName("localhost");
    }
    if (uri.Port().IsValid())
    {
        this->SetPort((ushort)uri.Port().AsInt());
    }
}

//------------------------------------------------------------------------------
/**
    Set the port number. Will be translated to network byte order internally.
*/
void
PosixIpAddress::SetPort(ushort port)
{
    ((sockaddr_in &)(this->addr)).sin_port = htons(port);
}

//------------------------------------------------------------------------------
/**
    Получите номер порта в порядке байтов хоста.
*/
ushort
PosixIpAddress::GetPort() const
{
    return ntohs(((sockaddr_in &)(this->addr)).sin_port);
}

//------------------------------------------------------------------------------
/**
    Set the host name, and immediately convert it to an ip address. This 
    accepts the special hostnames "any", "broadcast", "localhost", "self" 
    and "inetself". The result ip address can be returned in string form
    with the method GetAddrAsString().
*/
void
PosixIpAddress::SetHostName(const String& n)
{
    n_assert(n.IsValid());
    this->hostName = n;
    if (!this->GetHostByName(n, this->addr))
    {
        
    }
    this->addrAsString = inet_ntoa(((sockaddr_in &)this->addr).sin_addr);
}

//------------------------------------------------------------------------------
/**
    Получите имя хоста.
*/
const String&
PosixIpAddress::GetHostName() const
{
    return this->hostName;
}

//------------------------------------------------------------------------------
/**
    Верните адрес в виде строки.
*/
const String&
PosixIpAddress::GetHostAddr() const
{
    return this->addrAsString;
}

//------------------------------------------------------------------------------
/**
    This resolves a host name into a IPv4 ip address. The ip address is
    returned in network byte order in the hostAddress argument. The return value
    indicates whether the operation was successful. The following special hostnames 
    can be defined:

    - "any"         resolves to INADDR_ANY (0.0.0.0)
    - "broadcast"   resolves to INADDR_BROADCAST (255.255.255.255)
    - "localhost"   resolves to 127.0.0.1
    - "self"        resolves to the first address of this host
    - "inetself"    resolves to the first address which is not a LAN address

    An empty host name is invalid. A hostname can also be an address string
    of the form xxx.yyy.zzz.www.
*/
bool
PosixIpAddress::GetHostByName(const Util::String& hostName, sockaddr& outAddr)
{
    n_assert(hostName.IsValid());
    ((sockaddr_in &)outAddr).sin_addr.s_addr = 0;

    if ("any" == hostName)
    {
        ((sockaddr_in &)outAddr).sin_addr.s_addr = htonl(INADDR_ANY);
        return true;
    }
    else if ("broadcast" == hostName)
    {
        ((sockaddr_in &)outAddr).sin_addr.s_addr = htonl(INADDR_BROADCAST);
        return true;
    }
    else if (("self" == hostName) || ("inetself" == hostName))
    {
        // получить имя хоста машины
        char localHostName[512];
        int err = gethostname(localHostName, sizeof(localHostName));
        if (-1 == err)
        {
            return false;
        }

        // разрешить собственное имя хоста
        struct hostent* he = gethostbyname(localHostName);
        if (0 == he)
        {
            // не удалось разрешить собственное имя хоста
            return false;
        }

        // инициализировать с адресом по умолчанию 
        const in_addr* inAddr = (const in_addr *) he->h_addr;
        if (hostName == "inetself")
        {
            // if internet address requested, scan list of ip addresses
            // for a non-Class A,B or C network address
            int i;
            for (i = 0; (0 != he->h_addr_list[i]); i++)
            {
                if (IsInetAddr((const in_addr *)he->h_addr_list[i]))
                {
                    inAddr = (in_addr *)he->h_addr_list[i];
                    break;
                }
            }
        }
        ((sockaddr_in &)outAddr).sin_addr = *inAddr;
        return true;
    }
    else if (hostName.CheckValidCharSet(".0123456789"))
    {
        // цифровой адрес...
        ((sockaddr_in &)outAddr).sin_addr.s_addr = inet_addr(hostName.AsCharPtr());
        return true;
    }
    else
    {
        // случай по умолчанию
        struct hostent* he = gethostbyname(hostName.AsCharPtr());
        if (0 == he)
        {
            // не удалось разрешить имя хоста!
            return false;
        }
        ((sockaddr_in &)outAddr).sin_addr = *((in_addr*)he->h_addr);
        return true;
    }
}

//------------------------------------------------------------------------------
/**
    This method checks if the provided address is an "internet" address,
    not a LAN address (not a class A, B or C network address).
*/
bool
PosixIpAddress::IsInetAddr(const in_addr* addr)
{
    // генерировать адресную строку из addr
    String addrString = inet_ntoa(*addr);

    // разбить строку на ее члены
    Array<String> tokens = addrString.Tokenize(".");
    n_assert(tokens.Size() == 4);
    int b1 = tokens[0].AsInt();
    int b2 = tokens[1].AsInt();
    int b3 = tokens[2].AsInt();
    if ((b1 == 10) && (b2 >= 0) && (b2 <= 254))
    {
        // Сеть класса А
        return false;
    }
    else if ((b1 == 172) && (b2 >= 16) && (b2 <= 31))
    {
        // Сеть класса B
        return false;
    }
    else if ((b1 == 192) && (b2 == 168) && (b3 >= 0) && (b3 <= 254))
    {
        // Сеть класса C
        return false;
    }
    else if (b1 < 224)
    {
        // неизвестный другой тип локальной сети
        return false;
    }
    // Интернет-адрес
    return true;
}

} // пространство имен Posix
