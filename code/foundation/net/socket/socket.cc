//------------------------------------------------------------------------------
//  socket.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "net/socket/socket.h"

namespace Net
{
#if (__WIN32__)
__ImplementClass(Net::Socket, 'SOCK', Win32::Win32Socket);
#elif __linux__
__ImplementClass(Net::Socket, 'SOCK', Posix::PosixSocket);
#else
#error "Класс сокетов не реализован на этой платформе!"
#endif
}
