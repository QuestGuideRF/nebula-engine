//------------------------------------------------------------------------------
//  stringatompagehandler.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "core/debug/stringatompagehandler.h"
#include "http/html/htmlpagewriter.h"
#include "util/globalstringatomtable.h"
#include "math/scalar.h"

namespace Debug
{
__ImplementClass(Debug::StringAtomPageHandler, 'SAPH', Http::HttpRequestHandler);

using namespace IO;
using namespace Http;
using namespace Util;
using namespace Math;

//------------------------------------------------------------------------------
/**
*/
StringAtomPageHandler::StringAtomPageHandler()
{
    this->SetName("StringAtoms");
    this->SetDesc("показать отладочную информацию о системе StringAtom");
    this->SetRootLocation("stringatom");
}

//------------------------------------------------------------------------------
/**
*/
void
StringAtomPageHandler::HandleRequest(const Ptr<HttpRequest>& request) 
{
    n_assert(HttpMethod::Get == request->GetMethod());
    Dictionary<String,String> query = request->GetURI().ParseQuery();

    // настроить средство записи HTML-страниц
    Ptr<HtmlPageWriter> htmlWriter = HtmlPageWriter::Create();
    htmlWriter->SetStream(request->GetResponseContentStream());
    htmlWriter->SetTitle("Информация о туманности StringAtom");
    if (htmlWriter->Open())
    {
        htmlWriter->Element(HtmlElement::Heading1, "Система СтрингАтом");
        htmlWriter->AddAttr("href", "/index.html");
        htmlWriter->Element(HtmlElement::Anchor, "Home");

        // получить текущее содержимое таблицы атомов строки и общий размер строки
        GlobalStringAtomTable::DebugInfo debugInfo = GlobalStringAtomTable::Instance()->GetDebugInfo();
        htmlWriter->Element(HtmlElement::Heading3, "Информация о таблице строковых атомов:");
        htmlWriter->Begin(HtmlElement::Table);
            htmlWriter->TableRow2("Количество струн: ", String::FromSize(debugInfo.strings.Size()));
            htmlWriter->TableRow2("Размер куска: ", String::FromSize(debugInfo.chunkSize));
            htmlWriter->TableRow2("Количество кусков: ", String::FromSize(debugInfo.numChunks));
            htmlWriter->TableRow2("Размер распределения: ", String::FromSize(debugInfo.allocSize));
            htmlWriter->TableRow2("Используемый размер: ", String::FromSize(debugInfo.usedSize));
            htmlWriter->TableRow2("Рост возможен: ", debugInfo.growthEnabled ? "yes" : "no");
        htmlWriter->End(HtmlElement::Table);

        // Дамп таблицы атомов строк
        htmlWriter->Element(HtmlElement::Heading3, "Дамп таблицы строковых атомов:");
        const SizeT maxElementsInTable = 100;
        IndexT minIndex = 0;
        IndexT maxIndex = Math::min(debugInfo.strings.Size(), maxElementsInTable);
        if (query.Contains("min"))
        {
            minIndex = Math::max(query["min"].AsInt(), 0);
        }
        if (query.Contains("max"))
        {
            maxIndex = Math::min(query["max"].AsInt(), debugInfo.strings.Size());
        }

        // напишите предыдущую/следующую ссылки
        if (minIndex > 0)
        {
            String str;
            str.Format("/stringatom?min=%d&max=%d", minIndex - maxElementsInTable, minIndex);
            htmlWriter->AddAttr("href", str);
            htmlWriter->Element(HtmlElement::Anchor, "<< предыдущая|");
        }
        if (maxIndex < debugInfo.strings.Size())
        {
            String str;
            str.Format("/stringatom?min=%d&max=%d", maxIndex, maxIndex + maxElementsInTable);
            htmlWriter->AddAttr("href", str);
            htmlWriter->Element(HtmlElement::Anchor, "|следующий >>");
        }
        htmlWriter->LineBreak();

        htmlWriter->AddAttr("border", "1");
        htmlWriter->AddAttr("rules", "cols");
        htmlWriter->Begin(HtmlElement::Table);
            htmlWriter->AddAttr("bgcolor", "lightsteelblue");
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableHeader, "Index");
                htmlWriter->Element(HtmlElement::TableHeader, "StringAtom");
            htmlWriter->End(HtmlElement::TableRow);
            IndexT i;
            for (i = minIndex; i < maxIndex; i++)
            {
                htmlWriter->Begin(HtmlElement::TableRow);
                    htmlWriter->TableRow2(String::FromInt(i), debugInfo.strings[i]);
                htmlWriter->End(HtmlElement::TableRow);
            }
        htmlWriter->End(HtmlElement::Table);
        htmlWriter->Close();
        request->SetStatus(HttpStatus::OK);
    }
    else
    {
        request->SetStatus(HttpStatus::InternalServerError);
    }
}

} // Отладка пространства имен