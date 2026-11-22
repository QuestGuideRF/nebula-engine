//------------------------------------------------------------------------------
//  httprequestreader.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/httprequestreader.h"
#include "io/textreader.h"

namespace Http
{
__ImplementClass(Http::HttpRequestReader, 'HRQR', IO::StreamReader);

using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
HttpRequestReader::HttpRequestReader() :
    isValidHttpRequest(false),
    httpMethod(HttpMethod::InvalidHttpMethod)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool
HttpRequestReader::ReadRequest()
{
    this->isValidHttpRequest = false;
        
    // прикрепите текстовый ридер к нашему потоку и разберите заголовок запроса
    Ptr<TextReader> textReader = TextReader::Create();
    textReader->SetStream(this->stream);
    if (textReader->Open())
    {
        // прочитайте первую строку запроса
        // должно быть «ПУТЬ МЕТОДА HTTP/1.1[CRLF]
        String headLine =  textReader->ReadLine();
        Array<String> headTokens = headLine.Tokenize(" ");
        if (headTokens.Size() != 3)
        {
            // неверный заголовок запроса
            textReader->Close();
            return false;
        }

        // проверьте, выглядит ли строка версии HTTP в порядке
        if (!String::MatchPattern(headTokens[2], "HTTP/*"))
        {
            // неверный заголовок запроса
            textReader->Close();
            return false;
        }

        // декодировать метод HTTP
        this->httpMethod = HttpMethod::FromString(headTokens[0]);

        // декодировать оставшиеся строки заголовка запроса
        String host;
        bool endOfHeader = false;
        while (!textReader->Eof() && !endOfHeader)
        {
            String curLine = textReader->ReadLine();
            if (curLine.IsValid())
            {
                if (String::MatchPattern(curLine, "Хозяин: *"))
                {
                    host = curLine;
                    host.SubstituteString("Host: ", "");
                }
            }
            else
            {
                endOfHeader = true;
            }
        }

        // построить URI
        String uriString;
        uriString.Format("http://%s%s", host.AsCharPtr(), headTokens[1].AsCharPtr());
        this->requestURI = uriString;

        this->isValidHttpRequest = true;
        textReader->Close();
        return true;
    }
    return false;
}

} // пространство имен HTTP