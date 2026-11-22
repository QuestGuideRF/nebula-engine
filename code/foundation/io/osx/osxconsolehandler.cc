//------------------------------------------------------------------------------
//  osxconsolehandler.cc
//  (C) 2010 Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "osxconsolehandler.h"
#include "core/coreserver.h"
#include "core/sysfunc.h"

namespace OSX
{
__ImplementClass(OSX::OSXConsoleHandler, 'OSXC', IO::ConsoleHandler);
    
using namespace Util;
    
//------------------------------------------------------------------------------
/**
 */
void
OSXConsoleHandler::Print(const String& s)
{
    fprintf(stdout, "%s", s.AsCharPtr());
}
    
//------------------------------------------------------------------------------
/**
 */
void
OSXConsoleHandler::DebugOut(const String& s)
{
    fprintf(stderr, "%s", s.AsCharPtr());
}
    
//------------------------------------------------------------------------------
/**
 */
void
OSXConsoleHandler::Error(const String& msg)
{
    const char* appName = "???";
    if (Core::CoreServer::HasInstance())
    {
        appName = Core::CoreServer::Instance()->GetAppName().Value();
    }
    String str;
    str.Format("*** ОШИБКА ***\nПриложение: %s\nОшибка: %s", appName, msg.AsCharPtr());
    Core::SysFunc::Error(str.AsCharPtr());
}
    
//------------------------------------------------------------------------------
/**
 */
void
OSXConsoleHandler::Warning(const String& s)
{
    Core::SysFunc::Error(s.AsCharPtr());
}
    
//------------------------------------------------------------------------------
/**
 */
void
OSXConsoleHandler::Confirm(const String& s)
{
    Core::SysFunc::MessageBox(s.AsCharPtr());
}
    
//------------------------------------------------------------------------------
/**
*/
bool
OSXConsoleHandler::HasInput()
{
    // консольный ввод не реализован в OSX
    return false;
}
    
//------------------------------------------------------------------------------
/**
*/
String
OSXConsoleHandler::GetInput()
{
    // консольный ввод не реализован в OSX
    return "";
}
    
} // пространство имен OSX