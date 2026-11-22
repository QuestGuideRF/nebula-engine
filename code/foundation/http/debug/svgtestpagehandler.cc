//------------------------------------------------------------------------------
//  svgtestpagehandler.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/debug/svgtestpagehandler.h"
#include "http/svg/svgpagewriter.h"
#include "http/svg/svglinechartwriter.h"

namespace Debug
{
__ImplementClass(Debug::SvgTestPageHandler, 'SVTP', Http::HttpRequestHandler);

using namespace Http;
using namespace Util;
using namespace Math;

//------------------------------------------------------------------------------
/**
*/
SvgTestPageHandler::SvgTestPageHandler()
{
    this->SetName("Тест рендеринга SVG");
    this->SetDesc("протестировать функциональность рендеринга SVG");
    this->SetRootLocation("svgtest");
}

//------------------------------------------------------------------------------
/**
*/
bool
SvgTestPageHandler::TestShapeRendering(const Ptr<HttpRequest>& request)
{
    // настроить средство записи страниц SVG
    Ptr<SvgPageWriter> svgWriter = SvgPageWriter::Create();
    svgWriter->SetStream(request->GetResponseContentStream());
    svgWriter->SetCanvasDimensions(1000, 1000);
    svgWriter->SetUnitDimensions(1000, 1000);
    if (svgWriter->Open())
    {
        // сделать контур
        svgWriter->BeginPaintGroup("none", "black", 1);
            svgWriter->Rect(0, 0, 1000, 1000);
        svgWriter->EndGroup();

        // рендеринг прямоугольников
        svgWriter->BeginPaintGroup("red", "black", 1);
            svgWriter->Rect(100, 100, 150, 150);
            svgWriter->Rect(300, 300, 150, 150);
        svgWriter->EndGroup();
        svgWriter->BeginPaintGroup("black", "red", 1);
            svgWriter->Rect(300, 100, 150, 150);
            svgWriter->Rect(100, 300, 150, 150);
        svgWriter->EndGroup();

        // визуализировать круги
        svgWriter->BeginPaintGroup("red", "black", 1);
            svgWriter->Circle(700, 700, 50);
            svgWriter->Circle(800, 800, 50);
        svgWriter->EndGroup();
        svgWriter->BeginPaintGroup("black", "red", 1);
            svgWriter->Circle(700, 800, 50);
            svgWriter->Circle(800, 700, 50);
        svgWriter->EndGroup();

        // визуализировать полилинию
        svgWriter->BeginPaintGroup("none", "green", 1);
            Array<vec2> points;
            int x;
            for (x = 100; x < 400; x += 2)
            {
                points.Append(vec2(float(x), 750.0f + Math::rand(-250, 250)));
            }
            svgWriter->PolyLine(points);
        svgWriter->EndGroup();

        // визуализировать текст
        svgWriter->BeginTextGroup(30, "black");
            svgWriter->Text(600, 250, "Привет, мир");
        svgWriter->EndGroup();

        // done...
        svgWriter->Close();
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
bool
SvgTestPageHandler::TestLineChartRendering(const Ptr<HttpRequest>& request)
{
    // настроить средство записи линейных диаграмм SVG
    Ptr<SvgLineChartWriter> writer = SvgLineChartWriter::Create();
    writer->SetStream(request->GetResponseContentStream());
    writer->SetCanvasDimensions(1024, 256);
    if (writer->Open())
    {
        writer->SetupXAxis("frame", "frames", -512, 0);
        writer->SetupYAxis("fps", "1/sec", 0.0f, 60.0f);
        Array<float> values(512, 0);
        IndexT i;
        for (i = 0; i < 512; i++)
        {
            values.Append(Math::rand(10.0f, 45.0f));
        }
        writer->AddTrack("fps", "green", values);
        writer->Draw();
        writer->Close();
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
SvgTestPageHandler::HandleRequest(const Ptr<HttpRequest>& request)
{
    if (this->TestLineChartRendering(request))
    {
        request->SetStatus(HttpStatus::OK);
    }
    else
    {
        request->SetStatus(HttpStatus::InternalServerError);
    }
}

} // Отладка пространства имен