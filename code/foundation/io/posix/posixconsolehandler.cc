//------------------------------------------------------------------------------
//  posixconsolehandler.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "io/posix/posixconsolehandler.h"
#include "core/coreserver.h"
#include "core/sysfunc.h"
#include "threading/thread.h"

namespace Posix
{
__ImplementClass(Posix::PosixConsoleHandler, 'PXSC', IO::ConsoleHandler);

using namespace Util;

//------------------------------------------------------------------------------
/**
*/
PosixConsoleHandler::PosixConsoleHandler()
{
    // получить дескриптор стандартного вывода
    this->stdoutHandle = stdout;
    this->stdinHandle = stdin;
    this->stderrHandle = stderr;
    n_assert(NULL != this->stdoutHandle);
    n_assert(NULL != this->stdinHandle);
    n_assert(NULL != this->stderrHandle);
}

//------------------------------------------------------------------------------
/**
*/
void
PosixConsoleHandler::Print(const String& s)
{
    const char* threadName = Threading::Thread::GetMyThreadName();
    String msg;
    if (0 == threadName)
    {
        // сообщение из основной темы
        fprintf(this->stdoutHandle, s.AsCharPtr());
    }
    else
    {
        // сообщение из другой темы, добавьте название темы к сообщению
        msg.Format("[%s] %s", Threading::Thread::GetMyThreadName(), s.AsCharPtr());
        fprintf(this->stdoutHandle, msg.AsCharPtr());
    }
}

//------------------------------------------------------------------------------
/**
*/
void
PosixConsoleHandler::DebugOut(const String& s)
{
    const char* threadName = Threading::Thread::GetMyThreadName();
    String msg;
    if (0 == threadName)
    {
        // сообщение из основной темы
        fprintf(this->stderrHandle, s.AsCharPtr());
    }
    else
    {
        String msg;
        msg.Format("[%s] %s", Threading::Thread::GetMyThreadName(), s.AsCharPtr());
        fprintf(this->stderrHandle, msg.AsCharPtr());
    }
}

//------------------------------------------------------------------------------
/**
*/
void
PosixConsoleHandler::Error(const String& msg)
{
    const char* threadName = Threading::Thread::GetMyThreadName();
    const char* appName = "???";
    if (0 == threadName)
    {
        threadName = "Main";
    }
    if (Core::CoreServer::HasInstance())
    {
        appName = Core::CoreServer::Instance()->GetAppName().Value();
    }
    String str;
    str.Format("*** ОШИБКА ***\nПриложение: %s\nПоток: %s\nОшибка: %s", appName, threadName, msg.AsCharPtr());
    fprintf(this->stderrHandle, str.AsCharPtr());
    Core::SysFunc::Error(str.AsCharPtr());
}

//------------------------------------------------------------------------------
/**
*/
void
PosixConsoleHandler::Warning(const String& s)
{
    const char* threadName = Threading::Thread::GetMyThreadName();
    String msg;
    if (0 == threadName)
    {
        // сообщение из основной темы
        fprintf(this->stderrHandle, s.AsCharPtr());
    }
    else
    {
        // сообщение из другой темы, добавьте название темы к сообщению
        msg.Format("[%s] %s", Threading::Thread::GetMyThreadName(), s.AsCharPtr());
        fprintf(this->stderrHandle, msg.AsCharPtr());
    }
}

//------------------------------------------------------------------------------
/**
    Поскольку мы блокируем приложение, ожидающее ввода пользователя, мы всегда предоставляем 
    вход.
*/
bool
PosixConsoleHandler::HasInput()
{
    return !feof(this->stdinHandle);
}

//------------------------------------------------------------------------------
/**
    Получите пользовательский ввод с консоли.
*/
String
PosixConsoleHandler::GetInput()
{
    String result;
    char buf[4096];
    char * bufRead = fgets(buf, sizeof(buf), this->stdinHandle);
    if (NULL != bufRead)
    {
        result.SetCharPtr(buf);
        result.TrimRight("\n\r");
    }
    return result;
}

}; // пространство имен ввода-вывода
