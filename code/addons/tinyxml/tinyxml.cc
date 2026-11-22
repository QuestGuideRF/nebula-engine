/*
www.sourceforge.net/projects/tinyxml
Original code (2.0 and earlier )copyright (c) 2000-2002 Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/
#include "foundation/stdneb.h"
#include <ctype.h>
#include "tinyxml.h"

#ifdef TIXML_USE_STL
#include <sstream>
#include <iostream>
#endif


bool TiXmlBase::condenseWhiteSpace = true;

void TiXmlBase::PutString( const TIXML_STRING& str, TIXML_OSTREAM* stream )
{
    TIXML_STRING buffer;
    PutString( str, &buffer );
    (*stream) << buffer;
}

void TiXmlBase::PutString( const TIXML_STRING& str, TIXML_STRING* outString )
{
    int i=0;

    while( i<(int)str.length() )
    {
        unsigned char c = (unsigned char) str[i];

        if (    c == '&' 
             && i < ( (int)str.length() - 2 )
             && str[i+1] == '#'
             && str[i+2] == 'x' )
        {
            // Ссылка на шестнадцатеричный символ.
            // Пройти без изменений.
            // &#xA9;   -- например, символ авторского права.
            //
            // -1 — это исправление ошибки от Роба Лаво. Он держит
            // переполнение не произойдет, если нет ';'.
            // На самом деле есть два способа выйти из этого цикла:
            // while fails (error case) and break (semicolon found).
            // However, there is no mechanism (currently) for
            // эта функция возвращает ошибку.
            while ( i<(int)str.length()-1 )
            {
                outString->append( str.c_str() + i, 1 );
                ++i;
                if ( str[i] == ';' )
                    break;
            }
        }
        else if ( c == '&' )
        {
            outString->append( entity[0].str, entity[0].strLength );
            ++i;
        }
        else if ( c == '<' )
        {
            outString->append( entity[1].str, entity[1].strLength );
            ++i;
        }
        else if ( c == '>' )
        {
            outString->append( entity[2].str, entity[2].strLength );
            ++i;
        }
        else if ( c == '\"' )
        {
            outString->append( entity[3].str, entity[3].strLength );
            ++i;
        }
        else if ( c == '\'' )
        {
            outString->append( entity[4].str, entity[4].strLength );
            ++i;
        }
        else if ( c < 32 )
        {
            // Легкий переход к небуквенным/цифровым/символьным значениям
            // Ниже 32 – символично.
            char buf[ 32 ];
            
            #if defined(TIXML_SNPRINTF)     
                TIXML_SNPRINTF( buf, sizeof(buf), "&#x%02X;", (unsigned) ( c & 0xff ) );
            #else
                sprintf( buf, "&#x%02X;", (unsigned) ( c & 0xff ) );
            #endif      

            //*ME: предупреждение C4267: преобразовать «size_t» в «int»
            //*Я: Int-Cast, чтобы порадовать компилятор...
            outString->append( buf, (int)strlen( buf ) );
            ++i;
        }
        else
        {
            //charrealc = (char) c;
            //outString->append(&realc, 1);
            *outString += (char) c; // несколько более эффективный вызов функции.
            ++i;
        }
    }
}


// <-- Странный класс для исправления ошибок. Найдите STL_STRING_BUG
TiXmlBase::StringToBuffer::StringToBuffer( const TIXML_STRING& str )
{
    buffer = new char[ str.length()+1 ];
    if ( buffer )
    {
        strcpy( buffer, str.c_str() );
    }
}


TiXmlBase::StringToBuffer::~StringToBuffer()
{
    delete [] buffer;
}
// Завершить странное исправление ошибки. -->


TiXmlNode::TiXmlNode( NodeType _type ) : TiXmlBase()
{
    parent = 0;
    type = _type;
    firstChild = 0;
    lastChild = 0;
    prev = 0;
    next = 0;
}


TiXmlNode::~TiXmlNode()
{
    TiXmlNode* node = firstChild;
    TiXmlNode* temp = 0;

    while ( node )
    {
        temp = node;
        node = node->next;
        delete temp;
    }   
}


void TiXmlNode::CopyTo( TiXmlNode* target ) const
{
    target->SetValue (value.c_str() );
    target->userData = userData; 
}


void TiXmlNode::Clear()
{
    TiXmlNode* node = firstChild;
    TiXmlNode* temp = 0;

    while ( node )
    {
        temp = node;
        node = node->next;
        delete temp;
    }   

    firstChild = 0;
    lastChild = 0;
}


TiXmlNode* TiXmlNode::LinkEndChild( TiXmlNode* node )
{
    assert( node->parent == 0 || node->parent == this );
    assert( node->GetDocument() == 0 || node->GetDocument() == this->GetDocument() );

    node->parent = this;

    node->prev = lastChild;
    node->next = 0;

    if ( lastChild )
        lastChild->next = node;
    else
        firstChild = node;          // это был пустой список.

    lastChild = node;
    return node;
}


TiXmlNode* TiXmlNode::InsertEndChild( const TiXmlNode& addThis )
{
    TiXmlNode* node = addThis.Clone();
    if ( !node )
        return 0;

    return LinkEndChild( node );
}


TiXmlNode* TiXmlNode::InsertBeforeChild( TiXmlNode* beforeThis, const TiXmlNode& addThis )
{   
    if ( !beforeThis || beforeThis->parent != this )
        return 0;

    TiXmlNode* node = addThis.Clone();
    if ( !node )
        return 0;
    node->parent = this;

    node->next = beforeThis;
    node->prev = beforeThis->prev;
    if ( beforeThis->prev )
    {
        beforeThis->prev->next = node;
    }
    else
    {
        assert( firstChild == beforeThis );
        firstChild = node;
    }
    beforeThis->prev = node;
    return node;
}


TiXmlNode* TiXmlNode::InsertAfterChild( TiXmlNode* afterThis, const TiXmlNode& addThis )
{
    if ( !afterThis || afterThis->parent != this )
        return 0;

    TiXmlNode* node = addThis.Clone();
    if ( !node )
        return 0;
    node->parent = this;

    node->prev = afterThis;
    node->next = afterThis->next;
    if ( afterThis->next )
    {
        afterThis->next->prev = node;
    }
    else
    {
        assert( lastChild == afterThis );
        lastChild = node;
    }
    afterThis->next = node;
    return node;
}


TiXmlNode* TiXmlNode::ReplaceChild( TiXmlNode* replaceThis, const TiXmlNode& withThis )
{
    if ( replaceThis->parent != this )
        return 0;

    TiXmlNode* node = withThis.Clone();
    if ( !node )
        return 0;

    node->next = replaceThis->next;
    node->prev = replaceThis->prev;

    if ( replaceThis->next )
        replaceThis->next->prev = node;
    else
        lastChild = node;

    if ( replaceThis->prev )
        replaceThis->prev->next = node;
    else
        firstChild = node;

    delete replaceThis;
    node->parent = this;
    return node;
}


bool TiXmlNode::RemoveChild( TiXmlNode* removeThis )
{
    if ( removeThis->parent != this )
    {   
        assert( 0 );
        return false;
    }

    if ( removeThis->next )
        removeThis->next->prev = removeThis->prev;
    else
        lastChild = removeThis->prev;

    if ( removeThis->prev )
        removeThis->prev->next = removeThis->next;
    else
        firstChild = removeThis->next;

    delete removeThis;
    return true;
}

const TiXmlNode* TiXmlNode::FirstChild( const char * _value ) const
{
    const TiXmlNode* node;
    for ( node = firstChild; node; node = node->next )
    {
        if ( strcmp( node->Value(), _value ) == 0 )
            return node;
    }
    return 0;
}


TiXmlNode* TiXmlNode::FirstChild( const char * _value )
{
    TiXmlNode* node;
    for ( node = firstChild; node; node = node->next )
    {
        if ( strcmp( node->Value(), _value ) == 0 )
            return node;
    }
    return 0;
}


const TiXmlNode* TiXmlNode::LastChild( const char * _value ) const
{
    const TiXmlNode* node;
    for ( node = lastChild; node; node = node->prev )
    {
        if ( strcmp( node->Value(), _value ) == 0 )
            return node;
    }
    return 0;
}

TiXmlNode* TiXmlNode::LastChild( const char * _value )
{
    TiXmlNode* node;
    for ( node = lastChild; node; node = node->prev )
    {
        if ( strcmp( node->Value(), _value ) == 0 )
            return node;
    }
    return 0;
}

const TiXmlNode* TiXmlNode::IterateChildren( const TiXmlNode* previous ) const
{
    if ( !previous )
    {
        return FirstChild();
    }
    else
    {
        assert( previous->parent == this );
        return previous->NextSibling();
    }
}

TiXmlNode* TiXmlNode::IterateChildren( TiXmlNode* previous )
{
    if ( !previous )
    {
        return FirstChild();
    }
    else
    {
        assert( previous->parent == this );
        return previous->NextSibling();
    }
}

const TiXmlNode* TiXmlNode::IterateChildren( const char * val, const TiXmlNode* previous ) const
{
    if ( !previous )
    {
        return FirstChild( val );
    }
    else
    {
        assert( previous->parent == this );
        return previous->NextSibling( val );
    }
}

TiXmlNode* TiXmlNode::IterateChildren( const char * val, TiXmlNode* previous )
{
    if ( !previous )
    {
        return FirstChild( val );
    }
    else
    {
        assert( previous->parent == this );
        return previous->NextSibling( val );
    }
}

const TiXmlNode* TiXmlNode::NextSibling( const char * _value ) const 
{
    const TiXmlNode* node;
    for ( node = next; node; node = node->next )
    {
        if ( strcmp( node->Value(), _value ) == 0 )
            return node;
    }
    return 0;
}

TiXmlNode* TiXmlNode::NextSibling( const char * _value )
{
    TiXmlNode* node;
    for ( node = next; node; node = node->next )
    {
        if ( strcmp( node->Value(), _value ) == 0 )
            return node;
    }
    return 0;
}

const TiXmlNode* TiXmlNode::PreviousSibling( const char * _value ) const
{
    const TiXmlNode* node;
    for ( node = prev; node; node = node->prev )
    {
        if ( strcmp( node->Value(), _value ) == 0 )
            return node;
    }
    return 0;
}

TiXmlNode* TiXmlNode::PreviousSibling( const char * _value )
{
    TiXmlNode* node;
    for ( node = prev; node; node = node->prev )
    {
        if ( strcmp( node->Value(), _value ) == 0 )
            return node;
    }
    return 0;
}

void TiXmlElement::RemoveAttribute( const char * name )
{
    TIXML_STRING str( name );
    TiXmlAttribute* node = attributeSet.Find( str );
    if ( node )
    {
        attributeSet.Remove( node );
        delete node;
    }
}

const TiXmlElement* TiXmlNode::FirstChildElement() const
{
    const TiXmlNode* node;

    for (   node = FirstChild();
            node;
            node = node->NextSibling() )
    {
        if ( node->ToElement() )
            return node->ToElement();
    }
    return 0;
}

TiXmlElement* TiXmlNode::FirstChildElement()
{
    TiXmlNode* node;

    for (   node = FirstChild();
            node;
            node = node->NextSibling() )
    {
        if ( node->ToElement() )
            return node->ToElement();
    }
    return 0;
}

const TiXmlElement* TiXmlNode::FirstChildElement( const char * _value ) const
{
    const TiXmlNode* node;

    for (   node = FirstChild( _value );
            node;
            node = node->NextSibling( _value ) )
    {
        if ( node->ToElement() )
            return node->ToElement();
    }
    return 0;
}

TiXmlElement* TiXmlNode::FirstChildElement( const char * _value )
{
    TiXmlNode* node;

    for (   node = FirstChild( _value );
            node;
            node = node->NextSibling( _value ) )
    {
        if ( node->ToElement() )
            return node->ToElement();
    }
    return 0;
}

const TiXmlElement* TiXmlNode::NextSiblingElement() const
{
    const TiXmlNode* node;

    for (   node = NextSibling();
    node;
    node = node->NextSibling() )
    {
        if ( node->ToElement() )
            return node->ToElement();
    }
    return 0;
}

TiXmlElement* TiXmlNode::NextSiblingElement()
{
    TiXmlNode* node;

    for (   node = NextSibling();
    node;
    node = node->NextSibling() )
    {
        if ( node->ToElement() )
            return node->ToElement();
    }
    return 0;
}

const TiXmlElement* TiXmlNode::NextSiblingElement( const char * _value ) const
{
    const TiXmlNode* node;

    for (   node = NextSibling( _value );
    node;
    node = node->NextSibling( _value ) )
    {
        if ( node->ToElement() )
            return node->ToElement();
    }
    return 0;
}

TiXmlElement* TiXmlNode::NextSiblingElement( const char * _value )
{
    TiXmlNode* node;

    for (   node = NextSibling( _value );
    node;
    node = node->NextSibling( _value ) )
    {
        if ( node->ToElement() )
            return node->ToElement();
    }
    return 0;
}


const TiXmlDocument* TiXmlNode::GetDocument() const
{
    const TiXmlNode* node;

    for( node = this; node; node = node->parent )
    {
        if ( node->ToDocument() )
            return node->ToDocument();
    }
    return 0;
}

TiXmlDocument* TiXmlNode::GetDocument()
{
    TiXmlNode* node;

    for( node = this; node; node = node->parent )
    {
        if ( node->ToDocument() )
            return node->ToDocument();
    }
    return 0;
}

TiXmlElement::TiXmlElement (const char * _value)
    : TiXmlNode( TiXmlNode::ELEMENT )
{
    firstChild = lastChild = 0;
    value = _value;
}


#ifdef TIXML_USE_STL
TiXmlElement::TiXmlElement( const std::string& _value ) 
    : TiXmlNode( TiXmlNode::ELEMENT )
{
    firstChild = lastChild = 0;
    value = _value;
}
#endif


TiXmlElement::TiXmlElement( const TiXmlElement& copy)
    : TiXmlNode( TiXmlNode::ELEMENT )
{
    firstChild = lastChild = 0;
    copy.CopyTo( this );    
}


void TiXmlElement::operator=( const TiXmlElement& base )
{
    ClearThis();
    base.CopyTo( this );
}


TiXmlElement::~TiXmlElement()
{
    ClearThis();
}


void TiXmlElement::ClearThis()
{
    Clear();
    while( attributeSet.First() )
    {
        TiXmlAttribute* node = attributeSet.First();
        attributeSet.Remove( node );
        delete node;
    }
}


const char * TiXmlElement::Attribute( const char * name ) const
{
    TIXML_STRING str( name );
    const TiXmlAttribute* node = attributeSet.Find( str );

    if ( node )
        return node->Value();

    return 0;
}


const char * TiXmlElement::Attribute( const char * name, int* i ) const
{
    const char * s = Attribute( name );
    if ( i )
    {
        if ( s )
            *i = atoi( s );
        else
            *i = 0;
    }
    return s;
}


const char * TiXmlElement::Attribute( const char * name, double* d ) const
{
    const char * s = Attribute( name );
    if ( d )
    {
        if ( s )
            *d = atof( s );
        else
            *d = 0;
    }
    return s;
}


int TiXmlElement::QueryIntAttribute( const char* name, int* ival ) const
{
    TIXML_STRING str( name );
    const TiXmlAttribute* node = attributeSet.Find( str );
    if ( !node )
        return TIXML_NO_ATTRIBUTE;

    return node->QueryIntValue( ival );
}


int TiXmlElement::QueryDoubleAttribute( const char* name, double* dval ) const
{
    TIXML_STRING str( name );
    const TiXmlAttribute* node = attributeSet.Find( str );
    if ( !node )
        return TIXML_NO_ATTRIBUTE;

    return node->QueryDoubleValue( dval );
}


void TiXmlElement::SetAttribute( const char * name, int val )
{   
    char buf[64];
    #if defined(TIXML_SNPRINTF)     
        TIXML_SNPRINTF( buf, sizeof(buf), "%d", val );
    #else
        sprintf( buf, "%d", val );
    #endif
    SetAttribute( name, buf );
}


#ifdef TIXML_USE_STL
void TiXmlElement::SetAttribute( const std::string& name, int val )
{   
   std::ostringstream oss;
   oss << val;
   SetAttribute( name, oss.str() );
}
#endif


void TiXmlElement::SetDoubleAttribute( const char * name, double val )
{   
    char buf[256];
    #if defined(TIXML_SNPRINTF)     
        TIXML_SNPRINTF( buf, sizeof(buf), "%f", val );
    #else
        sprintf( buf, "%f", val );
    #endif
    SetAttribute( name, buf );
}


void TiXmlElement::SetAttribute( const char * cname, const char * cvalue )
{
    TIXML_STRING _name( cname );
    TIXML_STRING _value( cvalue );

    TiXmlAttribute* node = attributeSet.Find( _name );
    if ( node )
    {
        node->SetValue( cvalue );
        return;
    }

    TiXmlAttribute* attrib = new TiXmlAttribute( cname, cvalue );
    if ( attrib )
    {
        attributeSet.Add( attrib );
    }
    else
    {
        TiXmlDocument* document = GetDocument();
        if ( document ) document->SetError( TIXML_ERROR_OUT_OF_MEMORY, 0, 0, TIXML_ENCODING_UNKNOWN );
    }
}


#ifdef TIXML_USE_STL
void TiXmlElement::SetAttribute( const std::string& name, const std::string& _value )
{
    TiXmlAttribute* node = attributeSet.Find( name );
    if ( node )
    {
        node->SetValue( _value );
        return;
    }

    TiXmlAttribute* attrib = new TiXmlAttribute( name, _value );
    if ( attrib )
    {
        attributeSet.Add( attrib );
    }
    else
    {
        TiXmlDocument* document = GetDocument();
        if ( document ) document->SetError( TIXML_ERROR_OUT_OF_MEMORY, 0, 0, TIXML_ENCODING_UNKNOWN );
    }
}
#endif


void TiXmlElement::Print(const Ptr<IO::TextWriter>& textWriter, int depth) const
{
    n_assert(textWriter.isvalid());
    int i;
    for (i = 0; i < depth; i++)
    {
        textWriter->WriteString("    ");
    }
    textWriter->WriteFormatted("<%s", value.c_str());

    const TiXmlAttribute* attrib;
    for (attrib = attributeSet.First(); attrib; attrib = attrib->Next())
    {
        textWriter->WriteChar(' ');
        attrib->Print(textWriter, depth);
    }

    // Существует три различных подхода к форматированию:
    // 1) Элемент без дочерних элементов печатается как узел <foo/>.
    // 2) An element with only a text child is printed as <foo> text </foo>
    // 3) Элемент с дочерними элементами печатается в несколько строк.
    TiXmlNode* node;
    if (!firstChild)
    {
        textWriter->WriteString(" />");
    }
    else if (firstChild == lastChild && firstChild->ToText())
    {
        textWriter->WriteChar('>');
        firstChild->Print(textWriter, depth + 1);
        textWriter->WriteFormatted("</%s>", value.c_str());
    }
    else
    {
        textWriter->WriteChar('>');
        for (node = firstChild; node; node=node->NextSibling())
        {
            if (!node->ToText())
            {
                textWriter->WriteChar('\n');
            }
            node->Print(textWriter, depth+1);
        }
        textWriter->WriteChar('\n');
        for(i=0; i<depth; ++i)
        {
            textWriter->WriteString("    ");
        }
        textWriter->WriteFormatted("</%s>", value.c_str());
    }
}

void TiXmlElement::StreamOut( TIXML_OSTREAM * stream ) const
{
    (*stream) << "<" << value;

    const TiXmlAttribute* attrib;
    for ( attrib = attributeSet.First(); attrib; attrib = attrib->Next() )
    {   
        (*stream) << " ";
        attrib->StreamOut( stream );
    }

    // Если у этого узла есть дочерние элементы, дайте ему закрывающий тег. Еще
    // сделайте это пустым тегом.
    TiXmlNode* node;
    if ( firstChild )
    {       
        (*stream) << ">";

        for ( node = firstChild; node; node=node->NextSibling() )
        {
            node->StreamOut( stream );
        }
        (*stream) << "</" << value << ">";
    }
    else
    {
        (*stream) << " />";
    }
}


void TiXmlElement::CopyTo( TiXmlElement* target ) const
{
    // superclass:
    TiXmlNode::CopyTo( target );

    // Класс элемента: 
    // Клонируйте атрибуты, затем клонируйте дочерние элементы.
    const TiXmlAttribute* attribute = 0;
    for(    attribute = attributeSet.First();
    attribute;
    attribute = attribute->Next() )
    {
        target->SetAttribute( attribute->Name(), attribute->Value() );
    }

    TiXmlNode* node = 0;
    for ( node = firstChild; node; node = node->NextSibling() )
    {
        target->LinkEndChild( node->Clone() );
    }
}


TiXmlNode* TiXmlElement::Clone() const
{
    TiXmlElement* clone = new TiXmlElement( Value() );
    if ( !clone )
        return 0;

    CopyTo( clone );
    return clone;
}


const char* TiXmlElement::GetText() const
{
    const TiXmlNode* child = this->FirstChild();
    if ( child ) {
        const TiXmlText* childText = child->ToText();
        if ( childText ) {
            return childText->Value();
        }
    }
    return 0;
}


TiXmlDocument::TiXmlDocument() : TiXmlNode( TiXmlNode::DOCUMENT )
{
    tabsize = 4;
    useMicrosoftBOM = false;
    ClearError();
}

TiXmlDocument::TiXmlDocument( const TiXmlDocument& copy ) : TiXmlNode( TiXmlNode::DOCUMENT )
{
    copy.CopyTo( this );
}


void TiXmlDocument::operator=( const TiXmlDocument& copy )
{
    Clear();
    copy.CopyTo( this );
}


bool TiXmlDocument::LoadStream(const Ptr<IO::Stream>& stream, TiXmlEncoding encoding)
{
    if (!stream.isvalid()) 
    {
        SetError( TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN );
        return false;
    }

    // Удалить существующие данные:
    Clear();
    location.Clear();

    // Получите размер файла, чтобы мы могли заранее выделить строку. ОГРОМНОЕ влияние скорости.
    IO::Stream::Size length = stream->GetSize();

    // Странный случай, но хорошо разобраться заранее.
    if (length == 0)
    {
        SetError( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
        return false;
    }

    // Если у нас есть файл, предположим, что это один большой XML-файл, и прочитаем его.
    // Однако анализатор документа может решить, что документ заканчивается раньше, чем весь файл.
    TIXML_STRING data;
    data.reserve(length);

    // Здесь тонкая ошибка. TinyXml использовал fgets. Но из спецификации XML:
    // 2.11 Обработка конца строки
    // <фрагмент>
    // <цитата>
    // ...процессор XML ДОЛЖЕН вести себя так, как если бы он нормализовал все разрывы строк во внешних 
    // анализируемые объекты (включая объект документа) на входе перед анализом путем перевода 
    // both the two-character sequence #xD #xA and any #xD that is not followed by #xA to 
    // один символ #xA.
    // </quote>
    //
    // Непонятно, что fgets это делает, и уж точно неясно, работает ли он кроссплатформенно. 
    // Как правило, вы ожидаете, что fgets преобразует соглашение ОС в c/unix.
    // соглашение, а не работать вообще.
    /*
    while (fgets(buf, sizeof(buf), file))
    {
        data += buf;
    }
    */

    char* buf = new char[length+1];
    buf[0] = 0;
    if (stream->Read(buf, length) != length)
    {
        SetError(TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN);
        return false;
    }

    const char* lastPos = buf;
    const char* p = buf;

    buf[length] = 0;
    while( *p ) {
        assert( p < (buf+length) );
        if ( *p == 0xa ) {
            // Символ новой строки. Никаких особых правил для этого нет. Добавить все символы
            // начиная с последней строки, и включите новую строку.
            data.append( lastPos, (p-lastPos+1) );  // добавить, включить новую строку
            ++p;                                    // пройти мимо новой строки
            lastPos = p;                            // and point to the new buffer (may be 0)
            assert( p <= (buf+length) );
        }
        else if ( *p == 0xd ) {
            // Возврат каретки. Добавьте то, что у нас есть на данный момент, затем
            // дескриптор перемещения вперед в буфере.
            if ( (p-lastPos) > 0 ) {
                data.append( lastPos, p-lastPos );  // не добавляйте CR
            }
            data += (char)0xa;                      // правильная новая строка

            if ( *(p+1) == 0xa ) {
                // Возврат каретки – новая последовательность строк
                p += 2;
                lastPos = p;
                assert( p <= (buf+length) );
            }
            else {
                // за этим последовало что-то еще... видимо, снова персонажи.
                ++p;
                lastPos = p;
                assert( p <= (buf+length) );
            }
        }
        else {
            ++p;
        }
    }
    // Обработайте оставшиеся символы.
    if ( p-lastPos ) {
        data.append( lastPos, p-lastPos );
    }       
    delete [] buf;
    buf = 0;

    Parse( data.c_str(), 0, encoding );

    if (  Error() )
        return false;
    else
        return true;
}


