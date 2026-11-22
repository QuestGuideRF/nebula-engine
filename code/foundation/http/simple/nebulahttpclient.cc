//------------------------------------------------------------------------------
//  nebulahttpclient.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "http/simple/nebulahttpclient.h"
#include "http/httprequestwriter.h"
#include "http/httpresponsereader.h"

namespace Http
{
__ImplementClass(Http::NebulaHttpClient, 'NHTC', Core::RefCounted);

using namespace Util;
using namespace IO;
using namespace Net;

//------------------------------------------------------------------------------
/**
*/
NebulaHttpClient::NebulaHttpClient() :
    userAgent("Mozilla")    // ПРИМЕЧАНИЕ. Веб-браузер требователен к строкам пользовательского агента, поэтому используйте что-то обычное.
{
    this->tcpClient = TcpClient::Create();
    this->tcpClient->SetBlocking(true);
}

//------------------------------------------------------------------------------
/**
*/
NebulaHttpClient::~NebulaHttpClient()
{
    if (this->IsConnected())
    {
        this->Disconnect();
    }
    this->tcpClient = nullptr;
}

//------------------------------------------------------------------------------
/**
*/
bool
NebulaHttpClient::Connect(const URI& uri)
{
    n_assert(!this->IsConnected());
    n_assert(this->tcpClient->IsBlocking());

    // еще не подключен, настройте соединение через HTTP-сервер
    IpAddress ipAddress(uri);
    if (ipAddress.GetPort() == 0)
    {
        ipAddress.SetPort(80);
    }
    this->tcpClient->SetServerAddress(ipAddress);
    TcpClient::Result result = this->tcpClient->Connect();
    n_assert(result != TcpClient::Connecting);
    return (TcpClient::Success == result);
}

//------------------------------------------------------------------------------
/**
*/
void
NebulaHttpClient::Disconnect()
{
    if (this->IsConnected())
    {
        this->tcpClient->Disconnect();
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
NebulaHttpClient::IsConnected() const
{
    return this->tcpClient->IsConnected();
}

//------------------------------------------------------------------------------
/**
*/
HttpStatus::Code 
NebulaHttpClient::SendRequest(HttpMethod::Code requestMethod, const URI& uri, const Ptr<Stream>& responseContentStream)
{
    n_assert(this->IsConnected());

    // напишите правильный HTTP-запрос в поток отправки
    Ptr<HttpRequestWriter> httpRequestWriter = HttpRequestWriter::Create();
    httpRequestWriter->SetMethod(requestMethod);
    httpRequestWriter->SetURI(uri);
    httpRequestWriter->SetUserAgent(this->userAgent);
    httpRequestWriter->SetStream(this->tcpClient->GetSendStream());
    if (httpRequestWriter->Open())
    {
        httpRequestWriter->WriteRequestHeader();
        httpRequestWriter->Close();
    }
    else
    {
        return HttpStatus::ServiceUnavailable;
    }

    // отправьте http-запрос и дождитесь ответа
    if (!this->tcpClient->Send())
    {
        // отправить не удалось
        return HttpStatus::ServiceUnavailable;
    }

    // wait for the response (we're working in blocking mode)
    if (!this->tcpClient->Recv())
    {
        // получить не удалось
        return HttpStatus::ServiceUnavailable;
    }

    // декодировать ответ HTTP из потока приема
    Ptr<HttpResponseReader> httpResponseReader = HttpResponseReader::Create();
    httpResponseReader->SetStream(this->tcpClient->GetRecvStream());
    if (httpResponseReader->Open())
    {
        httpResponseReader->ReadResponse();
        if (httpResponseReader->IsValidHttpResponse())
        {
            if (HttpStatus::OK == httpResponseReader->GetStatusCode())
            {
                SizeT contentLength = httpResponseReader->GetContentLength();
                if (contentLength > 0)
                {
                    uchar* buf = (uchar*) Memory::Alloc(Memory::ScratchHeap, contentLength);
                    SizeT bufPos = 0;
                    SizeT bytesToRead = contentLength;
                    bool done = false;
                    while (!done)
                    {
                        const Ptr<Stream>& recvStream = this->tcpClient->GetRecvStream();
                        SizeT bytesRead = recvStream->Read(&(buf[bufPos]), bytesToRead);
                        bytesToRead -= bytesRead;
                        bufPos += bytesRead;
                        if (bytesToRead > 0)
                        {
                            recvStream->Close();
                            this->tcpClient->Recv();
                            recvStream->Open();
                        }
                        else
                        {
                            done = true;
                        }
                    }
                    if (responseContentStream->IsOpen())
                    {
                        // if stream is already open, simply append the read data
                        responseContentStream->Write(buf, contentLength);
                    }
                    else
                    {
                        // поток содержимого ответа не открыт, поэтому откройте -> напишите -> закройте
                        responseContentStream->SetAccessMode(Stream::WriteAccess);
                        if (responseContentStream->Open())
                        {
                            responseContentStream->Write(buf, contentLength);
                            responseContentStream->Close();
                        }
                        else
                        {
                            n_error("HttpClient: не удалось открыть «%s» для записи!", uri.AsString().AsCharPtr());
                            return HttpStatus::ServiceUnavailable;
                        }
                    }
                    Memory::Free(Memory::ScratchHeap, buf);
                }
            }
        }
        httpResponseReader->Close();
        return httpResponseReader->GetStatusCode();
    }
    // провал: ошибка
    return HttpStatus::ServiceUnavailable;
}

//------------------------------------------------------------------------------
/**
*/
HttpStatus::Code
NebulaHttpClient::SendRequest(HttpMethod::Code requestMethod, const IO::URI& uri, const Util::String & body, const Ptr<IO::Stream>& responseContentStream)
{
    n_assert(this->IsConnected());

    // напишите правильный HTTP-запрос в поток отправки
    Ptr<HttpRequestWriter> httpRequestWriter = HttpRequestWriter::Create();
    httpRequestWriter->SetMethod(requestMethod);
    httpRequestWriter->SetURI(uri);
    httpRequestWriter->SetUserAgent(this->userAgent);
    httpRequestWriter->SetStream(this->tcpClient->GetSendStream());
    if (httpRequestWriter->Open())
    {
        httpRequestWriter->WriteRequestHeaderWithBody(body);
        httpRequestWriter->Close();
    }
    else
    {
        return HttpStatus::ServiceUnavailable;
    }

    // отправьте http-запрос и дождитесь ответа
    if (!this->tcpClient->Send())
    {
        // отправить не удалось
        return HttpStatus::ServiceUnavailable;
    }

    // wait for the response (we're working in blocking mode)
    if (!this->tcpClient->Recv())
    {
        // получить не удалось
        return HttpStatus::ServiceUnavailable;
    }

    // декодировать ответ HTTP из потока приема
    Ptr<HttpResponseReader> httpResponseReader = HttpResponseReader::Create();
    httpResponseReader->SetStream(this->tcpClient->GetRecvStream());
    if (httpResponseReader->Open())
    {
        httpResponseReader->ReadResponse();
        if (httpResponseReader->IsValidHttpResponse())
        {
            if (HttpStatus::OK == httpResponseReader->GetStatusCode())
            {
                SizeT contentLength = httpResponseReader->GetContentLength();
                if (contentLength > 0)
                {
                    uchar* buf = (uchar*)Memory::Alloc(Memory::ScratchHeap, contentLength);
                    SizeT bufPos = 0;
                    SizeT bytesToRead = contentLength;
                    bool done = false;
                    while (!done)
                    {
                        const Ptr<Stream>& recvStream = this->tcpClient->GetRecvStream();
                        SizeT bytesRead = recvStream->Read(&(buf[bufPos]), bytesToRead);
                        bytesToRead -= bytesRead;
                        bufPos += bytesRead;
                        if (bytesToRead > 0)
                        {
                            recvStream->Close();
                            this->tcpClient->Recv();
                            recvStream->Open();
                        }
                        else
                        {
                            done = true;
                        }
                    }
                    if (responseContentStream->IsOpen())
                    {
                        // if stream is already open, simply append the read data
                        responseContentStream->Write(buf, contentLength);
                    }
                    else
                    {
                        // поток содержимого ответа не открыт, поэтому откройте -> напишите -> закройте
                        responseContentStream->SetAccessMode(Stream::WriteAccess);
                        if (responseContentStream->Open())
                        {
                            responseContentStream->Write(buf, contentLength);
                            responseContentStream->Close();
                        }
                        else
                        {
                            n_error("HttpClient: не удалось открыть «%s» для записи!", uri.AsString().AsCharPtr());
                            return HttpStatus::ServiceUnavailable;
                        }
                    }
                    Memory::Free(Memory::ScratchHeap, buf);
                }
            }
        }
        httpResponseReader->Close();
        return httpResponseReader->GetStatusCode();
    }
    // провал: ошибка
    return HttpStatus::ServiceUnavailable;
}
} // пространство имен HTTP