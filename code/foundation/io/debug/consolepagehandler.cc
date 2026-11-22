//------------------------------------------------------------------------------
//  consolepagehandler.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/debug/consolepagehandler.h"
#include "io/console.h"
#include "http/html/htmlpagewriter.h"

namespace Debug
{
__ImplementClass(Debug::ConsolePageHandler, 'COPH', Http::HttpRequestHandler);

using namespace Http;
using namespace IO;
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
ConsolePageHandler::ConsolePageHandler()
{
    // обработчик запроса на настройку
    this->SetName("Console");
    this->SetDesc("показать журнал консоли");
    this->SetRootLocation("console");

    // прикрепите обработчик консоли истории, который отслеживает сообщения журнала
    this->historyConsoleHandler = HistoryConsoleHandler::Create();
    Console::Instance()->AttachHandler(this->historyConsoleHandler.upcast<ConsoleHandler>());
}

//------------------------------------------------------------------------------
/**
*/
ConsolePageHandler::~ConsolePageHandler()
{
    this->historyConsoleHandler = nullptr;
}

//------------------------------------------------------------------------------
/**
*/
void
ConsolePageHandler::HandleRequest(const Ptr<HttpRequest>& request)
{
    Ptr<HtmlPageWriter> htmlWriter = HtmlPageWriter::Create();
    htmlWriter->SetStream(request->GetResponseContentStream());
    htmlWriter->SetTitle("Журнал консоли");
    if (htmlWriter->Open())
    {
        htmlWriter->Element(HtmlElement::Heading1, "Журнал консоли");

        // получить текущую историю журнала консоли
        const RingBuffer<String>& history = this->historyConsoleHandler->GetHistory();
        SizeT num = history.Size();
        IndexT i;
        for (i = 0; i < num; i++)
        {
            htmlWriter->Text(history[i]);
            htmlWriter->LineBreak();
        }
        htmlWriter->Close();
        request->SetStatus(HttpStatus::OK);
    }
    else
    {
        request->SetStatus(HttpStatus::InternalServerError);
    }
}

} // пространство имен ввода-вывода