bool TiXmlDocument::SaveStream(const Ptr<IO::Stream>& stream) const
{
    Ptr<IO::TextWriter> textWriter = IO::TextWriter::Create();
    textWriter->SetStream(stream);
    if (textWriter->Open())
    {
        if (useMicrosoftBOM) 
        {
            const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
            const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
            const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

            textWriter->WriteChar(TIXML_UTF_LEAD_0);
            textWriter->WriteChar(TIXML_UTF_LEAD_1);
            textWriter->WriteChar(TIXML_UTF_LEAD_2);
        }
        Print(textWriter, 0);
        textWriter->Close();
    }
    return true;
}


void TiXmlDocument::CopyTo( TiXmlDocument* target ) const
{
    TiXmlNode::CopyTo( target );

    target->error = error;
    target->errorDesc = errorDesc.c_str ();

    TiXmlNode* node = 0;
    for ( node = firstChild; node; node = node->NextSibling() )
    {
        target->LinkEndChild( node->Clone() );
    }   
}


TiXmlNode* TiXmlDocument::Clone() const
{
    TiXmlDocument* clone = new TiXmlDocument();
    if ( !clone )
        return 0;

    CopyTo( clone );
    return clone;
}


void TiXmlDocument::Print(const Ptr<IO::TextWriter>& textWriter, int depth) const
{
    const TiXmlNode* node;
    for ( node=FirstChild(); node; node=node->NextSibling() )
    {
        node->Print(textWriter, depth);
        textWriter->WriteChar('\n');
    }
}

