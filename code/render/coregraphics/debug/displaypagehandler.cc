//------------------------------------------------------------------------------
//  displaypagehandler.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coregraphics/debug/displaypagehandler.h"
#include "coregraphics/displaydevice.h"
#include "http/html/htmlpagewriter.h"
#include "coregraphics/graphicsdevice.h"

namespace Debug
{
__ImplementClass(Debug::DisplayPageHandler, 'DDPH', Http::HttpRequestHandler);

using namespace CoreGraphics;
using namespace Http;
using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
DisplayPageHandler::DisplayPageHandler()
{
    this->SetName("Display");
    this->SetDesc("show Отображение отладочной информации");
    this->SetRootLocation("display");
}

//------------------------------------------------------------------------------
/**
*/
void
DisplayPageHandler::HandleRequest(const Ptr<Http::HttpRequest>& request)
{
    n_assert(HttpMethod::Get == request->GetMethod());
    DisplayDevice* disp = DisplayDevice::Instance();

    // проверьте, запрашивается ли скриншот
    Dictionary<String,String> query = request->GetURI().ParseQuery();
    if (String::MatchPattern(request->GetURI().LocalPath(), "*/скриншот*"))
    {
        String fmt = query.Contains("fmt") ? query["fmt"] : "png";
        request->SetStatus(this->WriteScreenshot(fmt, request->GetResponseContentStream()));
        return;
    }

    // настроить средство записи HTML-страниц
    Ptr<HtmlPageWriter> htmlWriter = HtmlPageWriter::Create();
    htmlWriter->SetStream(request->GetResponseContentStream());
    htmlWriter->SetTitle("Отображение информации о туманности");
    if (htmlWriter->Open())
    {
        String str;
        htmlWriter->Element(HtmlElement::Heading1, "Display");
        htmlWriter->AddAttr("href", "/index.html");
        htmlWriter->Element(HtmlElement::Anchor, "Home");
        
        // напиши скриншот
        htmlWriter->Element(HtmlElement::Heading3, "Screenshot");
        htmlWriter->AddAttr("src", "/display/screenshot");
        htmlWriter->Element(HtmlElement::Image, "");

        // записать текущие свойства отображения
        htmlWriter->Element(HtmlElement::Heading3, "Свойства отображения");
        htmlWriter->Begin(HtmlElement::Table);        
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Дисплей открыт:");
                htmlWriter->Element(HtmlElement::TableData, disp->IsOpen() ? "Yes" : "No");
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Adapter:");
                htmlWriter->Element(HtmlElement::TableData, Adapter::ToString(disp->GetAdapter()));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Вертикальная синхронизация:");
                htmlWriter->Element(HtmlElement::TableData, disp->IsVerticalSyncEnabled() ? "Yes" : "No");
            htmlWriter->End(HtmlElement::TableRow);
        htmlWriter->End(HtmlElement::Table);
        
        const Util::Array<WindowId>& windows = disp->GetWindows();
        htmlWriter->Element(HtmlElement::Heading3, "Windows");        
        htmlWriter->Begin(HtmlElement::Table);
        IndexT i;
        for (i = 0; i < windows.Size(); i++)
        {
            const WindowId window = windows[i];
            const DisplayMode& displayMode = WindowGetDisplayMode(window);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Size:");
                str.Format("%d x %d", displayMode.GetWidth(), displayMode.GetHeight());
                htmlWriter->Element(HtmlElement::TableData, str);
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Формат пикселей:");
                htmlWriter->Element(HtmlElement::TableData, PixelFormat::ToString(displayMode.GetPixelFormat()));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Режим АА:");
                htmlWriter->Element(HtmlElement::TableData, AntiAliasQuality::ToString(WindowGetAAQuality(window)));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Fullscreen:");
                htmlWriter->Element(HtmlElement::TableData, WindowIsFullscreen(window) ? "Yes" : "No");
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Resizable:");
                htmlWriter->Element(HtmlElement::TableData, WindowIsResizable(window) ? "Yes" : "No");
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Decorated:");
                htmlWriter->Element(HtmlElement::TableData, WindowIsDecorated(window) ? "Yes" : "No");
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Название значка:");
                htmlWriter->Element(HtmlElement::TableData, WindowGetIcon(window).Value());
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Название окна:");
                htmlWriter->Element(HtmlElement::TableData, WindowGetTitle(window).Value());
            htmlWriter->End(HtmlElement::TableRow);
        }           
        htmlWriter->End(HtmlElement::Table);        

        this->WriteAdapterInfo(Adapter::Primary, htmlWriter);
     //   this->WriteAdapterInfo(Adapter::Secondary, htmlWriter);

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
    Запишите информацию об адаптере на веб-страницу.
*/
void
DisplayPageHandler::WriteAdapterInfo(Adapter::Code adapter, const Ptr<HtmlPageWriter>& htmlWriter)
{
    DisplayDevice* disp = DisplayDevice::Instance();
    if (disp->AdapterExists(adapter))
    {
        String str;
        htmlWriter->Element(HtmlElement::Heading3, "Adapter: " + Adapter::ToString(adapter));
        AdapterInfo info = disp->GetAdapterInfo(adapter);
        htmlWriter->Element(HtmlElement::Heading4, "Информация об адаптере:");
        htmlWriter->Begin(HtmlElement::Table);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Текущий режим:");
                DisplayMode curMode = disp->GetCurrentAdapterDisplayMode(adapter);
                str.Format("%d x %d %s", curMode.GetWidth(), curMode.GetHeight(), PixelFormat::ToString(curMode.GetPixelFormat()).AsCharPtr());
                htmlWriter->Element(HtmlElement::TableData, str);
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Имя водителя:");
                htmlWriter->Element(HtmlElement::TableData, info.GetDriverName());
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Description:");
                htmlWriter->Element(HtmlElement::TableData, info.GetDescription());
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Имя устройства:");
                htmlWriter->Element(HtmlElement::TableData, info.GetDeviceName());
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Версия драйвера (высокая часть):");
                htmlWriter->Element(HtmlElement::TableData, String::FromInt(info.GetDriverVersionHighPart()));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Версия драйвера (нижняя часть):");
                htmlWriter->Element(HtmlElement::TableData, String::FromInt(info.GetDriverVersionLowPart()));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Идентификатор поставщика:");
                htmlWriter->Element(HtmlElement::TableData, String::FromInt(info.GetVendorId()));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Идентификатор устройства:");
                htmlWriter->Element(HtmlElement::TableData, String::FromInt(info.GetDeviceId()));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Идентификатор подсистемы:");
                htmlWriter->Element(HtmlElement::TableData, String::FromInt(info.GetSubSystemId()));
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Revision:");
                htmlWriter->Element(HtmlElement::TableData, String::FromInt(info.GetRevision()));
            htmlWriter->End(HtmlElement::TableRow);
        htmlWriter->End(HtmlElement::Table);
        
        // перечислить режимы отображения
        htmlWriter->Element(HtmlElement::Heading4, "Режимы отображения адаптера:");
        htmlWriter->Begin(HtmlElement::UnorderedList);
        uint pixelFormat;
        for (pixelFormat = 0; pixelFormat < PixelFormat::NumPixelFormats; pixelFormat++)
        {
            Array<DisplayMode> modes = disp->GetAvailableDisplayModes(adapter, PixelFormat::Code(pixelFormat));
            if (modes.Size() > 0)
            {
                htmlWriter->Element(HtmlElement::ListItem, PixelFormat::ToString(PixelFormat::Code(pixelFormat)));
                htmlWriter->Begin(HtmlElement::UnorderedList);
                IndexT modeIndex;
                for (modeIndex = 0; modeIndex < modes.Size(); modeIndex++)
                {
                    str.Format("%d x %d @ %d Гц", modes[modeIndex].GetWidth(), modes[modeIndex].GetHeight(), modes[modeIndex].GetRefreshRate());
                    htmlWriter->Element(HtmlElement::ListItem, str);
                }
                htmlWriter->End(HtmlElement::UnorderedList);
            }
        }
        htmlWriter->End(HtmlElement::UnorderedList);
    }            
}

//------------------------------------------------------------------------------
/**
*/
HttpStatus::Code
DisplayPageHandler::WriteScreenshot(const String& fileFormat, const Ptr<IO::Stream>& responseContentStream)
{
    ImageFileFormat::Code fmt = ImageFileFormat::FromString(fileFormat);
    if (ImageFileFormat::InvalidImageFileFormat == fmt)
    {
        fmt = ImageFileFormat::PNG;
    }
    CoreGraphics::SaveScreenshot(fmt, responseContentStream);
    return HttpStatus::OK;
}

} // Отладка пространства имен
