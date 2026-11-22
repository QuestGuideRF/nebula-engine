//------------------------------------------------------------------------------
//  historyconsolehandler.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/historyconsolehandler.h"

namespace IO
{
__ImplementClass(IO::HistoryConsoleHandler, 'HICH', IO::ConsoleHandler);

using namespace Util;

//------------------------------------------------------------------------------
/**
*/
HistoryConsoleHandler::HistoryConsoleHandler() :
    history(NEBULA_CONSOLE_HISTORY_SIZE)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
HistoryConsoleHandler::Print(const String& s)
{
    this->history.Add(s);
}

//------------------------------------------------------------------------------
/**
*/
void
HistoryConsoleHandler::Error(const String& s)
{
    String str("[ОШИБКА] ");
    str.Append(s);
    this->history.Add(str);
}

//------------------------------------------------------------------------------
/**
*/
void
HistoryConsoleHandler::Warning(const String& s)
{
    String str("[ПРЕДУПРЕЖДЕНИЕ] ");
    str.Append(s);
    this->history.Add(str);
}

//------------------------------------------------------------------------------
/**
*/
void
HistoryConsoleHandler::DebugOut(const String& s)
{
    String str("[ОТЛАЖИВАТЬ] ");
    str.Append(s);
    this->history.Add(str);
}
        
} // пространство имен ввода-вывода
