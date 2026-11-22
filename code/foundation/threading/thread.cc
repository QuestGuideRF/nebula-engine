//------------------------------------------------------------------------------
//  thread.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "threading/thread.h"

namespace Threading
{
#if __WIN32__
__ImplementClass(Threading::Thread, 'TRED', Win32::Win32Thread);
#elif __OSX__
__ImplementClass(Threading::Thread, 'TRED', OSX::OSXThread);
#elif __linux__
__ImplementClass(Threading::Thread, 'TRED', Linux::LinuxThread);
#else
#error "Класс потока не реализован на этой платформе!"
#endif

Threading::ThreadId MainThreadId = Threading::InvalidThreadId;
}
