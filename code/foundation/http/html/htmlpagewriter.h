#pragma once
#ifndef HTTP_HTMLPAGEWRITER_H
#define HTTP_HTMLPAGEWRITER_H
#include "io/streamwriter.h"
#include "http/html/htmlelement.h"
#include "io/xmlwriter.h"
#include "util/array.h"
#include "util/stack.h"
namespace Http
{
class HtmlPageWriter : public IO::StreamWriter
{
    __DeclareClass(HtmlPageWriter);
public:
    HtmlPageWriter();
    virtual ~HtmlPageWriter();
    void SetTitle(const Util::String& title);
    void SetStyle(const Util::String& style);
    virtual bool Open();
    virtual void Close();
    void AddAttr(const Util::String& name, const Util::String& value);
    void Begin(HtmlElement::Code e);
    void End(HtmlElement::Code e);
    void Element(HtmlElement::Code e, const Util::String& text);
    void LineBreak();
    void HorizontalRule();
    void Text(const Util::String& t);
    void Raw(const Util::String& r);
    void TableRow2(const Util::String& col0, const Util::String& col1);
private:
    Util::String title;
    Util::String style;
    Ptr<IO::XmlWriter> xmlWriter;
    Util::Array<Util::KeyValuePair<Util::String,Util::String> > attrs;
    Util::Stack<HtmlElement::Code> elementStack;
};
inline void
HtmlPageWriter::SetTitle(const Util::String& t)
{
    this->title = t;
}
inline void
HtmlPageWriter::SetStyle(const Util::String& s)
{
    this->style = s;
}
}
#endif