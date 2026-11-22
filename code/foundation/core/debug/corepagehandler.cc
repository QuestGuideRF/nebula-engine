//------------------------------------------------------------------------------
//  corepagehandler.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "core/debug/corepagehandler.h"
#include "http/html/htmlpagewriter.h"
#include "system/systeminfo.h"
#include "core/refcounted.h"

namespace Debug
{
__ImplementClass(Debug::CorePageHandler, 'CPGH', Http::HttpRequestHandler);

using namespace IO;
using namespace Http;
using namespace Util;
using namespace Core;
using namespace System;

//------------------------------------------------------------------------------
/**
*/
CorePageHandler::CorePageHandler()
{
    this->SetName("Core");
    this->SetDesc("показать отладочную информацию о подсистеме Core");
    this->SetRootLocation("core");
}

//------------------------------------------------------------------------------
/**
*/
void
CorePageHandler::HandleRequest(const Ptr<HttpRequest>& request) 
{
    n_assert(HttpMethod::Get == request->GetMethod());

    // настроить средство записи HTML-страниц
    Ptr<HtmlPageWriter> htmlWriter = HtmlPageWriter::Create();
    htmlWriter->SetStream(request->GetResponseContentStream());
    htmlWriter->SetTitle("Основная информация о туманности");
    if (htmlWriter->Open())
    {
        htmlWriter->Element(HtmlElement::Heading1, "Основная подсистема");
        htmlWriter->AddAttr("href", "/index.html");
        htmlWriter->Element(HtmlElement::Anchor, "Home");

        // написать основную информацию о системе
        htmlWriter->Element(HtmlElement::Heading3, "Информация о системе");
        SystemInfo systemInfo;
        htmlWriter->Begin(HtmlElement::Table);        
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Хост-платформа:");
                htmlWriter->Element(HtmlElement::TableData, System::PlatformTypeAsString(System::Platform));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Тип процессора:");
                htmlWriter->Element(HtmlElement::TableData, System::CpuArchTypeAsString(System::CpuArch));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Ядра процессора:");
                htmlWriter->Element(HtmlElement::TableData, String::FromInt(System::NumCpuCores));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Размер страницы:");
                htmlWriter->Element(HtmlElement::TableData, String::FromInt(System::PageSize));
            htmlWriter->End(HtmlElement::TableRow);
        htmlWriter->End(HtmlElement::Table);
            
        // if not compiled with NEBULA_DEBUG, display a message
        #if (NEBULA_DEBUG == 0)
        htmlWriter->LineBreak();
        htmlWriter->LineBreak();
        htmlWriter->Text("Статистика RefCounted недоступна, поскольку приложение не было скомпилировано с отладочной информацией!");
        #else
        htmlWriter->Element(HtmlElement::Heading3, "Экземпляры с подсчетом повторов");
        Dictionary<String, RefCounted::Stats> refCountedStats = RefCounted::GetOverallStats();
        /// подсчитать общее количество экземпляров
        IndexT i;
        SizeT overallNumInstances = 0;
        for (i = 0; i < refCountedStats.Size(); i++)
        {
            overallNumInstances += refCountedStats.ValueAtIndex(i).numObjects;
        }
        htmlWriter->Text("Количество зарегистрированных классов: " + String::FromInt(refCountedStats.Size()));
        htmlWriter->LineBreak();
        htmlWriter->Text("Общее количество экземпляров: " + String::FromInt(overallNumInstances));
        htmlWriter->LineBreak();
        htmlWriter->LineBreak();

        htmlWriter->AddAttr("border", "1");
        htmlWriter->AddAttr("rules", "cols");
        htmlWriter->Begin(HtmlElement::Table);
            htmlWriter->AddAttr("bgcolor", "lightsteelblue");
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableHeader, "ClassName");
                htmlWriter->Element(HtmlElement::TableHeader, "ClassFourCC");
                htmlWriter->Element(HtmlElement::TableHeader, "InstanceSize");
                htmlWriter->Element(HtmlElement::TableHeader, "NumInstances");
                htmlWriter->Element(HtmlElement::TableHeader, "NumReferences");
            htmlWriter->End(HtmlElement::TableRow);

            for (i = 0; i < refCountedStats.Size(); i++)
            {
                const RefCounted::Stats& curStats = refCountedStats.ValueAtIndex(i);
                htmlWriter->Begin(HtmlElement::TableRow);
                    htmlWriter->Element(HtmlElement::TableData, curStats.className);
                    if (curStats.classFourCC.IsValid())
                    {
                        htmlWriter->Element(HtmlElement::TableData, curStats.classFourCC.AsString());
                    }
                    else
                    {
                        // Устаревшие классы Мангалора не имеют кода класса FourCC.
                        htmlWriter->Element(HtmlElement::TableData, "<NULL>");
                    }
                    htmlWriter->Element(HtmlElement::TableData, String::FromInt(curStats.instanceSize));
                    htmlWriter->Element(HtmlElement::TableData, String::FromInt(curStats.numObjects));
                    htmlWriter->Element(HtmlElement::TableData, String::FromInt(curStats.overallRefCount));
                htmlWriter->End(HtmlElement::TableRow);
            }
        htmlWriter->End(HtmlElement::Table);
        #endif
        htmlWriter->Close();
        request->SetStatus(HttpStatus::OK);
    }
    else
    {
        request->SetStatus(HttpStatus::InternalServerError);
    }
}

} // Отладка пространства имен
