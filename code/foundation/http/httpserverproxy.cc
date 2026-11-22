//------------------------------------------------------------------------------
//  httpserverproxy.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/httpserverproxy.h"

#if __NEBULA_HTTP__
#include "http/httpprotocol.h"
#include "http/httpinterface.h"
#endif

namespace Http
{
__ImplementClass(Http::HttpServerProxy, 'HTSP', Core::RefCounted);
__ImplementSingleton(Http::HttpServerProxy);

//------------------------------------------------------------------------------
/**
*/
HttpServerProxy::HttpServerProxy() :
    isOpen(false)
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
HttpServerProxy::~HttpServerProxy()
{
    n_assert(!this->isOpen);
    __DestructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
void
HttpServerProxy::Open()
{
    n_assert(!this->isOpen);
    n_assert(this->requestHandlers.IsEmpty());
    this->isOpen = true;
}

//------------------------------------------------------------------------------
/**
*/
void
HttpServerProxy::Close()
{
    n_assert(this->isOpen);
    
    // обработчики запросов на очистку
    while (this->requestHandlers.Size() > 0)
    {
        this->RemoveRequestHandler(this->requestHandlers.Back());
    }
    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
*/
void
HttpServerProxy::AttachRequestHandler(const Ptr<HttpRequestHandler>& requestHandler)
{
    n_assert(this->isOpen);
    this->requestHandlers.Append(requestHandler);

#if __NEBULA_HTTP__
    if (HttpInterface::HasInstance())
    {
        // зарегистрировать обработчик запроса с потоком HttpServer 
        Ptr<Http::AttachRequestHandler> msg = Http::AttachRequestHandler::Create();
        msg->SetRequestHandler(requestHandler);
        HttpInterface::Instance()->Send(msg.cast<Messaging::Message>());
    }
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
HttpServerProxy::RemoveRequestHandler(const Ptr<HttpRequestHandler>& requestHandler)
{
    n_assert(this->isOpen);    
    IndexT index = this->requestHandlers.FindIndex(requestHandler);
    n_assert(InvalidIndex != index);
    
#if __NEBULA_HTTP__
    if (HttpInterface::HasInstance())
    {
        // отменить регистрацию обработчика запроса из потока HttpServer
        Ptr<Http::RemoveRequestHandler> msg = Http::RemoveRequestHandler::Create();
        msg->SetRequestHandler(requestHandler);
        HttpInterface::Instance()->Send(msg.cast<Messaging::Message>());
    }
#endif

    // delete from local array
    this->requestHandlers.EraseIndex(index);
}

//------------------------------------------------------------------------------
/**
*/
void
HttpServerProxy::HandlePendingRequests()
{
    n_assert(this->isOpen);

    IndexT i;
    for (i = 0; i < this->requestHandlers.Size(); i++)
    {
        this->requestHandlers[i]->HandlePendingRequests();
    }
}

} // пространство имен HTTP
