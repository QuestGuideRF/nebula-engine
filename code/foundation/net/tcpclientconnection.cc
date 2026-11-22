//------------------------------------------------------------------------------
//  tcpclientconnection.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "net/tcpclientconnection.h"

namespace Net
{
#if (__WIN32__ || __linux__)
__ImplementClass(Net::TcpClientConnection, 'TPCC', Net::StdTcpClientConnection);
#else
#error "Net::TcpClientConnection не реализован на этой платформе!"
#endif
}