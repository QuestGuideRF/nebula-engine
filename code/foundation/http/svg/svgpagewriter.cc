//------------------------------------------------------------------------------
//  svgpagewriter.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/svg/svgpagewriter.h"

namespace Http
{
__ImplementClass(Http::SvgPageWriter, 'SVGW', IO::StreamWriter);

using namespace Util;
using namespace IO;
using namespace Math;

//------------------------------------------------------------------------------
/**
*/
SvgPageWriter::SvgPageWriter() :
    canvasWidth(100),
    canvasHeight(100),
    unitWidth(100),
    unitHeight(100)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
SvgPageWriter::~SvgPageWriter()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool
SvgPageWriter::Open()
{
    if (StreamWriter::Open())
    {
        // создайте XmlWriter и прикрепите его к нашему потоку
        this->xmlWriter = XmlWriter::Create();
        this->xmlWriter->SetStream(this->stream);
        this->xmlWriter->Open();

        // написать корневой элемент SVG
        this->xmlWriter->BeginNode("svg");
        this->xmlWriter->SetString("xmlns", "http://www.w3.org/2000/svg");
        this->xmlWriter->SetInt("width", this->canvasWidth);
        this->xmlWriter->SetInt("height", this->canvasHeight);
        String viewBox;
        viewBox.Format("0 0 %d %d", this->unitWidth, this->unitHeight);
        this->xmlWriter->SetString("viewBox", viewBox);
        this->xmlWriter->SetString("preserveAspectRatio", "none");

        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
SvgPageWriter::Close()
{
    n_assert(this->IsOpen());

    // закрыть страницу SVG
    this->xmlWriter->EndNode();

    // закрыть модуль записи XML
    this->xmlWriter->Close();
    this->xmlWriter = nullptr;

    // установите тип MIME в нашем потоке
    this->stream->SetMediaType(MediaType("изображение/svg+xml"));

    // вызвать родительский класс
    StreamWriter::Close();
}

//------------------------------------------------------------------------------
/**
*/
void
SvgPageWriter::BeginNode(const Util::String& nodeName)
{
    this->xmlWriter->BeginNode(nodeName);
}

//------------------------------------------------------------------------------
/**
*/
void
SvgPageWriter::EndNode()
{
    this->xmlWriter->EndNode();
}

//------------------------------------------------------------------------------
/**
*/
void
SvgPageWriter::SetString(const Util::String& name, const Util::String& value)
{
    this->xmlWriter->SetString(name, value);
}

//------------------------------------------------------------------------------
/**
*/
void
SvgPageWriter::BeginTransformGroup(const vec2& translate, float rotate, const vec2& scale)
{
    // отформатировать строку атрибута
    String str;
    str.Format("транслировать(%.3f,%.3f) вращать(%.3f) масштабировать(%.3f,%.3f)", 
        translate.x, translate.y,
        rotate,
        scale.x, scale.y);

    // написать XML-элемент
    this->xmlWriter->BeginNode("g");
    this->xmlWriter->SetString("transform", str);
}

//------------------------------------------------------------------------------
/**
*/
void
SvgPageWriter::BeginPaintGroup(const Util::String& fillColor, const Util::String& strokeColor, int strokeWidth)
{
    n_assert(fillColor.IsValid());
    n_assert(strokeColor.IsValid());

    this->xmlWriter->BeginNode("g");
    this->xmlWriter->SetString("fill", fillColor);
    this->xmlWriter->SetString("stroke", strokeColor);
    this->xmlWriter->SetInt("stroke-width", strokeWidth);
}

//------------------------------------------------------------------------------
/**
*/
void
SvgPageWriter::BeginTextGroup(int fontSize, const Util::String& textColor)
{
    this->xmlWriter->BeginNode("g");
    this->xmlWriter->SetInt("font-size", fontSize);
    this->xmlWriter->SetString("fill", textColor);
}

//------------------------------------------------------------------------------
/**
*/
void
SvgPageWriter::EndGroup()
{
    this->xmlWriter->EndNode();
}

//------------------------------------------------------------------------------
/**
*/
void
SvgPageWriter::Rect(float x, float y, float w, float h)
{
    this->xmlWriter->BeginNode("rect");
        this->xmlWriter->SetFloat("x", x);
        this->xmlWriter->SetFloat("y", y);
        this->xmlWriter->SetFloat("width", w);
        this->xmlWriter->SetFloat("height", h);
    this->xmlWriter->EndNode();
}

//------------------------------------------------------------------------------
/**
*/
void
SvgPageWriter::Circle(float x, float y, float r)
{
    this->xmlWriter->BeginNode("circle");
        this->xmlWriter->SetFloat("cx", x);
        this->xmlWriter->SetFloat("cy", y);
        this->xmlWriter->SetFloat("r", r);
    this->xmlWriter->EndNode();
}

//------------------------------------------------------------------------------
/**
*/
void
SvgPageWriter::Ellipse(float x, float y, float rx, float ry)
{
    this->xmlWriter->BeginNode("ellipse");
        this->xmlWriter->SetFloat("cx", x);
        this->xmlWriter->SetFloat("cy", y);
        this->xmlWriter->SetFloat("rx", rx);
        this->xmlWriter->SetFloat("ry", ry);
    this->xmlWriter->EndNode();
}

//------------------------------------------------------------------------------
/**
*/
void
SvgPageWriter::Line(float x1, float y1, float x2, float y2)
{
    this->xmlWriter->BeginNode("line");
        this->xmlWriter->SetFloat("x1", x1);
        this->xmlWriter->SetFloat("y1", y1);
        this->xmlWriter->SetFloat("x2", x2);
        this->xmlWriter->SetFloat("y2", y2);
    this->xmlWriter->EndNode();
}

//------------------------------------------------------------------------------
/**
    Примечание: из-за ограничения длины XML-атрибутов в TinyXML.
    размер массива для одного поли-примитива должен составлять максимум около 200.
*/
void
SvgPageWriter::PolyLine(const Array<vec2>& points)
{
    // построить строку координат
    String str;
    str.Reserve(points.Size() * 20);
    IndexT i;
    for (i = 0; i < points.Size(); i++)
    {
        str.AppendFloat(points[i].x);
        str.Append(",");
        str.AppendFloat(points[i].y);
        str.Append(" ");
    }

    // написать XML-элемент
    this->xmlWriter->BeginNode("polyline");
        this->xmlWriter->SetString("points", str);
    this->xmlWriter->EndNode();
}

//------------------------------------------------------------------------------
/**
    Примечание: из-за ограничения длины XML-атрибутов в TinyXML.
    размер массива для одного поли-примитива должен составлять максимум около 200.
*/
void
SvgPageWriter::Polygon(const Array<vec2>& points)
{
    // построить строку координат
    String str;
    str.Reserve(points.Size() * 20);
    IndexT i;
    for (i = 0; i < points.Size(); i++)
    {
        str.AppendFloat(points[i].x);
        str.Append(",");
        str.AppendFloat(points[i].y);
        str.Append(" ");
    }

    // написать XML-элемент
    this->xmlWriter->BeginNode("polygon");
        this->xmlWriter->SetString("points", str);
    this->xmlWriter->EndNode();
}

//------------------------------------------------------------------------------
/**
*/
void
SvgPageWriter::Text(float x, float y, const String& text)
{
    this->xmlWriter->BeginNode("text");
        this->xmlWriter->SetFloat("x", x);
        this->xmlWriter->SetFloat("y", y);
        this->xmlWriter->WriteContent(text);
    this->xmlWriter->EndNode();
}

//------------------------------------------------------------------------------
/**
*/
void
SvgPageWriter::WriteContent(const Util::String& text)
{
    this->xmlWriter->WriteContent(text);
}
} // пространство имен HTTP