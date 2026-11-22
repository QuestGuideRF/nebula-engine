//------------------------------------------------------------------------------
//  debughandler.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "debug/debughandler.h"
#include "debug/debugpagehandler.h"
#include "timing/time.h"

namespace Debug
{
__ImplementClass(Debug::DebugHandler, 'DBGH', Interface::InterfaceHandlerBase);

using namespace Util;
using namespace Http;
using namespace Interface;

//------------------------------------------------------------------------------
/**
*/
DebugHandler::DebugHandler()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
DebugHandler::~DebugHandler()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Этот метод выполняется в контексте потока отладки и устанавливает необходимые
    время выполнения.
*/
void
DebugHandler::Open()
{
    n_assert(!this->IsOpen());
    InterfaceHandlerBase::Open();

    // настройка основной среды выполнения, сервера отладки и прокси-сервера http
    this->debugServer = DebugServer::Create();
    this->debugServer->Open();

    this->httpServerProxy = HttpServerProxy::Create();
    this->httpServerProxy->Open();
    this->httpServerProxy->AttachRequestHandler(Debug::DebugPageHandler::Create());
}

//------------------------------------------------------------------------------
/**
*/
void
DebugHandler::Close()
{
    n_assert(this->IsOpen());

    this->httpServerProxy->Close();
    this->httpServerProxy = nullptr;

    this->debugServer->Close();
    this->debugServer = nullptr;

    InterfaceHandlerBase::Close();
}

//------------------------------------------------------------------------------
/**
    Покадровый метод просто периодически проверяет, есть ли
    любые ожидающие обработки HttpRequests...
*/
void
DebugHandler::DoWork()
{
    n_assert(this->IsOpen());
    this->httpServerProxy->HandlePendingRequests();
    Timing::Sleep(0.1);
}

} // Отладка пространства имен
