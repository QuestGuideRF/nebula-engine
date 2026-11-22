//------------------------------------------------------------------------------
//  httpresponsereader.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/httpresponsereader.h"
#include "io/textreader.h"

namespace Http
{
__ImplementClass(Http::HttpResponseReader, 'HTRR', IO::StreamReader);

using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
HttpResponseReader::HttpResponseReader() :
    isValidHttpResponse(false),
    httpStatus(HttpStatus::InvalidHttpStatus),
    contentLength(0)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool
HttpResponseReader::ReadResponse()
{
    this->isValidHttpResponse = false;
    this->httpStatus = HttpStatus::InvalidHttpStatus;
    this->contentLength = 0;
    this->contentType.Clear();
    bool retval = false;
        
    // подключите читалку текста к нашему потоку и разберите заголовок ответа
    Ptr<TextReader> textReader = TextReader::Create();
    textReader->SetStream(this->stream);
    if (textReader->Open())
    {
        // прочитайте первую строку, она должна быть "HTTP/xx STATUSCODE STATUSSTRING"
        String headLine = textReader->ReadLine();
        Array<String> headTokens = headLine.Tokenize(" ");
        if (headTokens.Size() < 3)
        {
            // неверный заголовок ответа
            return false;
        }

        // проверьте, выглядит ли строка версии HTTP в порядке
        if (!String::MatchPattern(headTokens[0], "HTTP/*"))
        {
            // неверный заголовок запроса
            return false;
        }

        // расшифровать код состояния
        this->httpStatus = HttpStatus::FromString(headTokens[1]);
        if (HttpStatus::OK == this->httpStatus)
        {
            // декодировать оставшиеся строки заголовка запроса
            bool endOfHeader = false;
            while (!textReader->Eof() && !endOfHeader)
            {
                String curLine = textReader->ReadLine();
                Array<String> tokens;
                curLine.TrimRight("\r");
                if (curLine.IsValid())
                {
                    if (String::MatchPattern(curLine, "Длина контента: *"))
                    {
                        // читать длину контента
                        tokens = curLine.Tokenize(" :");
                        n_assert(tokens.Size() == 2);
                        this->contentLength = tokens[1].AsInt();
                    }
                    else if (String::MatchPattern(curLine, "Тип контента: *"))
                    {
                        // прочитать тип MIME содержимого
                        tokens = curLine.Tokenize(" :");
                        n_assert(tokens.Size() == 2);
                        this->contentType.Set(tokens[1]);
                    }
                }
                else
                {
                    endOfHeader = true;
                }
            }
            retval = true;
        }
        this->isValidHttpResponse = true;
        textReader->Close();
    }
    return retval;
}

} // пространство имен HTTP
