//------------------------------------------------------------------------------
//  memorypagehandler.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "memory/debug/memorypagehandler.h"
#include "memory/heap.h"
#include "http/html/htmlpagewriter.h"
#include "memory/poolarrayallocator.h"

namespace Debug
{
__ImplementClass(Debug::MemoryPageHandler, 'MPGH', Http::HttpRequestHandler);

using namespace IO;
using namespace Http;
using namespace Util;
using namespace Memory;

//------------------------------------------------------------------------------
/**
*/
MemoryPageHandler::MemoryPageHandler()
{
    this->SetName("Memory");
    this->SetDesc("показать информацию об отладке памяти");
    this->SetRootLocation("memory");
}

//------------------------------------------------------------------------------
/**
*/
void
MemoryPageHandler::HandleRequest(const Ptr<HttpRequest>& request) 
{
    n_assert(HttpMethod::Get == request->GetMethod());

    // настроить средство записи HTML-страниц
    Ptr<HtmlPageWriter> htmlWriter = HtmlPageWriter::Create();
    htmlWriter->SetStream(request->GetResponseContentStream());
    htmlWriter->SetTitle("Информация о памяти туманности");
    if (htmlWriter->Open())
    {
        htmlWriter->Element(HtmlElement::Heading1, "Memory");
        htmlWriter->AddAttr("href", "/index.html");
        htmlWriter->Element(HtmlElement::Anchor, "Home");
        htmlWriter->LineBreak();
        htmlWriter->LineBreak();

        // if not compiled with NEBULA_MEMORY_STATS, display a message
        #if (NEBULA_MEMORY_STATS == 0)
        htmlWriter->Text("Статистика памяти недоступна, поскольку NEBULA_MEMORY_STATS не определена. "
                         "когда приложение было скомпилировано. Перейдите в /foundation/core/config.h, измените "
                         "NEBULA_MEMORY_STATS в (1) и перекомпилируйте приложение!");
        #else

        htmlWriter->Text("Сюда входят все выделения, которые проходят через подсистему памяти. "
                         "функции (сюда входит создание всех объектов Nebula). Другая память "
                         "такие выделения, как ресурсы DirectX, НЕ включены!");
        htmlWriter->LineBreak();
        htmlWriter->LineBreak();

        // вывести общую статистику
        Array<Heap::Stats> heapStats = Heap::GetAllHeapStats();
        long heapAllocCount = 0;
        long heapAllocSize = 0;
        IndexT i;
        for (i = 0; i < heapStats.Size(); i++)
        {
            heapAllocCount += heapStats[i].allocCount;
            heapAllocSize  += heapStats[i].allocSize;
        };
        htmlWriter->Element(HtmlElement::Heading3, "Общая статистика Небулы");
        htmlWriter->Begin(HtmlElement::Table);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Число распределенных глобальных куч Nebula: ");
                htmlWriter->Element(HtmlElement::TableData, String::FromLong(Memory::TotalAllocCount));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Размер выделения глобальных куч Nebula: ");
                htmlWriter->Element(HtmlElement::TableData, String::FromLong(Memory::TotalAllocSize) + " bytes");
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Число выделенных локальных куч Nebula: ");
                htmlWriter->Element(HtmlElement::TableData, String::FromLong(heapAllocCount));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Размер выделения локальных куч туманности: ");
                htmlWriter->Element(HtmlElement::TableData, String::FromLong(heapAllocSize) + " bytes");
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Общее количество выделенных ресурсов Nebula: ");
                htmlWriter->Element(HtmlElement::TableData, String::FromLong(heapAllocCount + Memory::TotalAllocCount));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Общий размер выделения туманности: ");
                htmlWriter->Element(HtmlElement::TableData, String::FromLong(heapAllocSize + Memory::TotalAllocSize) + " bytes");
            htmlWriter->End(HtmlElement::TableRow);
        htmlWriter->End(HtmlElement::Table);

        // низкоуровневая системная статистика
        Memory::TotalMemoryStatus globalStats = Memory::GetTotalMemoryStatus();
        const long mega = 1024 * 1024;
        htmlWriter->Element(HtmlElement::Heading3, "Статистика системной памяти");
        htmlWriter->Begin(HtmlElement::Table);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Общая физическая память:");
                htmlWriter->Element(HtmlElement::TableData, String::FromInt(globalStats.totalPhysical / mega) + String(" MB"));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Доступная физическая память:");
                htmlWriter->Element(HtmlElement::TableData, String::FromInt(globalStats.availPhysical / mega) + String(" MB"));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Общая виртуальная память:");
                htmlWriter->Element(HtmlElement::TableData, String::FromInt(globalStats.totalVirtual / mega) + String(" MB"));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Доступная виртуальная память:");
                htmlWriter->Element(HtmlElement::TableData, String::FromInt(globalStats.availVirtual / mega) + String(" MB"));
            htmlWriter->End(HtmlElement::TableRow);
        htmlWriter->End(HtmlElement::Table);

        // отображать информацию о глобальных кучах
        htmlWriter->Element(HtmlElement::Heading3, "Глобальная статистика кучи");
        htmlWriter->AddAttr("border", "1");
        htmlWriter->AddAttr("rules", "cols");
        htmlWriter->Begin(HtmlElement::Table);
            htmlWriter->AddAttr("bgcolor", "lightsteelblue");
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableHeader, "Имя кучи");
                htmlWriter->Element(HtmlElement::TableHeader, "Количество выделенных мест");
                htmlWriter->Element(HtmlElement::TableHeader, "Выделить размер");
            htmlWriter->End(HtmlElement::TableRow);

            for (i = 0; i < Memory::NumHeapTypes; i++)
            {
                htmlWriter->Begin(HtmlElement::TableRow);
                    htmlWriter->Element(HtmlElement::TableData, Memory::GetHeapTypeName((Memory::HeapType)i));
                    htmlWriter->Element(HtmlElement::TableData, String::FromLong(Memory::HeapTypeAllocCount[i]));
                    htmlWriter->Element(HtmlElement::TableData, String::FromLong(Memory::HeapTypeAllocSize[i]));
                htmlWriter->End(HtmlElement::TableRow);
            }
        htmlWriter->End(HtmlElement::Table);

        // отображать информацию о локальных кучах
        htmlWriter->Element(HtmlElement::Heading3, "Статистика локальной кучи");
        htmlWriter->AddAttr("border", "1");
        htmlWriter->AddAttr("rules", "cols");
        htmlWriter->Begin(HtmlElement::Table);
            htmlWriter->AddAttr("bgcolor", "lightsteelblue");
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableHeader, "Имя кучи");
                htmlWriter->Element(HtmlElement::TableHeader, "Количество выделенных мест");
                htmlWriter->Element(HtmlElement::TableHeader, "Выделить размер");
            htmlWriter->End(HtmlElement::TableRow);

            for (i = 0; i < heapStats.Size(); i++)
            {
                htmlWriter->Begin(HtmlElement::TableRow);
                    htmlWriter->Element(HtmlElement::TableData, heapStats[i].name);
                    htmlWriter->Element(HtmlElement::TableData, String::FromInt(heapStats[i].allocCount));
                    htmlWriter->Element(HtmlElement::TableData, String::FromInt(heapStats[i].allocSize));
                htmlWriter->End(HtmlElement::TableRow);
            }
        htmlWriter->End(HtmlElement::Table);

        #endif // NEBULA_MEMORY_STATS
        htmlWriter->Close();
        request->SetStatus(HttpStatus::OK);
    }
    else
    {
        request->SetStatus(HttpStatus::InternalServerError);
    }
}

} // Отладка пространства имен
