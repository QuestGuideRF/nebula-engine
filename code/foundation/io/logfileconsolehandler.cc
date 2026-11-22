//------------------------------------------------------------------------------
//  logfileconsolehandler.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/logfileconsolehandler.h"
#include "io/ioserver.h"
#include "timing/calendartime.h"
#include "core/coreserver.h"

namespace IO
{
__ImplementClass(IO::LogFileConsoleHandler, 'LFCH', IO::ConsoleHandler);

using namespace Util;
using namespace Timing;

//------------------------------------------------------------------------------
/**
*/
LogFileConsoleHandler::LogFileConsoleHandler()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
LogFileConsoleHandler::~LogFileConsoleHandler()
{
    if (this->IsOpen())
    {
        this->Close();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
LogFileConsoleHandler::Open()
{
    ConsoleHandler::Open();
    IoServer* ioServer = IoServer::Instance();

    // убедитесь, что каталог журналов существует
    ioServer->CreateDirectory("bin:logfiles");

    // создать имя файла для файла журнала
    String calString = CalendarTime::Format("{YEAR}_{MONTH}_{DAY}_{HOUR}_{MINUTE}_{SECOND}", CalendarTime::GetLocalTime());
    String fileName;
    fileName.Format("bin:logfiles/%s_%s.txt", Core::CoreServer::Instance()->GetAppName().Value(), calString.AsCharPtr());

    // открыть поток файлов
    this->stream = ioServer->CreateStream(URI(fileName));
    this->textWriter = TextWriter::Create();
    this->textWriter->SetStream(this->stream);
    this->textWriter->Open();
}

//------------------------------------------------------------------------------
/**
*/
void
LogFileConsoleHandler::Close()
{
    n_assert(this->IsOpen());

    if (this->textWriter->IsOpen())
    {
        this->textWriter->Close();
    }
    this->textWriter = nullptr;
    this->stream = nullptr;
    
    ConsoleHandler::Close();
}

//------------------------------------------------------------------------------
/**
*/
void
LogFileConsoleHandler::Print(const String& s)
{
    if (this->textWriter->IsOpen())
    {
        this->textWriter->WriteString(s);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
LogFileConsoleHandler::Error(const String& s)
{
    if (this->textWriter->IsOpen())
    {
        String str("[ОШИБКА] ");
        str.Append(s);
        this->textWriter->WriteString(str);
        this->stream->Flush();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
LogFileConsoleHandler::Warning(const String& s)
{
    if (this->textWriter->IsOpen())
    {
        String str("[ПРЕДУПРЕЖДЕНИЕ] ");
        str.Append(s);
        this->textWriter->WriteString(str);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
LogFileConsoleHandler::DebugOut(const String& s)
{
    if (this->textWriter->IsOpen())
    {
        String str("[ОТЛАДКА] ");
        str.Append(s);
        this->textWriter->WriteString(str);
    }
}

} // пространство имен ввода-вывода