void TiXmlDocument::StreamOut( TIXML_OSTREAM * out ) const
{
    const TiXmlNode* node;
    for ( node=FirstChild(); node; node=node->NextSibling() )
    {
        node->StreamOut( out );

        // Специальное правило для потоков: остановка после корневого элемента.
        // Поток в коде будет читать только один элемент, поэтому не
        // напишите больше одного.
        if ( node->ToElement() )
            break;
    }
}


const TiXmlAttribute* TiXmlAttribute::Next() const
{
    // Используем знания дозорного. Страж
    // иметь значение или имя.
    if ( next->value.empty() && next->name.empty() )
        return 0;
    return next;
}

TiXmlAttribute* TiXmlAttribute::Next()
{
    // Используем знания дозорного. Страж
    // иметь значение или имя.
    if ( next->value.empty() && next->name.empty() )
        return 0;
    return next;
}

const TiXmlAttribute* TiXmlAttribute::Previous() const
{
    // Используем знания дозорного. Страж
    // иметь значение или имя.
    if ( prev->value.empty() && prev->name.empty() )
        return 0;
    return prev;
}

TiXmlAttribute* TiXmlAttribute::Previous()
{
    // Используем знания дозорного. Страж
    // иметь значение или имя.
    if ( prev->value.empty() && prev->name.empty() )
        return 0;
    return prev;
}

