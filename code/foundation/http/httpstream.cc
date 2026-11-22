//------------------------------------------------------------------------------
//  httpstream.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/httpstream.h"
#include "http/httpclientregistry.h"

namespace Http
{
__ImplementClass(Http::HttpStream, 'HTST', IO::MemoryStream);

//------------------------------------------------------------------------------
/**
*/
HttpStream::HttpStream() :
    retries(5)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
HttpStream::~HttpStream()
{
    if (this->IsOpen())
    {
        this->Close();
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
HttpStream::Open()
{
    n_assert(!this->IsOpen());
    bool retval = false;

    if (MemoryStream::Open())
    {
        n_printf("HttpStream: открытие «%s»...", this->uri.AsString().AsCharPtr());

        HttpStatus::Code res = HttpStatus::ServiceUnavailable;
        int retryCount = retries;

        while (res == HttpStatus::ServiceUnavailable && retryCount > 0)
        {
            // создайте HTTP-клиент и откройте соединение
            Ptr<HttpClient> httpClient = HttpClientRegistry::Instance()->ObtainConnection(this->uri);
            if (httpClient->IsConnected())
            {
                AccessMode oldAccessMode = this->accessMode;
                this->accessMode = WriteAccess;
                res = httpClient->SendRequest(HttpMethod::Get, this->uri, this);
                this->accessMode = oldAccessMode;
                this->Seek(0, Stream::Begin);
                if (HttpStatus::OK == res)
                {
                    n_printf("ок!\n");
                    retval = true;
                }
                else if (res == HttpStatus::ServiceUnavailable)
                {
                    --retryCount;
                    if (retryCount > 0)
                    {
                        n_printf("...retrying");
                        // убить сокет на всякий случай
                        httpClient->Disconnect();
                    }
                    else
                    {
                        n_printf("..не удалось (%d)\n", res);
                    }
                }
                else
                {
                    n_printf("неуспешный! (%d)\n", res);
                }
                HttpClientRegistry::Instance()->ReleaseConnection(this->uri);
            }
        }
    }
    return retval;
}

} // пространство имен HTTP
