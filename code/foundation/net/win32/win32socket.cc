//------------------------------------------------------------------------------
//  win32socket.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "net/socket/socket.h"

namespace Win32
{
__ImplementClass(Win32::Win32Socket, 'XSCK', Core::RefCounted);
using namespace Util;

bool Win32Socket::NetworkInitialized = false;

//------------------------------------------------------------------------------
/**
*/
Win32Socket::Win32Socket() :
    error(ErrorNone),
    sock(0),
    isBlocking(true),
    isBound(false)
{
    n_assert(NetworkInitialized);
}

//------------------------------------------------------------------------------
/**
*/
Win32Socket::~Win32Socket()
{
    if (this->IsOpen())
    {
        this->Close();
    }
}

//------------------------------------------------------------------------------
/**
    This is a one-time init for the Windows Sockets system. The method
    is called from SysFunc::Setup() once at startup before any threads
    are launched.
*/
void
Win32Socket::InitNetwork()
{
    n_assert(!NetworkInitialized);

    // теперь действуй как обычно
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (0 != result)
    {
        n_error("WSAStartup() failed with '%s'!", WSAErrorToString(result).AsCharPtr());
    }
    NetworkInitialized = true;
}   

//------------------------------------------------------------------------------
/**
*/
bool
Win32Socket::Open(Protocol protocol)
{
    n_assert(!this->IsOpen());

    this->ClearError();

    int sockType;
    int protType;
    switch (protocol)
    {
        case TCP:    
            sockType = SOCK_STREAM; 
            protType = IPPROTO_TCP;
            break;
        case UDP:  
            sockType = SOCK_DGRAM; 
            protType = IPPROTO_UDP;
            break;
        default:
            // не может случиться.
            n_error("Неверный тип сокета!");
            sockType = SOCK_STREAM; 
            protType = IPPROTO_TCP;
            break;
    }
    this->sock = socket(AF_INET, sockType, protType);
    if (INVALID_SOCKET == this->sock)
    {
        this->sock = 0;
        this->SetToLastWSAError();
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
    Open the socket object with an existing initialized system socket.
    This is a private method and only called by Accept() on the
    new socket created by the accept() function.
*/
void
Win32Socket::OpenWithExistingSocket(SOCKET s)
{
    n_assert(INVALID_SOCKET != s);
    n_assert(!this->IsOpen());
    this->sock = s;
}

//------------------------------------------------------------------------------
/**
*/
void
Win32Socket::Close()
{
    n_assert(this->IsOpen());
    this->ClearError();

    int res = 0;
    if (this->IsConnected())
    {
        res = shutdown(this->sock, SD_BOTH);
        if (SOCKET_ERROR == res)
        {
            // примечание: функция выключения может возвращать NotConnected, это
            // это не совсем ошибка
            this->SetToLastWSAError();
            if (ErrorNotConnected != this->error)
            {
                n_printf("Win32Socket::Close(): shutdown() failed with '%s'.\n", this->GetErrorString().AsCharPtr());
            }
        }
    }
    res = closesocket(this->sock);
    if (SOCKET_ERROR == res)
    {
        this->SetToLastWSAError();
        n_printf("Win32Socket::Close(): closesocket() failed with '%s'.\n", this->GetErrorString().AsCharPtr());
    }
    this->sock = 0;
}

//------------------------------------------------------------------------------
/**
    Установите логический параметр для сокета. Это частный помощник
    функция.
*/
void
Win32Socket::SetBoolOption(int optName, bool val)
{
    n_assert(this->IsOpen());
    this->ClearError();
    int level = SOL_SOCKET;
    if (optName == TCP_NODELAY)
    {
        level = IPPROTO_TCP;
    }
    int optVal = val ? 1 : 0;
    int res = setsockopt(this->sock, level, optName, (const char*) &optVal, sizeof(optVal));
    if (SOCKET_ERROR == res)
    {
        this->SetToLastWSAError();
        n_printf("Win32Socket::SetBoolOption(): setsockopt() failed with '%s'.\n", this->GetErrorString().AsCharPtr());
    }
}

//------------------------------------------------------------------------------
/**
    Получите логический параметр в сокете. Это частный помощник
    функция.
*/
bool
Win32Socket::GetBoolOption(int optName)
{
    n_assert(this->IsOpen());
    this->ClearError();
    int level = SOL_SOCKET;
    if (optName == TCP_NODELAY)
    {
        level = IPPROTO_TCP;
    }
    int optVal = 0;
    int optValSize = sizeof(optVal);
    int res = getsockopt(this->sock, level, optName, (char*) &optVal, &optValSize);
    if (SOCKET_ERROR == res)
    {
        this->SetToLastWSAError();
        n_printf("Win32Socket::GetBoolOption(): getsockopt() failed with '%s'.\n", this->GetErrorString().AsCharPtr());
    }
    n_assert(sizeof(optVal) == optValSize);
    return (0 != optVal);
}

//------------------------------------------------------------------------------
/**
    Установите параметр сокета int. Это частная вспомогательная функция.
*/
void
Win32Socket::SetIntOption(int optName, int val)
{
    n_assert(this->IsOpen());
    this->ClearError();
    int res = setsockopt(this->sock, SOL_SOCKET, optName, (const char*) &val, sizeof(val));
    if (SOCKET_ERROR == res)
    {
        this->SetToLastWSAError();
        n_printf("Win32Socket::SetIntOption(): setsockopt() failed with '%s'.\n", this->GetErrorString().AsCharPtr());
    }
}

//------------------------------------------------------------------------------
/**
    Получите опцию int сокета. Это частная вспомогательная функция.
*/
int
Win32Socket::GetIntOption(int optName)
{
    n_assert(this->IsOpen());
    this->ClearError();
    int optVal = 0;
    int optValSize = sizeof(optVal);
    int res = getsockopt(this->sock, SOL_SOCKET, optName, (char*) &optVal, &optValSize);
    if (SOCKET_ERROR == res)
    {
        this->SetToLastWSAError();
        n_printf("Win32Socket::GetIntOption(): getsockopt() failed with '%s'.\n", this->GetErrorString().AsCharPtr());
    }
    n_assert(sizeof(optVal) == optValSize);
    return optVal;
}

//------------------------------------------------------------------------------
/**
    Установите сокет в режим блокировки.
*/
void
Win32Socket::SetBlocking(bool b)
{
    n_assert(this->IsOpen());
    this->ClearError();
    u_long arg = b ? 0 : 1;
    int res = ioctlsocket(this->sock, FIONBIO, &arg);
    if (SOCKET_ERROR == res)
    {
        this->SetToLastWSAError();
        n_printf("Win32Socket::SetBlocking(): ioctlsocket() failed with '%s'.\n", this->GetErrorString().AsCharPtr());
    }
    this->isBlocking = b;
}

//------------------------------------------------------------------------------
/**
    Bind the socket to its ip address set with SetAddress() and
    SetPort(). After binding the socket to an address, call
    the Listen() method to wait for incoming connections. This method
    only makes sense for server sockets.
*/
bool
Win32Socket::Bind()
{
    n_assert(this->IsOpen());
    n_assert(!this->IsBound());
    this->ClearError();
    const sockaddr_in& sockAddr = this->addr.GetSockAddr();
    int res = bind(this->sock, (const sockaddr*) &sockAddr, sizeof(sockAddr));
    if (SOCKET_ERROR == res)
    {
        this->SetToLastWSAError();
        n_printf("Win32Socket::Bind(): bind() failed with '%s'!\n", this->GetErrorString().AsCharPtr());
        return false;
    }
    this->isBound = true;
    return true;
}

//------------------------------------------------------------------------------
/**
    Дождитесь входящих подключений к сокету сервера. Позвони сюда 
    метод на стороне сервера после привязки сокета к его адресу.
*/
bool
Win32Socket::Listen()
{
    n_assert(this->IsOpen());
    n_assert(this->IsBound());
    this->ClearError();
    int res = listen(this->sock, SOMAXCONN);
    if (SOCKET_ERROR == res)
    {
        this->SetToLastWSAError();
        n_printf("Win32Socket::Listen(): listen() failed with '%s'!\n", this->GetErrorString().AsCharPtr());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
    Принять входящее соединение с сокетом сервера. Это будет порождать
    новый сокет для соединения, который будет возвращен в предоставленном
    ссылка на указатель. Адрес возвращаемого сокета будет установлен на
    адрес «связывающего объекта».
*/
bool
Win32Socket::Accept(Ptr<Net::Socket>& outSocket)
{
    n_assert(this->IsOpen());
    n_assert(this->IsBound());

    // проверьте, заблокируется ли сокет
    fd_set readFd;
    FD_ZERO(&readFd);
    FD_SET(this->sock,&readFd);
    timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if(select(0,&readFd,0,0,&tv) != 1)
    {
        return false;
    }

    this->ClearError();
    outSocket = nullptr;
    sockaddr_in sockAddr;
    int sockAddrSize = sizeof(sockAddr);
    SOCKET newSocket = accept(this->sock, (sockaddr*) &sockAddr, &sockAddrSize);
    if (INVALID_SOCKET == newSocket)
    {
        this->SetToLastWSAError();
        n_printf("Win32Socket::Accept(): accept() failed with '%s'!\n", this->GetErrorString().AsCharPtr());
        return false;
    }
    outSocket = Net::Socket::Create();
    outSocket->SetAddress(Win32IpAddress(sockAddr));
    outSocket->OpenWithExistingSocket(newSocket);
    outSocket->SetBlocking(this->isBlocking);
    
    return true;
}

//------------------------------------------------------------------------------
/**
    Connect to a server socket. This method is called by a client socket
    to connect to a server socket identified by the socket object's address.
    A non-blocking socket will return immediately with WouldBlock, since the
    connection cannot be established immediately. In this case, just continue
    to call Connect() until the method returns Success, or alternative, check
    the IsConnected() method, which will also return true once the connection
    has been establish.
*/
Win32Socket::Result
Win32Socket::Connect()
{
    n_assert(this->IsOpen());
    n_assert(!this->IsBound());

    this->ClearError();
    const sockaddr_in& sockAddr = this->addr.GetSockAddr();
    int res = connect(this->sock, (const sockaddr*) &sockAddr, sizeof(sockAddr));
    if (SOCKET_ERROR == res)
    {
        // специальная обработка неблокирующих сокетов
        int wsaError = WSAGetLastError();
        if (!this->GetBlocking())
        {
            if (WSAEWOULDBLOCK == wsaError)
            {
                return WouldBlock;
            }
            else if (WSAEALREADY == wsaError)
            {
                // соединение выполняется, но еще не завершено
                return WouldBlock;
            }
            else if (WSAEISCONN == wsaError)
            {
                // соединение установлено
                return Success;
            }
            // провал: обычная ошибка
        }
        this->SetWSAError(wsaError);
        n_printf("Win32Socket::Connect(): connect() failed with '%s'!\n", this->GetErrorString().AsCharPtr());
        return Error;
    }
    return Success;
}

//------------------------------------------------------------------------------
/**
    This tests if the socket is actually connected by doing a select()
    on the socket to probe for writability. So the IsConnected() method
    basically checks whether data can be sent through the socket.
*/
bool
Win32Socket::IsConnected()
{
    n_assert(this->IsOpen());
    fd_set writeSet = { 1, { this->sock } };
    TIMEVAL timeVal = { 0, 0 };
    int res = select(0, 0, &writeSet, 0, &timeVal);
    if (SOCKET_ERROR == res)
    {
        this->SetToLastWSAError();
        n_printf("Win32Socket::IsConnected(): select() failed with '%s'!\n", this->GetErrorString().AsCharPtr());
        return false;
    }
    else if (0 == res)
    {
        return false;
    }
    else
    {
        return true;
    }
}

//------------------------------------------------------------------------------
/**
    Send raw data into the socket. Note that depending on the buffer size
    of the underlying socket implementation and other sockets, the method
    may not be able to send all provided data. In this case, the returned 
    content of bytesSent will be less then numBytes, even though the
    return value will be Success. It is up to the caller to handle the
    extra data which hasn't been sent with the current call.
*/
Win32Socket::Result
Win32Socket::Send(const void* buf, SizeT numBytes, SizeT& bytesSent)
{
    n_assert(this->IsOpen());
    n_assert(0 != buf);
    this->ClearError();
    bytesSent = 0;
    int res = send(this->sock, (const char*) buf, numBytes, 0);
    if (SOCKET_ERROR == res)
    {
        if (WSAEWOULDBLOCK == res)
        {
            return WouldBlock;
        }
        else
        {
            this->SetToLastWSAError();
            n_printf("Win32Socket::Send(): send() failed with '%s'\n", this->GetErrorString().AsCharPtr());
            return Error;
        }
    }
    else
    {
        bytesSent = res;
        return Success;
    }
}

//------------------------------------------------------------------------------
/**
    This method checks if the socket has received data available. Use
    this method in a loop with Recv() to get all data waiting at the
    socket. This method will never block.
*/
bool
Win32Socket::HasRecvData()
{
    n_assert(this->IsOpen());
    fd_set readSet = { 1, { this->sock } };
    TIMEVAL timeVal = { 0, 0 };
    int res = select(0, &readSet, 0, 0, &timeVal);
    if (SOCKET_ERROR == res)
    {
        this->SetToLastWSAError();
        n_printf("Win32Socket::HasRecvData(): select() failed with '%s'!\n", this->GetErrorString().AsCharPtr());
        return false;
    }
    else if (0 == res)
    {
        return false;
    }
    else
    {
        return true;
    }
}

//------------------------------------------------------------------------------
/**
    Receive raw data from a socket and write the received data into the
    provided buffer. On a blocking socket this method will block until
    data arrives at the socket. A non-blocking socket would immediately return in
    this case with a WouldBlock result. When valid data has been received
    the method will return with a Success result and the bytesReceived argument
    will contain the number of received bytes. It is not guaranteed that a single
    receive will return all data waiting on the socket. To make sure that the
    socket is really empty, call Recv() in a loop until HasRecvData()
    returns false.
    When the socket has been gracefully closed by the other side, the method will 
    return with a Closed return value. Everything else will return with an Error 
    return code. Call GetErrorCode() or GetErrorString() to find out more in this case.
*/
Win32Socket::Result
Win32Socket::Recv(void* buf, SizeT bufSize, SizeT& bytesReceived)
{
    n_assert(this->IsOpen());
    n_assert(0 != buf);
    this->ClearError();
    bytesReceived = 0;
    int res = recv(this->sock, (char*) buf, bufSize, 0);
    if (0 == res)
    {
        // соединение было корректно закрыто
        return Closed;
    }
    else if (SOCKET_ERROR == res)
    {
        // поймать особые условия ошибки
        int wsaError = WSAGetLastError();
        if (WSAEMSGSIZE == wsaError)
        {
            // дополнительные данные ожидаются
            bytesReceived = bufSize;    // FIXME: это правильно?
            return Success;
        }
        if (!this->isBlocking)
        {
            // сокет неблокируется и данные недоступны
            if (WSAEWOULDBLOCK == wsaError)
            {
                return WouldBlock;
            }
        }

        // провал: настоящая ошибка
        this->SetToLastWSAError();
        n_printf("Win32Socket::Recv(): recv() failed with '%s'\n", this->GetErrorString().AsCharPtr());
        return Error;
    }
    else
    {
        bytesReceived = res;
        return Success;
    }
}

//------------------------------------------------------------------------------
/**
    FIXME: это метод отправки для сокетов без установления соединения с использованием 
    UDP-протокол.
*/
Win32Socket::Result
Win32Socket::SendTo(const void* /*buf*/, SizeT /*numBytes*/, uint /*addr*/, ushort /*port*/, SizeT& /*bytesSent*/)
{
    n_error("Win32Socket::SendTo(): ​​РЕАЛИЗУЙТЕ МЕНЯ!");
    return Error;
}

//------------------------------------------------------------------------------
/**
    FIXME: это метод получения сокета без установления соединения с использованием UDP.
    протокол.
*/
Win32Socket::Result
Win32Socket::RecvFrom(void* /*buf*/, SizeT /*bufSize*/, uint /*addr*/, ushort /*port*/, SizeT& /*bytesReceived*/)
{
    n_error("Win32Socket::RecvFrom(): РЕАЛИЗУЙТЕ МЕНЯ!");
    return Error;
}

//------------------------------------------------------------------------------
/**
    Устанавливает код внутренней ошибки NoError.
*/
void
Win32Socket::ClearError()
{
    this->error = ErrorNone;
}

//------------------------------------------------------------------------------
/**
    Устанавливает внутренний код ошибки WSAGetLastError().
*/
void
Win32Socket::SetToLastWSAError()
{
    this->error = WSAErrorToErrorCode(WSAGetLastError());
}

//------------------------------------------------------------------------------
/**
    Устанавливает предоставленную ошибку WSA в качестве кода ошибки.
*/
void
Win32Socket::SetWSAError(int wsaError)
{
    this->error = WSAErrorToErrorCode(wsaError);
}

//------------------------------------------------------------------------------
/**
    This method converts an Windows Socket error code into a
    portable error code used by Win32Socket.
*/
Win32Socket::ErrorCode
Win32Socket::WSAErrorToErrorCode(int wsaErrorCode)
{
    switch (wsaErrorCode)
    {
        case WSAEINTR:              return ErrorInterrupted; break;            
        case WSAEACCES:             return ErrorPermissionDenied; break;       
        case WSAEFAULT:             return ErrorBadAddress; break;             
        case WSAEINVAL:             return ErrorInvalidArgument; break;        
        case WSAEMFILE:             return ErrorTooManyOpenFiles; break;       
        case WSAEWOULDBLOCK:        return ErrorWouldBlock; break;             
        case WSAEINPROGRESS:        return ErrorInProgress; break;             
        case WSAEALREADY:           return ErrorAlreadyInProgress; break;      
        case WSAENOTSOCK:           return ErrorNotASocket; break;             
        case WSAEDESTADDRREQ:       return ErrorDestAddrRequired; break;       
        case WSAEMSGSIZE:           return ErrorMsgTooLong; break;             
        case WSAEPROTOTYPE:         return ErrorInvalidProtocol; break;        
        case WSAENOPROTOOPT:        return ErrorBadProtocolOption; break;      
        case WSAEPROTONOSUPPORT:    return ErrorProtocolNotSupported; break;   
        case WSAESOCKTNOSUPPORT:    return ErrorSocketTypeNotSupported; break; 
        case WSAEOPNOTSUPP:         return ErrorOperationNotSupported; break;  
        case WSAEPFNOSUPPORT:       return ErrorProtFamilyNotSupported; break; 
        case WSAEAFNOSUPPORT:       return ErrorAddrFamilyNotSupported; break; 
        case WSAEADDRINUSE:         return ErrorAddrInUse; break;              
        case WSAEADDRNOTAVAIL:      return ErrorAddrNotAvailable; break;       
        case WSAENETDOWN:           return ErrorNetDown; break;                
        case WSAENETUNREACH:        return ErrorNetUnreachable; break;         
        case WSAENETRESET:          return ErrorNetReset; break;               
        case WSAECONNABORTED:       return ErrorConnectionAborted; break;      
        case WSAECONNRESET:         return ErrorConnectionReset; break;        
        case WSAENOBUFS:            return ErrorNoBufferSpace; break;          
        case WSAEISCONN:            return ErrorIsConnected; break;            
        case WSAENOTCONN:           return ErrorNotConnected; break;           
        case WSAESHUTDOWN:          return ErrorIsShutdown; break;             
        case WSAETIMEDOUT:          return ErrorIsTimedOut; break;             
        case WSAECONNREFUSED:       return ErrorConnectionRefused; break;      
        case WSAEHOSTDOWN:          return ErrorHostDown; break;               
        case WSAEHOSTUNREACH:       return ErrorHostUnreachable; break;        
        case WSAEPROCLIM:           return ErrorTooManyProcesses; break;       
        case WSASYSNOTREADY:        return ErrorSystemNotReady; break;         
        case WSAVERNOTSUPPORTED:    return ErrorVersionNotSupported; break;    
        case WSANOTINITIALISED:     return ErrorNotInitialized; break;         
        case WSAEDISCON:            return ErrorDisconnecting; break;          
        case WSATYPE_NOT_FOUND:     return ErrorTypeNotFound; break;           
        case WSAHOST_NOT_FOUND:     return ErrorHostNotFound; break;           
        case WSATRY_AGAIN:          return ErrorTryAgain; break;               
        case WSANO_RECOVERY:        return ErrorNoRecovery; break;             
        case WSANO_DATA:            return ErrorNoData; break;                 
        default:                    
            return ErrorUnknown; 
            break;
    }
}

//------------------------------------------------------------------------------
/**
    Преобразуйте код ошибки в удобочитаемое сообщение об ошибке.
*/
String
Win32Socket::ErrorAsString(ErrorCode err)
{
    switch (err)
    {
        case ErrorNone:                      return "Никакой ошибки.";
        case ErrorUnknown:                   return "Unknown error (not mapped by Win32Socket class).";
        case ErrorInterrupted:               return "Прерванный вызов функции.";
        case ErrorPermissionDenied:          return "Доступ запрещен.";
        case ErrorBadAddress:                return "Плохой адрес.";
        case ErrorInvalidArgument:           return "Неверный аргумент.";
        case ErrorTooManyOpenFiles:          return "Слишком много открытых файлов (сокетов).";
        case ErrorWouldBlock:                return "Операция будет заблокирована.";
        case ErrorInProgress:                return "Сейчас идет операция.";
        case ErrorAlreadyInProgress:         return "Операция уже идет.";
        case ErrorNotASocket:                return "Работа сокета без сокета.";
        case ErrorDestAddrRequired:          return "Требуется адрес назначения";
        case ErrorMsgTooLong:                return "Сообщение слишком длинное.";
        case ErrorInvalidProtocol:           return "Неправильный тип протокола для сокета.";
        case ErrorBadProtocolOption:         return "Плохой вариант протокола.";
        case ErrorProtocolNotSupported:      return "Протокол не поддерживается.";
        case ErrorSocketTypeNotSupported:    return "Тип сокета не поддерживается.";
        case ErrorOperationNotSupported:     return "Операция не поддерживается.";
        case ErrorProtFamilyNotSupported:    return "Семейство протоколов не поддерживается.";
        case ErrorAddrFamilyNotSupported:    return "Семейство адресов не поддерживается семейством протоколов.";
        case ErrorAddrInUse:                 return "Адрес уже используется.";
        case ErrorAddrNotAvailable:          return "Невозможно назначить запрошенный адрес.";
        case ErrorNetDown:                   return "Сеть не работает.";
        case ErrorNetUnreachable:            return "Сеть недоступна.";
        case ErrorNetReset:                  return "Сеть разорвала соединение при перезагрузке.";
        case ErrorConnectionAborted:         return "Программное обеспечение вызвало разрыв соединения.";
        case ErrorConnectionReset:           return "Сброс соединения партнером.";
        case ErrorNoBufferSpace:             return "Нет доступного буферного пространства.";
        case ErrorIsConnected:               return "Розетка уже подключена.";
        case ErrorNotConnected:              return "Розетка не подключена.";
        case ErrorIsShutdown:                return "Невозможно отправить после выключения сокета.";
        case ErrorIsTimedOut:                return "Время подключения истекло.";
        case ErrorConnectionRefused:         return "В соединении отказано.";
        case ErrorHostDown:                  return "Хост не работает.";
        case ErrorHostUnreachable:           return "Нет маршрута к хосту.";
        case ErrorTooManyProcesses:          return "Слишком много процессов.";
        case ErrorSystemNotReady:            return "Сетевая подсистема недоступна.";
        case ErrorVersionNotSupported:       return "Версия Winsock.dll вне допустимого диапазона.";
        case ErrorNotInitialized:            return "Успешный запуск WSAStartup еще не выполнен.";
        case ErrorDisconnecting:             return "Выполняется плавное отключение.";
        case ErrorTypeNotFound:              return "Тип класса не найден.";
        case ErrorHostNotFound:              return "Хост не найден.";
        case ErrorTryAgain:                  return "Неавторитетный хост не найден.";
        case ErrorNoRecovery:                return "Это неисправимая ошибка.";
        case ErrorNoData:                    return "Допустимое имя, нет записи данных запрошенного типа.";
        default:
            n_error("Win32Socket::ErrorAsString(): необработанный код ошибки!");
            return "";
    }
}

//------------------------------------------------------------------------------
/**
*/
String
Win32Socket::WSAErrorToString(int wsaError)
{
    return ErrorAsString(WSAErrorToErrorCode(wsaError));
}

//------------------------------------------------------------------------------
/**
*/
String
Win32Socket::GetErrorString() const
{
    return ErrorAsString(this->error);
}

//------------------------------------------------------------------------------
/**
*/
bool 
Win32Socket::IsNetworkInitialized()
{
    return NetworkInitialized;
}
} // пространство имен Win32