void TiXmlAttribute::Print(const Ptr<IO::TextWriter>& textWriter, int /*depth*/ ) const
{
    TIXML_STRING n, v;

    PutString( name, &n );
    PutString( value, &v );

    if (value.find ('\"') == TIXML_STRING::npos)
        textWriter->WriteFormatted("%s=\"%s\"", n.c_str(), v.c_str());
    else
        textWriter->WriteFormatted("%s='%s'", n.c_str(), v.c_str());
}


void TiXmlAttribute::StreamOut( TIXML_OSTREAM * stream ) const
{
    if (value.find( '\"' ) != TIXML_STRING::npos)
    {
        PutString( name, stream );
        (*stream) << "=" << "'";
        PutString( value, stream );
        (*stream) << "'";
    }
    else
    {
        PutString( name, stream );
        (*stream) << "=" << "\"";
        PutString( value, stream );
        (*stream) << "\"";
    }
}

int TiXmlAttribute::QueryIntValue( int* ival ) const
{
    if ( sscanf( value.c_str(), "%d", ival ) == 1 )
        return TIXML_SUCCESS;
    return TIXML_WRONG_TYPE;
}

int TiXmlAttribute::QueryDoubleValue( double* dval ) const
{
    if ( sscanf( value.c_str(), "%lф", dval ) == 1 )
        return TIXML_SUCCESS;
    return TIXML_WRONG_TYPE;
}

