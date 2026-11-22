#pragma once
#ifndef HTTP_SVGPAGEWRITER_H
#define HTTP_SVGPAGEWRITER_H
#include "io/streamwriter.h"
#include "io/xmlwriter.h"
#include "math/vec2.h"
namespace Http
{
class SvgPageWriter : public IO::StreamWriter
{
    __DeclareClass(SvgPageWriter);
public:
    SvgPageWriter();
    virtual ~SvgPageWriter();
    void SetCanvasDimensions(SizeT w, SizeT h);
    void SetUnitDimensions(SizeT w, SizeT h);
    virtual bool Open();
    virtual void Close();
    void WriteContent(const Util::String& text);
    void BeginNode(const Util::String& nodeName);
    void EndNode();
    void SetString(const Util::String& name, const Util::String& value);
    void BeginTransformGroup(const Math::vec2& translate, float rotate, const Math::vec2& scale);
    void BeginPaintGroup(const Util::String& fillColor, const Util::String& strokeColor, int strokeWidth);
    void BeginTextGroup(int fontSize, const Util::String& textColor);
    void EndGroup();
    void Rect(float x, float y, float w, float h);
    void Circle(float x, float y, float r);
    void Ellipse(float x, float y, float rx, float ry);
    void Line(float x1, float y1, float x2, float y2);
    void PolyLine(const Util::Array<Math::vec2>& points);
    void Polygon(const Util::Array<Math::vec2>& points);
    void Text(float x, float y, const Util::String& text);
protected:
    Ptr<IO::XmlWriter> xmlWriter;
    SizeT canvasWidth;
    SizeT canvasHeight;
    SizeT unitWidth;
    SizeT unitHeight;
};
inline void
SvgPageWriter::SetCanvasDimensions(SizeT w, SizeT h)
{
    this->canvasWidth = w;
    this->canvasHeight = h;
}
inline void
SvgPageWriter::SetUnitDimensions(SizeT w, SizeT h)
{
    this->unitWidth = w;
    this->unitHeight = h;
}
}
#endif