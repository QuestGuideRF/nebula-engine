//------------------------------------------------------------------------------
//  consoleapplication.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "app/consoleapplication.h"
#include "io/logfileconsolehandler.h"

namespace App
{
using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
ConsoleApplication::ConsoleApplication()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ConsoleApplication::~ConsoleApplication()
{
    n_assert(!this->IsOpen());
}

//------------------------------------------------------------------------------
/**
*/
bool
ConsoleApplication::Open()
{
    n_assert(!this->IsOpen());
    if (Application::Open())
    {
        // инициализировать базовую подсистему
        this->coreServer = Core::CoreServer::Create();
        this->coreServer->SetCompanyName(this->companyName);
        this->coreServer->SetAppName(this->appName);
        this->coreServer->Open();

        // инициализировать подсистему io
        this->ioServer = IoServer::Create();

        // прикрепите обработчик консоли файла журнала
        #if __WIN32__
        Ptr<LogFileConsoleHandler> logFileHandler = LogFileConsoleHandler::Create();
        Console::Instance()->AttachHandler(logFileHandler.upcast<ConsoleHandler>());
        #endif

        return true;   
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
ConsoleApplication::Close()
{
    n_assert(this->IsOpen());

    // выключение подсистемы ввода-вывода
    this->ioServer = nullptr;

    // выключение основной подсистемы
    this->coreServer = nullptr;

    Application::Close();
}

} // Приложение пространства имен
