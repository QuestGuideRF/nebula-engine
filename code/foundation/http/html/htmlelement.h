#pragma once
#ifndef HTTP_HTMLELEMENT_H
#define HTTP_HTMLELEMENT_H
#include "core/types.h"
#include "util/string.h"
namespace Http
{
class HtmlElement
{
public:
    enum Code
    {
        Html,
        Head,
        Title,
        Body,
        Heading1,
        Heading2,
        Heading3,
        Heading4,
        Heading5,
        Heading6,
        Paragraph,
        UnorderedList,
        OrderedList,
        ListItem,
        DefinitionList,
        DefinitionListTerm,
        DefinitionListDefinition,
        BlockQuote,
        PreFormatted,
        Bold,
        Italics,
        Teletyper,
        Underscore,
        Strike,
        Big,
        Small,
        Sup,
        Sub,
        Table,
        TableRow,
        TableHeader,
        TableData,
        Anchor,
        Image,
        Object,
        Form,
        Input,
        Div,
        Span,
        Style,
        Font,
        Script,
        InvalidHtmlElement,
    };
    static Util::String ToHtml(Code c);
};
}
#endif