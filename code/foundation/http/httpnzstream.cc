//------------------------------------------------------------------------------
//  httpnzstream.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/httpnzstream.h"
#include "http/httpclientregistry.h"
#include "zlib/zlib.h"

namespace Http
{
__ImplementClass(Http::HttpNzStream, 'HZST', IO::MemoryStream);

using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
HttpNzStream::HttpNzStream()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
HttpNzStream::~HttpNzStream()
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
HttpNzStream::Open()
{
    n_assert(!this->IsOpen());
    bool retval = false;

    // создать модифицированный URI
    String nzUriStr = this->uri.AsString();
    nzUriStr.Append(".nz");
    URI nzUri(nzUriStr);

    n_printf("HttpNzStream: открытие «%s»…", nzUri.AsString().AsCharPtr());

    // создайте HTTP-клиент и откройте соединение
    Ptr<HttpClient> httpClient = HttpClientRegistry::Instance()->ObtainConnection(nzUri);
    if (httpClient->IsConnected())
    {
        // создать поток памяти, содержащий сжатые данные
        Ptr<MemoryStream> srcStream = MemoryStream::Create();
        srcStream->SetAccessMode(Stream::WriteAccess);
        HttpStatus::Code res = httpClient->SendRequest(HttpMethod::Get, nzUri, srcStream.upcast<Stream>());
        if (HttpStatus::OK == res)
        {
            // доступ к сжатым исходным данным
            srcStream->SetAccessMode(Stream::ReadAccess);
            srcStream->Open();
            uint* srcPtr = (uint*) srcStream->Map();
            n_assert(srcStream->GetSize() < INT_MAX);
            uLong srcDataSize = (uLong)srcStream->GetSize() - 8;   // 8 — заголовок sizeof

            // проверьте магическое число и получите несжатый размер
            if (srcPtr[0] != 'NZ__')
            {
                n_error("HttpNzStream: поток «%s» не в формате .nz!\n", nzUri.AsString().AsCharPtr());
            }
            
            // распаковать в себя
            SizeT inflatedSize = srcPtr[1];
            this->SetSize(inflatedSize);
            MemoryStream::Open();
            void* dstPtr = this->Map();

            uLongf destLen = inflatedSize;
            int res = uncompress((Bytef*)dstPtr, &destLen, (const Bytef*)&(srcPtr[2]), srcDataSize);
            n_assert(Z_OK == res);

            this->Unmap();
            srcStream->Unmap();
            srcStream->Close();
            srcStream = nullptr;

            n_printf("ок!\n");
            retval = true;
        }
        else
        {
            n_printf("не удалось!\n");
        }
    }
    HttpClientRegistry::Instance()->ReleaseConnection(this->uri);
    return retval;
}

} // пространство имен HTTP