void TiXmlAttribute::SetIntValue( int _value )
{
    char buf [64];
    #if defined(TIXML_SNPRINTF)     
        TIXML_SNPRINTF(buf, sizeof(buf), "%d", _value);
    #else
        sprintf (buf, "%d", _value);
    #endif
    SetValue (buf);
}

void TiXmlAttribute::SetDoubleValue( double _value )
{
    char buf [256];
    #if defined(TIXML_SNPRINTF)     
        TIXML_SNPRINTF( buf, sizeof(buf), "%lф", _value);
    #else
        sprintf (buf, "%lф", _value);
    #endif
    SetValue (buf);
}

int TiXmlAttribute::IntValue() const
{
    return atoi (value.c_str ());
}

double  TiXmlAttribute::DoubleValue() const
{
    return atof (value.c_str ());
}


TiXmlComment::TiXmlComment( const TiXmlComment& copy ) : TiXmlNode( TiXmlNode::COMMENT )
{
    copy.CopyTo( this );
}


void TiXmlComment::operator=( const TiXmlComment& base )
{
    Clear();
    base.CopyTo( this );
}


void TiXmlComment::Print(const Ptr<IO::TextWriter>& textWriter, int depth) const
{
    for ( int i=0; i<depth; i++ )
    {
        textWriter->WriteString("    ");
    }
    textWriter->WriteFormatted("<!--%s-->", value.c_str());
}

