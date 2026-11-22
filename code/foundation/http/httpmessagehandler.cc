//------------------------------------------------------------------------------
//  httpmessagehandler.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/httpmessagehandler.h"

namespace Http
{
__ImplementClass(Http::HttpMessageHandler, 'HTMH', Interface::InterfaceHandlerBase);

using namespace Interface;
using namespace Messaging;
using namespace Threading;

//------------------------------------------------------------------------------
/**
*/
HttpMessageHandler::HttpMessageHandler()
:DefaultTcpPort(2100)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
HttpMessageHandler::~HttpMessageHandler()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
HttpMessageHandler::Open()
{   
    n_assert(!this->IsOpen());
    InterfaceHandlerBase::Open();

    // настройка основной среды выполнения и центрального http-сервера
    this->httpServer = HttpServer::Create();
    this->httpServer->SetPort(DefaultTcpPort);
    this->httpServer->Open();
}

//------------------------------------------------------------------------------
/**
*/
void
HttpMessageHandler::Close()
{
    n_assert(this->IsOpen());

    this->httpServer->Close();
    this->httpServer = nullptr;

    InterfaceHandlerBase::Close();
}

//------------------------------------------------------------------------------
/**
    Время от времени запускает http-сервер.
*/
void
HttpMessageHandler::DoWork()
{
    n_assert(this->IsOpen());

    // обрабатывать http-запросы только время от времени
    this->httpServer->OnFrame();
    Core::SysFunc::Sleep(0.1f);   
}

//------------------------------------------------------------------------------
/**
*/
bool
HttpMessageHandler::HandleMessage(const Ptr<Message>& msg)
{
    n_assert(msg.isvalid());

    if (msg->CheckId(AttachRequestHandler::Id))
    {
        this->OnAttachRequestHandler(msg.cast<AttachRequestHandler>());
    }
    else if (msg->CheckId(RemoveRequestHandler::Id))
    {
        this->OnRemoveRequestHandler(msg.cast<RemoveRequestHandler>());
    }
    else
    {
        // неизвестное сообщение
        return false;
    }
    // провал: сообщение было обработано
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
HttpMessageHandler::OnAttachRequestHandler(const Ptr<AttachRequestHandler>& msg)
{
    this->httpServer->AttachRequestHandler(msg->GetRequestHandler());
}

//------------------------------------------------------------------------------
/**
*/
void
HttpMessageHandler::OnRemoveRequestHandler(const Ptr<RemoveRequestHandler>& msg)
{
    this->httpServer->RemoveRequestHandler(msg->GetRequestHandler());
}

} // пространство имен HTTP
