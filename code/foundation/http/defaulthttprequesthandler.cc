//------------------------------------------------------------------------------
//  defaulthttprequesthandler.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "core/coreserver.h"
#include "http/defaulthttprequesthandler.h"
#include "timing/calendartime.h"
#include "http/html/htmlpagewriter.h"
#include "http/httpserver.h"
#include "http/httpinterface.h"

namespace Http
{
__ImplementClass(Http::DefaultHttpRequestHandler, 'DHRH', Http::HttpRequestHandler);

using namespace Core;
using namespace Util;
using namespace Timing;

//------------------------------------------------------------------------------
/**
*/
void
DefaultHttpRequestHandler::HandleRequest(const Ptr<HttpRequest>& request)
{
    // ничего не принимай, тогда получи
    if (HttpMethod::Get != request->GetMethod())
    {
        request->SetStatus(HttpStatus::NotImplemented);
        return;
    }
    
    // HttpInterface может отсутствовать на некоторых платформах.
    StringAtom companyName("unknown");
    StringAtom appName("unknown");
    StringAtom rootDir("unknown");
    if (HttpInterface::HasInstance())
    {
        companyName = HttpInterface::Instance()->GetCompanyName();
        appName = HttpInterface::Instance()->GetAppName();
        rootDir = HttpInterface::Instance()->GetRootDirectory();
    }
    
    // всегда показывать домашнюю страницу
    Ptr<HtmlPageWriter> htmlWriter = HtmlPageWriter::Create();
    htmlWriter->SetStream(request->GetResponseContentStream());
    htmlWriter->SetTitle("Приложение Небула");
    if (htmlWriter->Open())
    {
        htmlWriter->Element(HtmlElement::Heading1, "Главная страница");
        htmlWriter->Element(HtmlElement::Heading3, "Информация о приложении");
        htmlWriter->Begin(HtmlElement::Table);
            htmlWriter->Begin(HtmlElement::TableRow); 
                htmlWriter->Element(HtmlElement::TableData, "Название компании:");
                htmlWriter->Element(HtmlElement::TableData, companyName.Value());
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow); 
                htmlWriter->Element(HtmlElement::TableData, "Имя приложения:");
                htmlWriter->Element(HtmlElement::TableData, appName.Value());
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow); 
                htmlWriter->Element(HtmlElement::TableData, "Корневой каталог:");
                htmlWriter->Element(HtmlElement::TableData, rootDir.Value());        
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow); 
                htmlWriter->Element(HtmlElement::TableData, "Календарное время:");
                htmlWriter->Element(HtmlElement::TableData, CalendarTime::Format("{WEEKDAY} {DAY}.{MONTH}.{YEAR} {HOUR}:{MINUTE}:{SECOND}", CalendarTime::GetLocalTime()));
            htmlWriter->End(HtmlElement::TableRow);
        htmlWriter->End(HtmlElement::Table);

        // список зарегистрированных HttpRequestHandlers
        htmlWriter->Element(HtmlElement::Heading3, "Доступные страницы");        
        Array<Ptr<HttpRequestHandler> > handlers = HttpServer::Instance()->GetRequestHandlers();
        if (handlers.Size() > 0)
        {
            htmlWriter->Begin(HtmlElement::UnorderedList);
            IndexT i;
            for (i = 0; i < handlers.Size(); i++)
            {
                const Ptr<HttpRequestHandler>& handler = handlers[i];
                htmlWriter->Begin(HtmlElement::ListItem);
                htmlWriter->AddAttr("href", handler->GetRootLocation());
                htmlWriter->Element(HtmlElement::Anchor, handler->GetName());
                htmlWriter->Text(String(" - ") + handler->GetDesc());
                htmlWriter->End(HtmlElement::ListItem);
            }
            htmlWriter->End(HtmlElement::UnorderedList);
        }
        htmlWriter->Close();
        request->SetStatus(HttpStatus::OK);
    }
    else
    {
        request->SetStatus(HttpStatus::InternalServerError);
    }
}

} // пространство имен HTTP