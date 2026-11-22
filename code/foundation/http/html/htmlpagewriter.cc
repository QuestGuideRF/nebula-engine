//------------------------------------------------------------------------------
//  htmlpagewriter.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/html/htmlpagewriter.h"

namespace Http
{
__ImplementClass(Http::HtmlPageWriter, 'HMPW', IO::StreamWriter);

using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
HtmlPageWriter::HtmlPageWriter()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
HtmlPageWriter::~HtmlPageWriter()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool
HtmlPageWriter::Open()
{
    if (StreamWriter::Open())
    {
        // создайте XmlWriter и прикрепите его к нашему потоку
        this->xmlWriter = XmlWriter::Create();
        this->xmlWriter->SetStream(this->stream);
        this->xmlWriter->Open();
        
        this->Begin(HtmlElement::Html);
        this->Begin(HtmlElement::Head);
        this->Begin(HtmlElement::Title);
        this->Text(this->title);
        this->End(HtmlElement::Title);
        if (!this->style.IsEmpty())
        {
            this->Begin(HtmlElement::Style);
            this->Raw(this->style);
            this->End(HtmlElement::Style);
        }
        this->End(HtmlElement::Head);

        this->AddAttr("style", "семейство шрифтов: 'Пользовательский интерфейс Segoe'");
        this->Begin(HtmlElement::Body);


        return true;
    }
    else
    {
        return false;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
HtmlPageWriter::Close()
{
    n_assert(this->IsOpen());

    // закрыть HTML-страницу
    this->End(HtmlElement::Body);
    this->End(HtmlElement::Html);
    n_assert(elementStack.IsEmpty());

    // закрыть модуль записи XML
    this->xmlWriter->Close();
    this->xmlWriter = nullptr;

    // установите тип MIME в нашем потоке
    this->stream->SetMediaType(MediaType("text/html"));    

    // вызвать родительский класс
    StreamWriter::Close();
}

//------------------------------------------------------------------------------
/**
    Adds an attribute for the next HTML element. Any number of attributes
    can be defined before calling Begin(). The attribute stack will be cleared
    as Begin() returns.
*/
void
HtmlPageWriter::AddAttr(const String& name, const String& value)
{
    n_assert(this->IsOpen());
    n_assert(name.IsValid());
    n_assert(value.IsValid());
    this->attrs.Append(KeyValuePair<String,String>(name, value));
}

//------------------------------------------------------------------------------
/**
    Начните с общего HTML-элемента. Элементы могут быть вложенными.
*/
void
HtmlPageWriter::Begin(HtmlElement::Code element)
{
    n_assert(this->IsOpen());
    this->elementStack.Push(element);
    this->xmlWriter->BeginNode(HtmlElement::ToHtml(element));
    IndexT i;
    for (i = 0; i < this->attrs.Size(); i++)
    {
        this->xmlWriter->SetString(this->attrs[i].Key(), this->attrs[i].Value());
    }
    this->attrs.Clear();
}

//------------------------------------------------------------------------------
/**
    Завершите общий HTML-элемент. Элемент должен соответствовать последнему вызову Begin()!
*/
void
HtmlPageWriter::End(HtmlElement::Code element)
{
    n_assert(this->IsOpen());
    n_assert(!this->elementStack.IsEmpty());
    n_assert(this->elementStack.Peek() == element);
    this->elementStack.Pop();
    this->xmlWriter->EndNode();
}

//------------------------------------------------------------------------------
/**
*/
void
HtmlPageWriter::LineBreak()
{
    n_assert(this->IsOpen());
    this->xmlWriter->BeginNode("br");
    this->xmlWriter->EndNode();
}

//------------------------------------------------------------------------------
/**
*/
void
HtmlPageWriter::HorizontalRule()
{
    n_assert(this->IsOpen());
    this->xmlWriter->BeginNode("hr");
    this->xmlWriter->EndNode();
}

//------------------------------------------------------------------------------
/**
*/
void
HtmlPageWriter::Text(const Util::String& str)
{
    this->xmlWriter->WriteContent(str);
}

//------------------------------------------------------------------------------
/**
*/
void 
HtmlPageWriter::Raw( const Util::String& r )
{
    this->xmlWriter->WriteRaw(r);
}

//------------------------------------------------------------------------------
/**
*/
void
HtmlPageWriter::Element(HtmlElement::Code e, const Util::String& text)
{
    this->Begin(e);
    this->Text(text);
    this->End(e);
}

//------------------------------------------------------------------------------
/**
*/
void
HtmlPageWriter::TableRow2(const String& col0, const String& col1)
{
    this->Begin(HtmlElement::TableRow);
    this->Element(HtmlElement::TableData, col0);
    this->Element(HtmlElement::TableData, col1);
    this->End(HtmlElement::TableRow);   
}

} // пространство имен HTTP