void TiXmlComment::StreamOut( TIXML_OSTREAM * stream ) const
{
    (*stream) << "<!--";
    //PutString(значение, поток);
    (*stream) << value;
    (*stream) << "-->";
}


void TiXmlComment::CopyTo( TiXmlComment* target ) const
{
    TiXmlNode::CopyTo( target );
}


TiXmlNode* TiXmlComment::Clone() const
{
    TiXmlComment* clone = new TiXmlComment();

    if ( !clone )
        return 0;

    CopyTo( clone );
    return clone;
}


void TiXmlText::Print(const Ptr<IO::TextWriter>& textWriter, int depth) const
{
    if (cdata)
    {
        int i;
        textWriter->WriteChar('\n');
        for (i=0; i<depth; i++) 
        {
            textWriter->WriteString("    ");
        }
        textWriter->WriteFormatted("<![CDATA[%s]]>\n", value.c_str());
    }
    else
    {
        if (raw)
        {
            textWriter->WriteString(value.c_str());
        }
        else
        {
            TIXML_STRING buffer;
            PutString( value, &buffer );
            textWriter->WriteString(buffer.c_str());
        }
        
    }
}


void TiXmlText::StreamOut( TIXML_OSTREAM * stream ) const
{
    if ( cdata )
    {
        (*stream) << "<![CDATA[" << value << "]]>";
    }
    else
    {
        PutString( value, stream );
    }
}


