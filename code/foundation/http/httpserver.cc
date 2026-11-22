//------------------------------------------------------------------------------
//  httpserver.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/httpserver.h"
#include "http/httprequestreader.h"
#include "io/memorystream.h"

namespace Http
{
__ImplementClass(Http::HttpServer, 'HTPS', Core::RefCounted);
__ImplementSingleton(Http::HttpServer);

using namespace Util;
using namespace Net;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
HttpServer::HttpServer() :
    isOpen(false),
    isSingleThreadMode(false)
{
    __ConstructSingleton;
    this->ipAddress.SetHostName("any");
    this->ipAddress.SetPort(2100);
}

//------------------------------------------------------------------------------
/**
*/
HttpServer::~HttpServer()
{
    n_assert(!this->IsOpen());
    __DestructSingleton;        
}

//------------------------------------------------------------------------------
/**
*/
bool
HttpServer::Open()
{
    n_assert(!this->isOpen);
    this->isOpen = true;
    
    // настроить новый объект TcpServer
    this->tcpServer = TcpServer::Create();
    this->tcpServer->SetAddress(this->ipAddress);
    bool success = this->tcpServer->Open();

    // создать обработчик http-запросов по умолчанию
    this->defaultRequestHandler = DefaultHttpRequestHandler::Create();
    return success;
}

//------------------------------------------------------------------------------
/**
*/
void
HttpServer::Close()
{
    n_assert(this->isOpen);

    // очистить ожидающие запросы
    this->pendingRequests.Clear();

    // уничтожить обработчик http-запросов по умолчанию
    this->defaultRequestHandler = nullptr;

    // удалить обработчики запросов
    this->requestHandlers.Clear();

    // выключение TcpServer
    this->tcpServer->Close();
    this->tcpServer = nullptr;
    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
*/
void
HttpServer::AttachRequestHandler(const Ptr<HttpRequestHandler>& requestHandler)
{
    n_assert(requestHandler.isvalid());
    n_assert(this->isOpen);
    n_assert(!this->requestHandlers.Contains(requestHandler->GetRootLocation()));
    this->requestHandlers.Add(requestHandler->GetRootLocation(), requestHandler);
}

//------------------------------------------------------------------------------
/**
*/
void
HttpServer::RemoveRequestHandler(const Ptr<HttpRequestHandler>& requestHandler)
{
    n_assert(requestHandler.isvalid());
    n_assert(this->isOpen);
    this->requestHandlers.Erase(requestHandler->GetRootLocation());
}

//------------------------------------------------------------------------------
/**
*/
void
HttpServer::OnFrame()
{
    n_assert(this->isOpen);

    // обрабатывать ожидающие запросы клиентов
    Array<Ptr<TcpClientConnection> > recvConns = this->tcpServer->Recv();
    IndexT i;
    for (i = 0; i < recvConns.Size(); i++)
    {
        if (!this->HandleHttpRequest(recvConns[i]))
        {
            recvConns[i]->Shutdown();
        }
    }

    // обрабатывать обработанные http-запросы
    for (i = 0; i < this->pendingRequests.Size();)
    {
        const Ptr<HttpRequest>& httpRequest = this->pendingRequests[i].httpRequest;
        if (httpRequest->Handled())
        {
            const Ptr<TcpClientConnection>& conn = this->pendingRequests[i].clientConnection;
            if (this->BuildHttpResponse(conn, httpRequest))
            {
                conn->Send();
            }
            this->pendingRequests.EraseIndex(i);
        }
        else
        {
            i++;
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
HttpServer::HandleHttpRequest(const Ptr<TcpClientConnection>& clientConnection)  
{
    // расшифровать запрос
    Ptr<HttpRequestReader> httpRequestReader = HttpRequestReader::Create();
    httpRequestReader->SetStream(clientConnection->GetRecvStream());
    if (httpRequestReader->Open())
    {
        httpRequestReader->ReadRequest();
        httpRequestReader->Close();
    }
    if (httpRequestReader->IsValidHttpRequest())
    {
        URI requestURI = httpRequestReader->GetRequestURI();
    
        // создать поток контента для ответа
        Ptr<MemoryStream> responseContentStream = MemoryStream::Create();
        
        // создать объект HttpRequest
        Ptr<HttpRequest> httpRequest = HttpRequest::Create();
        httpRequest->SetMethod(httpRequestReader->GetHttpMethod());
        httpRequest->SetURI(httpRequestReader->GetRequestURI());
        httpRequest->SetResponseContentStream(responseContentStream.upcast<Stream>());
        httpRequest->SetStatus(HttpStatus::NotFound);

        // найти обработчик запроса, который принимает запрос
        Ptr<HttpRequestHandler> requestHandler;
        Array<String> tokens = requestURI.LocalPath().Tokenize("/");
        if (tokens.Size() > 0)
        {
            if (this->requestHandlers.Contains(tokens[0]))
            {
                requestHandler = this->requestHandlers[tokens[0]];
            }
        }
        if (requestHandler.isvalid())
        {
            // обработать запрос, по умолчанию используется асинхронная обработка 
            // (request is added to request handler with PutRequest()
            // и обрабатывается, когда поток, в котором находится обработчик запроса
            // жизни вызывает HandlePendingRequests()
            // в режиме SingleThread запрос будет обработан немедленно,
            // но это смертная казнь, если обработчики запросов будут жить дальше
            // разные темы!!!
            if (this->IsSingleThreadMode())
            {
                // немедленно обработать запрос
                requestHandler->HandleRequest(httpRequest);
                httpRequest->SetHandled(true);
            }
            else
            {
                // асинхронно обрабатывать запрос
                requestHandler->PutRequest(httpRequest);
            }
        }
        else
        {
            // ни один обработчик запроса не принимает запрос, пусть используется значение по умолчанию
            // обработчик запроса обработать запрос
            this->defaultRequestHandler->HandleRequest(httpRequest);
            httpRequest->SetHandled(true);
        }

        // добавить запрос в ожидающую очередь
        PendingRequest pendingRequest;
        pendingRequest.clientConnection = clientConnection;
        pendingRequest.httpRequest = httpRequest;
        this->pendingRequests.Append(pendingRequest);

        return true;
    }
    else
    {
        // полученные данные не были допустимым HTTP-запросом
        return false;
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
HttpServer::BuildHttpResponse(const Ptr<TcpClientConnection>& conn, const Ptr<HttpRequest>& httpRequest)
{
    Ptr<HttpResponseWriter> responseWriter = HttpResponseWriter::Create();
    responseWriter->SetStream(conn->GetSendStream());
    responseWriter->SetStatusCode(httpRequest->GetStatus());
    if (HttpStatus::OK != httpRequest->GetStatus())
    {
        // произошла ошибка, необходимо записать сообщение об ошибке в поток ответов
        Ptr<TextWriter> textWriter = TextWriter::Create();
        textWriter->SetStream(httpRequest->GetResponseContentStream());
        textWriter->Open();
        textWriter->WriteFormatted("%s %s", HttpStatus::ToString(httpRequest->GetStatus()).AsCharPtr(), HttpStatus::ToHumanReadableString(httpRequest->GetStatus()).AsCharPtr());
        textWriter->Close();
        httpRequest->GetResponseContentStream()->SetMediaType(MediaType("text/plain"));
    }
    if (httpRequest->GetResponseContentStream()->GetSize() > 0)
    {
        httpRequest->GetResponseContentStream()->GetMediaType().IsValid();
        responseWriter->SetContent(httpRequest->GetResponseContentStream());
    }
    // ничего не делать, если поток недействителен
    if (responseWriter->GetStream().isvalid())
    {
        responseWriter->Open();
        responseWriter->WriteResponse();
        responseWriter->Close();                 
        return true;
    }    
    return false;
}

} // пространство имен HTTP

