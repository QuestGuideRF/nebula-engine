//------------------------------------------------------------------------------
//  httprequestwriter.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/httprequestwriter.h"
#include "io/textwriter.h"

namespace Http
{
__ImplementClass(Http::HttpRequestWriter, 'HTRW', IO::StreamWriter);

using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
HttpRequestWriter::HttpRequestWriter() :
    httpMethod(HttpMethod::Get),
    userAgent("Mozilla")    // ПРИМЕЧАНИЕ. Веб-браузер требователен к строкам пользовательского агента, поэтому используйте что-то обычное.
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool
HttpRequestWriter::WriteRequestHeader()
{
    n_assert(this->IsOpen());
    n_assert(this->uri.IsValid());

    // прикрепите текстовый редактор к нашему потоку
    Ptr<TextWriter> textWriter = TextWriter::Create();
    textWriter->SetStream(this->stream);
    if (textWriter->Open())
    {
        // написать заголовок
        textWriter->WriteFormatted("%s /%s HTTP/1.1\r\n",
            HttpMethod::ToString(this->httpMethod).AsCharPtr(),
            this->uri.LocalPath().AsCharPtr());
        if (this->httpMethod == HttpMethod::Delete)
        {
            textWriter->WriteString("Длина контента: 0\r\n");
        }
        textWriter->WriteFormatted("Хост: %s\r\n", this->uri.Host().AsCharPtr());
        textWriter->WriteFormatted("Пользовательский агент: %s\r\n", this->userAgent.AsCharPtr());
        if (this->httpMethod == HttpMethod::Delete)
        {           
            textWriter->WriteString("Соединение: закрыть\r\n");
        }
        else
        {
            textWriter->WriteString("Поддержание жизни: 300\r\n");
            textWriter->WriteString("Соединение: поддержание активности\r\n");
        }               
        textWriter->WriteString("\r\n");
        textWriter->Close();
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
bool
HttpRequestWriter::WriteRequestHeaderWithBody(const Util::String & body)
{
    n_assert(this->IsOpen());
    n_assert(this->uri.IsValid());

    // прикрепите текстовый редактор к нашему потоку
    Ptr<TextWriter> textWriter = TextWriter::Create();
    textWriter->SetStream(this->stream);
    if (textWriter->Open())
    {
        // написать заголовок
        textWriter->WriteFormatted("%s /%s HTTP/1.1\r\n",
            HttpMethod::ToString(this->httpMethod).AsCharPtr(),
            this->uri.LocalPath().AsCharPtr());
        textWriter->WriteFormatted("Хост: %s\r\n", this->uri.Host().AsCharPtr());
        textWriter->WriteFormatted("Пользовательский агент: %s\r\n", this->userAgent.AsCharPtr());
        textWriter->WriteString("Поддержание жизни: 300\r\n");
        textWriter->WriteString("Соединение: поддержание активности\r\n");
        textWriter->WriteString("Тип контента: текстовый/обычный; кодировка = UTF-8\r\n");
        textWriter->WriteFormatted("Длина контента: %u\r\n", body.Length());
        textWriter->WriteString("\r\n");
        textWriter->WriteString(body);
        textWriter->Close();
        return true;
    }
    return false;
}
} // пространство имен HTTP