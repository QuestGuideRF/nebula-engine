//------------------------------------------------------------------------------
//  iopagehandler.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/debug/iopagehandler.h"
#include "http/html/htmlpagewriter.h"
#include "io/assignregistry.h"
#include "io/ioserver.h"
#include "io/archfs/archivefilesystem.h"
#include "io/archfs/archive.h"
#include "io/console.h"

namespace Debug
{
__ImplementClass(Debug::IoPageHandler, 'DIOP', Http::HttpRequestHandler);

using namespace Http;
using namespace IO;
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
IoPageHandler::IoPageHandler()
{
    this->SetName("IO");
    this->SetDesc("показать отладочную информацию о подсистеме ввода-вывода");
    this->SetRootLocation("inout");
}

//------------------------------------------------------------------------------
/**
*/
void
IoPageHandler::HandleRequest(const Ptr<HttpRequest>& request) 
{
    n_assert(HttpMethod::Get == request->GetMethod());

    // сначала проверьте, определена ли команда в URI
    Dictionary<String,String> query = request->GetURI().ParseQuery();
    if (query.Contains("ls"))
    {
        this->ListDirectory(query["ls"], request);
        return;
    }

    // нет команды, показать корневую страницу
    Ptr<HtmlPageWriter> htmlWriter = HtmlPageWriter::Create();
    htmlWriter->SetStream(request->GetResponseContentStream());
    htmlWriter->SetTitle("Информация о подсистеме Nebula IO");
    if (htmlWriter->Open())
    {
        AssignRegistry* assignRegistry = AssignRegistry::Instance();
        SchemeRegistry* schemeRegistry = SchemeRegistry::Instance();
        htmlWriter->Element(HtmlElement::Heading1, "Подсистема ввода-вывода");
        htmlWriter->AddAttr("href", "/index.html");
        htmlWriter->Element(HtmlElement::Anchor, "Home");

        // отображать зарегистрированные схемы URI
        htmlWriter->Element(HtmlElement::Heading3, "Зарегистрированные схемы");
        htmlWriter->Begin(HtmlElement::Table);
        Array<String> uriSchemes = schemeRegistry->GetAllRegisteredUriSchemes();
        IndexT schemeIndex;
        for (schemeIndex = 0; schemeIndex < uriSchemes.Size(); schemeIndex++)
        {
            htmlWriter->Begin(HtmlElement::TableRow);
            htmlWriter->Element(HtmlElement::TableData, uriSchemes[schemeIndex]);
            htmlWriter->Element(HtmlElement::TableData, schemeRegistry->GetStreamClassByUriScheme(uriSchemes[schemeIndex]).GetName());
            htmlWriter->End(HtmlElement::TableRow);
        }
        htmlWriter->End(HtmlElement::Table);
        
        // отображение зарегистрированных назначений
        htmlWriter->Element(HtmlElement::Heading3, "Зарегистрированные правопреемники");
        htmlWriter->Begin(HtmlElement::Table);
        Array<Assign> assigns = assignRegistry->GetAllAssigns();
        IndexT assignIndex;
        for (assignIndex = 0; assignIndex < assigns.Size(); assignIndex++)
        {
            htmlWriter->Begin(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableData);
                htmlWriter->AddAttr("href", "/inout?ls=" + assigns[assignIndex].GetName() + ":");
                htmlWriter->Element(HtmlElement::Anchor, assigns[assignIndex].GetName());
            htmlWriter->End(HtmlElement::TableData);
            htmlWriter->Element(HtmlElement::TableData, assigns[assignIndex].GetPath());
            htmlWriter->Element(HtmlElement::TableData, assignRegistry->ResolveAssigns(assigns[assignIndex].GetPath()).AsString());
            htmlWriter->End(HtmlElement::TableRow);
        }
        htmlWriter->End(HtmlElement::Table);

        // показать смонтированные архивы
        htmlWriter->Element(HtmlElement::Heading3, "Установленные архивы");
        Array<Ptr<Archive> > archives = ArchiveFileSystem::Instance()->GetMountedArchives();
        if (archives.Size() > 0)
        {
            htmlWriter->Begin(HtmlElement::UnorderedList);
            IndexT arcIndex;
            for (arcIndex = 0; arcIndex < archives.Size(); arcIndex++)
            {
                htmlWriter->Element(HtmlElement::ListItem, archives[arcIndex]->GetURI().AsString());
            }
            htmlWriter->End(HtmlElement::UnorderedList);
        }
        else
        {
            htmlWriter->Text("[НИКТО]");
        }

        // отображать прикрепленные обработчики консоли
        htmlWriter->Element(HtmlElement::Heading3, "Прикрепленные обработчики консоли");
        htmlWriter->Begin(HtmlElement::UnorderedList);
        Array<Ptr<ConsoleHandler> > handlers = Console::Instance()->GetHandlers();
        IndexT conHandlerIndex;
        for (conHandlerIndex = 0; conHandlerIndex < handlers.Size(); conHandlerIndex++)
        {
            htmlWriter->Element(HtmlElement::ListItem, handlers[conHandlerIndex]->GetClassName());
        }
        htmlWriter->End(HtmlElement::UnorderedList);
        htmlWriter->Close();
        request->SetStatus(HttpStatus::OK);
    }
    else
    {
        request->SetStatus(HttpStatus::InternalServerError);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
IoPageHandler::ListDirectory(const Util::String& path, const Ptr<HttpRequest>& request)
{
    Ptr<HtmlPageWriter> htmlWriter = HtmlPageWriter::Create();
    htmlWriter->SetStream(request->GetResponseContentStream());
    htmlWriter->SetTitle("Список каталогов Nebula");
    if (htmlWriter->Open())
    {
        IoServer* ioServer = IoServer::Instance();        
        AssignRegistry* assignRegistry = AssignRegistry::Instance();
        htmlWriter->Element(HtmlElement::Heading1, "Листинг каталога");
        htmlWriter->AddAttr("href", "/index.html");
        htmlWriter->Element(HtmlElement::Anchor, "Home");
        htmlWriter->LineBreak();
        htmlWriter->AddAttr("href", "/inout");
        htmlWriter->Element(HtmlElement::Anchor, "Главная страница подсистемы ввода-вывода");
        htmlWriter->LineBreak();
        if (path[path.Length() - 1] != ':')
        {
            // add parent directory navigation entry (if not top-level)
            String str = "/inout?ls=" + path.ExtractDirName();
            str.TrimRight("/");
            htmlWriter->AddAttr("href", str);
            htmlWriter->Element(HtmlElement::Anchor, "Родительский каталог");
        }
        htmlWriter->LineBreak();
        
        htmlWriter->Element(HtmlElement::Heading3, path + "    (" + assignRegistry->ResolveAssigns(path).AsString() + ")");
        Array<String> dirs = ioServer->ListDirectories(path, "*");
        Array<String> files = ioServer->ListFiles(path, "*");

        htmlWriter->Begin(HtmlElement::UnorderedList);
        IndexT i;
        for (i = 0; i < dirs.Size(); i++)
        {
            htmlWriter->Begin(HtmlElement::ListItem);
            if (path[path.Length() - 1] == ':')
            {
                // не добавляйте косую черту сразу после присваивания
                htmlWriter->AddAttr("href", "/inout?ls=" + path + dirs[i]);
            }
            else
            {                   
                htmlWriter->AddAttr("href", "/inout?ls=" + path + "/" + dirs[i]);
            }
            htmlWriter->Element(HtmlElement::Anchor, dirs[i]);
            htmlWriter->End(HtmlElement::ListItem);
        }
        for (i = 0; i < files.Size(); i++)
        {
            htmlWriter->Element(HtmlElement::ListItem, files[i]);
        }
        htmlWriter->End(HtmlElement::UnorderedList);

        htmlWriter->Close();
        request->SetStatus(HttpStatus::OK);
    }
    else
    {
        request->SetStatus(HttpStatus::InternalServerError);
    }
}

} // Отладка пространства имен