void TiXmlText::CopyTo( TiXmlText* target ) const
{
    TiXmlNode::CopyTo( target );
    target->cdata = cdata;
    target->raw = raw;
}


TiXmlNode* TiXmlText::Clone() const
{   
    TiXmlText* clone = 0;
    clone = new TiXmlText( "" );

    if ( !clone )
        return 0;

    CopyTo( clone );
    return clone;
}


TiXmlDeclaration::TiXmlDeclaration( const char * _version,
                                    const char * _encoding,
                                    const char * _standalone )
    : TiXmlNode( TiXmlNode::DECLARATION )
{
    version = _version;
    encoding = _encoding;
    standalone = _standalone;
}


#ifdef TIXML_USE_STL
TiXmlDeclaration::TiXmlDeclaration( const std::string& _version,
                                    const std::string& _encoding,
                                    const std::string& _standalone )
    : TiXmlNode( TiXmlNode::DECLARATION )
{
    version = _version;
    encoding = _encoding;
    standalone = _standalone;
}
#endif


TiXmlDeclaration::TiXmlDeclaration( const TiXmlDeclaration& copy )
    : TiXmlNode( TiXmlNode::DECLARATION )
{
    copy.CopyTo( this );    
}


void TiXmlDeclaration::operator=( const TiXmlDeclaration& copy )
{
    Clear();
    copy.CopyTo( this );
}


void TiXmlDeclaration::Print(const Ptr<IO::TextWriter>& textWriter, int /*depth*/ ) const
{
    textWriter->WriteString("<?xml ");
    if ( !version.empty() )
        textWriter->WriteFormatted("версия=\"%s\" ", version.c_str ());
    if ( !encoding.empty() )
        textWriter->WriteFormatted("кодирование=\"%s\" ", encoding.c_str ());
    if ( !standalone.empty() )
        textWriter->WriteFormatted("автономный = \"%s\" ", standalone.c_str ());
    textWriter->WriteString("?>");
}

void TiXmlDeclaration::StreamOut( TIXML_OSTREAM * stream ) const
{
    (*stream) << "<?xml ";

    if ( !version.empty() )
    {
        (*stream) << "версия=\"";
        PutString( version, stream );
        (*stream) << "\" ";
    }
    if ( !encoding.empty() )
    {
        (*stream) << "кодировка=\"";
        PutString( encoding, stream );
        (*stream ) << "\" ";
    }
    if ( !standalone.empty() )
    {
        (*stream) << "автономный=\"";
        PutString( standalone, stream );
        (*stream) << "\" ";
    }
    (*stream) << "?>";
}


void TiXmlDeclaration::CopyTo( TiXmlDeclaration* target ) const
{
    TiXmlNode::CopyTo( target );

    target->version = version;
    target->encoding = encoding;
    target->standalone = standalone;
}


TiXmlNode* TiXmlDeclaration::Clone() const
{   
    TiXmlDeclaration* clone = new TiXmlDeclaration();

    if ( !clone )
        return 0;

    CopyTo( clone );
    return clone;
}


void TiXmlUnknown::Print(const Ptr<IO::TextWriter>& textWriter, int depth ) const
{
    for ( int i=0; i<depth; i++ )
        textWriter->WriteString("    ");
    textWriter->WriteFormatted("<%s>", value.c_str());
}


