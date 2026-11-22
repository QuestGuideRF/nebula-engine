//------------------------------------------------------------------------------
//  httprequesthandler.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/httprequesthandler.h"

namespace Http
{
__ImplementClass(Http::HttpRequestHandler, 'HRHD', Core::RefCounted);

using namespace IO;
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
HttpRequestHandler::HttpRequestHandler()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
HttpRequestHandler::~HttpRequestHandler()
{
    this->pendingRequests.SetSignalOnEnqueueEnabled(false);
    // empty
}

//------------------------------------------------------------------------------
/**
    Put a http request into the request handlers message queue. This method
    is meant to be called from another thread.
*/
void
HttpRequestHandler::PutRequest(const Ptr<HttpRequest>& httpRequest)
{
    this->pendingRequests.Enqueue(httpRequest);
}

//------------------------------------------------------------------------------
/**
    Обработка всех ожидающих HTTP-запросов в очереди. Этот метод
    должен часто вызываться из потока, создавшего это
    обработчик запроса.
*/
void
HttpRequestHandler::HandlePendingRequests()
{
    this->curWorkRequests.Reserve(this->pendingRequests.Size());
    this->pendingRequests.DequeueAll(this->curWorkRequests);
    IndexT i;
    for (i = 0; i < this->curWorkRequests.Size(); i++)
    {
        this->HandleRequest(this->curWorkRequests[i]);
        this->curWorkRequests[i]->SetHandled(true);
    }
}

//------------------------------------------------------------------------------
/**
    Overwrite this method in your subclass. This method will be called by the
    HttpServer if AcceptsRequest() returned true. The request handler should
    properly process the request by filling the responseContentStream with
    data (for instance a HTML page), set the MediaType on the 
    responseContentStream (for instance "text/html") and return with a
    HttpStatus code (usually HttpStatus::OK).
*/
void
HttpRequestHandler::HandleRequest(const Ptr<HttpRequest>& request)
{
    request->SetStatus(HttpStatus::NotFound);
}

} // пространство имен HTTP
