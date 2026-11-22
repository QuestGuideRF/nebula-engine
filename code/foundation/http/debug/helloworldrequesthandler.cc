//------------------------------------------------------------------------------
//  helloworldrequesthandler.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/debug/helloworldrequesthandler.h"
#include "http/html/htmlpagewriter.h"

namespace Debug
{
__ImplementClass(Debug::HelloWorldRequestHandler, 'HWRH', Http::HttpRequestHandler);

using namespace Http;

//------------------------------------------------------------------------------
/**
*/
HelloWorldRequestHandler::HelloWorldRequestHandler()
{
    // нам нужно установить удобочитаемое имя, описание и
    // корневое расположение URI:
    this->SetName("Привет, мир");
    this->SetDesc("пример HttpRequestHandler");
    this->SetRootLocation("helloworld");
}

//------------------------------------------------------------------------------
/**
*/
void
HelloWorldRequestHandler::HandleRequest(const Ptr<HttpRequest>& request)
{
    // мы могли бы извлечь больше информации из URI, если захотим, но
    // поскольку это самый простой HttpRequestHandler, мы не будем :)

    // чтобы передать действительную HTML-страницу в ResponseContentStream запроса, 
    // нам нужно создать HtmlPageWriter и подключить его к 
    // ResponseContentStream, это стандартный материал Nebula IO...
    Ptr<HtmlPageWriter> htmlWriter = HtmlPageWriter::Create();
    htmlWriter->SetStream(request->GetResponseContentStream());
    htmlWriter->SetTitle("Привет, мир");
    if (htmlWriter->Open())
    {
        // написать стандартный текст на HTML-странице
        htmlWriter->Text("Привет, мир");
        htmlWriter->Close();

        // наконец установите код состояния HTTP
        request->SetStatus(HttpStatus::OK);
    }
    else
    {
        // этого не должно случиться, но если что-то пойдет не так, 
        // веб-браузер также должен знать
        request->SetStatus(HttpStatus::InternalServerError);
    }
}

} // Отладка пространства имен