void TiXmlUnknown::StreamOut( TIXML_OSTREAM * stream ) const
{
    (*stream) << "<" << value << ">";       // Не используйте здесь сущности! Это неизвестно.
}


void TiXmlUnknown::CopyTo( TiXmlUnknown* target ) const
{
    TiXmlNode::CopyTo( target );
}


TiXmlNode* TiXmlUnknown::Clone() const
{
    TiXmlUnknown* clone = new TiXmlUnknown();

    if ( !clone )
        return 0;

    CopyTo( clone );
    return clone;
}


TiXmlAttributeSet::TiXmlAttributeSet()
{
    sentinel.next = &sentinel;
    sentinel.prev = &sentinel;
}


TiXmlAttributeSet::~TiXmlAttributeSet()
{
    assert( sentinel.next == &sentinel );
    assert( sentinel.prev == &sentinel );
}


void TiXmlAttributeSet::Add( TiXmlAttribute* addMe )
{
    assert( !Find( TIXML_STRING( addMe->Name() ) ) );   // Не следует многократно добавлять к набору.

    addMe->next = &sentinel;
    addMe->prev = sentinel.prev;

    sentinel.prev->next = addMe;
    sentinel.prev      = addMe;
}

void TiXmlAttributeSet::Remove( TiXmlAttribute* removeMe )
{
    TiXmlAttribute* node;

    for( node = sentinel.next; node != &sentinel; node = node->next )
    {
        if ( node == removeMe )
        {
            node->prev->next = node->next;
            node->next->prev = node->prev;
            node->next = 0;
            node->prev = 0;
            return;
        }
    }
    assert( 0 );        // мы попытались удалить несвязанный атрибут.
}

const TiXmlAttribute* TiXmlAttributeSet::Find( const TIXML_STRING& name ) const
{
    const TiXmlAttribute* node;

    for( node = sentinel.next; node != &sentinel; node = node->next )
    {
        if ( node->name == name )
            return node;
    }
    return 0;
}

TiXmlAttribute* TiXmlAttributeSet::Find( const TIXML_STRING& name )
{
    TiXmlAttribute* node;

    for( node = sentinel.next; node != &sentinel; node = node->next )
    {
        if ( node->name == name )
            return node;
    }
    return 0;
}

#ifdef TIXML_USE_STL    
TIXML_ISTREAM & operator >> (TIXML_ISTREAM & in, TiXmlNode & base)
{
    TIXML_STRING tag;
    tag.reserve( 8 * 1000 );
    base.StreamIn( &in, &tag );

    base.Parse( tag.c_str(), 0, TIXML_DEFAULT_ENCODING );
    return in;
}
#endif


TIXML_OSTREAM & operator<< (TIXML_OSTREAM & out, const TiXmlNode & base)
{
    base.StreamOut (& out);
    return out;
}


#ifdef TIXML_USE_STL    
std::string & operator<< (std::string& out, const TiXmlNode& base )
{
   std::ostringstream os_stream( std::ostringstream::out );
   base.StreamOut( &os_stream );
   
   out.append( os_stream.str() );
   return out;
}
#endif


TiXmlHandle TiXmlHandle::FirstChild() const
{
    if ( node )
    {
        TiXmlNode* child = node->FirstChild();
        if ( child )
            return TiXmlHandle( child );
    }
    return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::FirstChild( const char * value ) const
{
    if ( node )
    {
        TiXmlNode* child = node->FirstChild( value );
        if ( child )
            return TiXmlHandle( child );
    }
    return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::FirstChildElement() const
{
    if ( node )
    {
        TiXmlElement* child = node->FirstChildElement();
        if ( child )
            return TiXmlHandle( child );
    }
    return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::FirstChildElement( const char * value ) const
{
    if ( node )
    {
        TiXmlElement* child = node->FirstChildElement( value );
        if ( child )
            return TiXmlHandle( child );
    }
    return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::Child( int count ) const
{
    if ( node )
    {
        int i;
        TiXmlNode* child = node->FirstChild();
        for (   i=0;
                child && i<count;
                child = child->NextSibling(), ++i )
        {
            // nothing
        }
        if ( child )
            return TiXmlHandle( child );
    }
    return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::Child( const char* value, int count ) const
{
    if ( node )
    {
        int i;
        TiXmlNode* child = node->FirstChild( value );
        for (   i=0;
                child && i<count;
                child = child->NextSibling( value ), ++i )
        {
            // nothing
        }
        if ( child )
            return TiXmlHandle( child );
    }
    return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::ChildElement( int count ) const
{
    if ( node )
    {
        int i;
        TiXmlElement* child = node->FirstChildElement();
        for (   i=0;
                child && i<count;
                child = child->NextSiblingElement(), ++i )
        {
            // nothing
        }
        if ( child )
            return TiXmlHandle( child );
    }
    return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::ChildElement( const char* value, int count ) const
{
    if ( node )
    {
        int i;
        TiXmlElement* child = node->FirstChildElement( value );
        for (   i=0;
                child && i<count;
                child = child->NextSiblingElement( value ), ++i )
        {
            // nothing
        }
        if ( child )
            return TiXmlHandle( child );
    }
    return TiXmlHandle( 0 );
}
