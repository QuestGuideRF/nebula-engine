#ifndef TINYXML_INCLUDED
#define TINYXML_INCLUDED
#ifdef _MSC_VER
#pragma warning( disable : 4530 )
#pragma warning( disable : 4786 )
#pragma warning( disable : 4996 )
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "io/stream.h"
#include "io/textwriter.h"
#if defined( _DEBUG ) && !defined( DEBUG )
#define DEBUG
#endif
#ifdef TIXML_USE_STL
    #include <string>
    #include <iostream>
    #define TIXML_STRING    std::string
    #define TIXML_ISTREAM   std::istream
    #define TIXML_OSTREAM   std::ostream
#else
    #include "tinystr.h"
    #define TIXML_STRING    TiXmlString
    #define TIXML_OSTREAM   TiXmlOutStream
#endif
#define TIXML_SAFE
#ifdef TIXML_SAFE
    #if defined(_MSC_VER) && (_MSC_VER >= 1400 )
        #define TIXML_SNPRINTF _snprintf_s
        #define TIXML_SNSCANF  _snscanf_s
    #elif defined(_MSC_VER) && (_MSC_VER >= 1200 )
        #define TIXML_SNPRINTF _snprintf
        #define TIXML_SNSCANF  _snscanf
    #elif defined(__GNUC__) && (__GNUC__ >= 3 )
        #define TIXML_SNPRINTF snprintf
        #define TIXML_SNSCANF  snscanf
    #endif
#endif
class TiXmlDocument;
class TiXmlElement;
class TiXmlComment;
class TiXmlUnknown;
class TiXmlAttribute;
class TiXmlText;
class TiXmlDeclaration;
class TiXmlParsingData;
const int TIXML_MAJOR_VERSION = 2;
const int TIXML_MINOR_VERSION = 4;
const int TIXML_PATCH_VERSION = 3;
struct TiXmlCursor
{
    TiXmlCursor()       { Clear(); }
    void Clear()        { row = col = -1; }
    int row;
    int col;
};
enum
{
    TIXML_SUCCESS,
    TIXML_NO_ATTRIBUTE,
    TIXML_WRONG_TYPE
};
enum TiXmlEncoding
{
    TIXML_ENCODING_UNKNOWN,
    TIXML_ENCODING_UTF8,
    TIXML_ENCODING_LEGACY
};
const TiXmlEncoding TIXML_DEFAULT_ENCODING = TIXML_ENCODING_UNKNOWN;
class TiXmlBase
{
    friend class TiXmlNode;
    friend class TiXmlElement;
    friend class TiXmlDocument;
public:
    TiXmlBase() :   userData(0) {}
    virtual ~TiXmlBase()                    {}
    virtual void Print(const Ptr<IO::TextWriter>& textWriter, int depth ) const = 0;
    static void SetCondenseWhiteSpace( bool condense )      { condenseWhiteSpace = condense; }
    static bool IsWhiteSpaceCondensed()                     { return condenseWhiteSpace; }
    int Row() const         { return location.row + 1; }
    int Column() const      { return location.col + 1; }
    void  SetUserData( void* user )         { userData = user; }
    void* GetUserData()                     { return userData; }
    static const int utf8ByteTable[256];
    virtual const char* Parse(  const char* p,
                                TiXmlParsingData* data,
                                TiXmlEncoding encoding  ) = 0;
    enum
    {
        TIXML_NO_ERROR = 0,
        TIXML_ERROR,
        TIXML_ERROR_OPENING_FILE,
        TIXML_ERROR_OUT_OF_MEMORY,
        TIXML_ERROR_PARSING_ELEMENT,
        TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME,
        TIXML_ERROR_READING_ELEMENT_VALUE,
        TIXML_ERROR_READING_ATTRIBUTES,
        TIXML_ERROR_PARSING_EMPTY,
        TIXML_ERROR_READING_END_TAG,
        TIXML_ERROR_PARSING_UNKNOWN,
        TIXML_ERROR_PARSING_COMMENT,
        TIXML_ERROR_PARSING_DECLARATION,
        TIXML_ERROR_DOCUMENT_EMPTY,
        TIXML_ERROR_EMBEDDED_NULL,
        TIXML_ERROR_PARSING_CDATA,
        TIXML_ERROR_STRING_COUNT
    };
protected:
    class StringToBuffer
    {
      public:
        StringToBuffer( const TIXML_STRING& str );
        ~StringToBuffer();
        char* buffer;
    };
    static const char*  SkipWhiteSpace( const char*, TiXmlEncoding encoding );
    inline static bool  IsWhiteSpace( char c )
    {
        return ( isspace( (unsigned char) c ) || c == '\n' || c == '\r' );
    }
    inline static bool  IsWhiteSpace( int c )
    {
        if ( c < 256 )
            return IsWhiteSpace( (char) c );
        return false;
    }
    virtual void StreamOut (TIXML_OSTREAM *) const = 0;
    #ifdef TIXML_USE_STL
        static bool StreamWhiteSpace( TIXML_ISTREAM * in, TIXML_STRING * tag );
        static bool StreamTo( TIXML_ISTREAM * in, int character, TIXML_STRING * tag );
    #endif
    static const char* ReadName( const char* p, TIXML_STRING* name, TiXmlEncoding encoding );
    static const char* ReadText(    const char* in,
                                    TIXML_STRING* text,
                                    bool ignoreWhiteSpace,
                                    const char* endTag,
                                    bool ignoreCase,
                                    TiXmlEncoding encoding );
    static const char* GetEntity( const char* in, char* value, int* length, TiXmlEncoding encoding );
    inline static const char* GetChar( const char* p, char* _value, int* length, TiXmlEncoding encoding )
    {
        assert( p );
        if ( encoding == TIXML_ENCODING_UTF8 )
        {
            *length = utf8ByteTable[ *((unsigned char*)p) ];
            assert( *length >= 0 && *length < 5 );
        }
        else
        {
            *length = 1;
        }
        if ( *length == 1 )
        {
            if ( *p == '&' )
                return GetEntity( p, _value, length, encoding );
            *_value = *p;
            return p+1;
        }
        else if ( *length )
        {
            for( int i=0; p[i] && i<*length; ++i ) {
                _value[i] = p[i];
            }
            return p + (*length);
        }
        else
        {
            return 0;
        }
    }
    static void PutString( const TIXML_STRING& str, TIXML_OSTREAM* out );
    static void PutString( const TIXML_STRING& str, TIXML_STRING* out );
    static bool StringEqual(    const char* p,
                                const char* endTag,
                                bool ignoreCase,
                                TiXmlEncoding encoding );
    static const char* errorString[ TIXML_ERROR_STRING_COUNT ];
    TiXmlCursor location;
    void*           userData;
    static int IsAlpha( unsigned char anyByte, TiXmlEncoding encoding );
    static int IsAlphaNum( unsigned char anyByte, TiXmlEncoding encoding );
    inline static int ToLower( int v, TiXmlEncoding encoding )
    {
        if ( encoding == TIXML_ENCODING_UTF8 )
        {
            if ( v < 128 ) return tolower( v );
            return v;
        }
        else
        {
            return tolower( v );
        }
    }
    static void ConvertUTF32ToUTF8( unsigned long input, char* output, int* length );
private:
    TiXmlBase( const TiXmlBase& );
    void operator=( const TiXmlBase& base );
    struct Entity
    {
        const char*     str;
        unsigned int    strLength;
        char            chr;
    };
    enum
    {
        NUM_ENTITY = 5,
        MAX_ENTITY_LENGTH = 6
    };
    static Entity entity[ NUM_ENTITY ];
    static bool condenseWhiteSpace;
};
class TiXmlNode : public TiXmlBase
{
    friend class TiXmlDocument;
    friend class TiXmlElement;
public:
    #ifdef TIXML_USE_STL
        friend std::istream& operator >> (std::istream& in, TiXmlNode& base);
        friend std::ostream& operator<< (std::ostream& out, const TiXmlNode& base);
        friend std::string& operator<< (std::string& out, const TiXmlNode& base );
    #else
        friend TIXML_OSTREAM& operator<< (TIXML_OSTREAM& out, const TiXmlNode& base);
    #endif
    enum NodeType
    {
        DOCUMENT,
        ELEMENT,
        COMMENT,
        UNKNOWN,
        TEXT,
        DECLARATION,
        TYPECOUNT
    };
    virtual ~TiXmlNode();
    const char *Value() const { return value.c_str (); }
    #ifdef TIXML_USE_STL
    const std::string& ValueStr() const { return value; }
    #endif
    void SetValue(const char * _value) { value = _value;}
    #ifdef TIXML_USE_STL
    void SetValue( const std::string& _value )  { value = _value; }
    #endif
    void Clear();
    TiXmlNode* Parent()                         { return parent; }
    const TiXmlNode* Parent() const             { return parent; }
    const TiXmlNode* FirstChild()   const   { return firstChild; }
    TiXmlNode* FirstChild()                 { return firstChild; }
    const TiXmlNode* FirstChild( const char * value ) const;
    TiXmlNode* FirstChild( const char * value );
    const TiXmlNode* LastChild() const  { return lastChild; }
    TiXmlNode* LastChild()  { return lastChild; }
    const TiXmlNode* LastChild( const char * value ) const;
    TiXmlNode* LastChild( const char * value );
    #ifdef TIXML_USE_STL
    const TiXmlNode* FirstChild( const std::string& _value ) const  {   return FirstChild (_value.c_str ());    }
    TiXmlNode* FirstChild( const std::string& _value )              {   return FirstChild (_value.c_str ());    }
    const TiXmlNode* LastChild( const std::string& _value ) const   {   return LastChild (_value.c_str ()); }
    TiXmlNode* LastChild( const std::string& _value )               {   return LastChild (_value.c_str ()); }
    #endif
    const TiXmlNode* IterateChildren( const TiXmlNode* previous ) const;
    TiXmlNode* IterateChildren( TiXmlNode* previous );
    const TiXmlNode* IterateChildren( const char * value, const TiXmlNode* previous ) const;
    TiXmlNode* IterateChildren( const char * value, TiXmlNode* previous );
    #ifdef TIXML_USE_STL
    const TiXmlNode* IterateChildren( const std::string& _value, const TiXmlNode* previous ) const  {   return IterateChildren (_value.c_str (), previous); }
    TiXmlNode* IterateChildren( const std::string& _value, TiXmlNode* previous ) {  return IterateChildren (_value.c_str (), previous); }
    #endif
    TiXmlNode* InsertEndChild( const TiXmlNode& addThis );
    TiXmlNode* LinkEndChild( TiXmlNode* addThis );
    TiXmlNode* InsertBeforeChild( TiXmlNode* beforeThis, const TiXmlNode& addThis );
    TiXmlNode* InsertAfterChild(  TiXmlNode* afterThis, const TiXmlNode& addThis );
    TiXmlNode* ReplaceChild( TiXmlNode* replaceThis, const TiXmlNode& withThis );
    bool RemoveChild( TiXmlNode* removeThis );
    const TiXmlNode* PreviousSibling() const            { return prev; }
    TiXmlNode* PreviousSibling()                        { return prev; }
    const TiXmlNode* PreviousSibling( const char * ) const;
    TiXmlNode* PreviousSibling( const char * );
    #ifdef TIXML_USE_STL
    const TiXmlNode* PreviousSibling( const std::string& _value ) const {   return PreviousSibling (_value.c_str ());   }
    TiXmlNode* PreviousSibling( const std::string& _value )             {   return PreviousSibling (_value.c_str ());   }
    const TiXmlNode* NextSibling( const std::string& _value) const      {   return NextSibling (_value.c_str ());   }
    TiXmlNode* NextSibling( const std::string& _value)                  {   return NextSibling (_value.c_str ());   }
    #endif
    const TiXmlNode* NextSibling() const                { return next; }
    TiXmlNode* NextSibling()                            { return next; }
    const TiXmlNode* NextSibling( const char * ) const;
    TiXmlNode* NextSibling( const char * );
    const TiXmlElement* NextSiblingElement() const;
    TiXmlElement* NextSiblingElement();
    const TiXmlElement* NextSiblingElement( const char * ) const;
    TiXmlElement* NextSiblingElement( const char * );
    #ifdef TIXML_USE_STL
    const TiXmlElement* NextSiblingElement( const std::string& _value) const    {   return NextSiblingElement (_value.c_str ());    }
    TiXmlElement* NextSiblingElement( const std::string& _value)                {   return NextSiblingElement (_value.c_str ());    }
    #endif
    const TiXmlElement* FirstChildElement() const;
    TiXmlElement* FirstChildElement();
    const TiXmlElement* FirstChildElement( const char * value ) const;
    TiXmlElement* FirstChildElement( const char * value );
    #ifdef TIXML_USE_STL
    const TiXmlElement* FirstChildElement( const std::string& _value ) const    {   return FirstChildElement (_value.c_str ()); }
    TiXmlElement* FirstChildElement( const std::string& _value )                {   return FirstChildElement (_value.c_str ()); }
    #endif
    int Type() const    { return type; }
    const TiXmlDocument* GetDocument() const;
    TiXmlDocument* GetDocument();
    bool NoChildren() const                     { return !firstChild; }
    virtual const TiXmlDocument*    ToDocument()    const { return 0; }
    virtual const TiXmlElement*     ToElement()     const { return 0; }
    virtual const TiXmlComment*     ToComment()     const { return 0; }
    virtual const TiXmlUnknown*     ToUnknown()     const { return 0; }
    virtual const TiXmlText*        ToText()        const { return 0; }
    virtual const TiXmlDeclaration* ToDeclaration() const { return 0; }
    virtual TiXmlDocument*          ToDocument()    { return 0; }
    virtual TiXmlElement*           ToElement()     { return 0; }
    virtual TiXmlComment*           ToComment()     { return 0; }
    virtual TiXmlUnknown*           ToUnknown()     { return 0; }
    virtual TiXmlText*              ToText()        { return 0; }
    virtual TiXmlDeclaration*       ToDeclaration() { return 0; }
    virtual TiXmlNode* Clone() const = 0;
protected:
    TiXmlNode( NodeType _type );
    void CopyTo( TiXmlNode* target ) const;
    #ifdef TIXML_USE_STL
        virtual void StreamIn( TIXML_ISTREAM* in, TIXML_STRING* tag ) = 0;
    #endif
    TiXmlNode* Identify( const char* start, TiXmlEncoding encoding );
    TiXmlNode*      parent;
    NodeType        type;
    TiXmlNode*      firstChild;
    TiXmlNode*      lastChild;
    TIXML_STRING    value;
    TiXmlNode*      prev;
    TiXmlNode*      next;
private:
    TiXmlNode( const TiXmlNode& );
    void operator=( const TiXmlNode& base );
};
class TiXmlAttribute : public TiXmlBase
{
    friend class TiXmlAttributeSet;
public:
    TiXmlAttribute() : TiXmlBase()
    {
        document = 0;
        prev = next = 0;
    }
    #ifdef TIXML_USE_STL
    TiXmlAttribute( const std::string& _name, const std::string& _value )
    {
        name = _name;
        value = _value;
        document = 0;
        prev = next = 0;
    }
    #endif
    TiXmlAttribute( const char * _name, const char * _value )
    {
        name = _name;
        value = _value;
        document = 0;
        prev = next = 0;
    }
    const char*     Name()  const       { return name.c_str (); }
    const char*     Value() const       { return value.c_str (); }
    int             IntValue() const;
    double          DoubleValue() const;
    const TIXML_STRING& NameTStr() const { return name; }
    int QueryIntValue( int* _value ) const;
    int QueryDoubleValue( double* _value ) const;
    void SetName( const char* _name )   { name = _name; }
    void SetValue( const char* _value ) { value = _value; }
    void SetIntValue( int _value );
    void SetDoubleValue( double _value );
    #ifdef TIXML_USE_STL
    void SetName( const std::string& _name )    { name = _name; }
    void SetValue( const std::string& _value )  { value = _value; }
    #endif
    const TiXmlAttribute* Next() const;
    TiXmlAttribute* Next();
    const TiXmlAttribute* Previous() const;
    TiXmlAttribute* Previous();
    bool operator==( const TiXmlAttribute& rhs ) const { return rhs.name == name; }
    bool operator<( const TiXmlAttribute& rhs )  const { return name < rhs.name; }
    bool operator>( const TiXmlAttribute& rhs )  const { return name > rhs.name; }
    virtual const char* Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding );
    virtual void Print(const Ptr<IO::TextWriter>& textWriter, int depth) const;
    virtual void StreamOut( TIXML_OSTREAM * out ) const;
    void SetDocument( TiXmlDocument* doc )  { document = doc; }
private:
    TiXmlAttribute( const TiXmlAttribute& );
    void operator=( const TiXmlAttribute& base );
    TiXmlDocument*  document;
    TIXML_STRING name;
    TIXML_STRING value;
    TiXmlAttribute* prev;
    TiXmlAttribute* next;
};
class TiXmlAttributeSet
{
public:
    TiXmlAttributeSet();
    ~TiXmlAttributeSet();
    void Add( TiXmlAttribute* attribute );
    void Remove( TiXmlAttribute* attribute );
    const TiXmlAttribute* First()   const   { return ( sentinel.next == &sentinel ) ? 0 : sentinel.next; }
    TiXmlAttribute* First()                 { return ( sentinel.next == &sentinel ) ? 0 : sentinel.next; }
    const TiXmlAttribute* Last() const      { return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }
    TiXmlAttribute* Last()                  { return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }
    const TiXmlAttribute*   Find( const TIXML_STRING& name ) const;
    TiXmlAttribute* Find( const TIXML_STRING& name );
private:
    /
class TiXmlElement : public TiXmlNode
{
public:
    TiXmlElement (const char * in_value);
    #ifdef TIXML_USE_STL
    TiXmlElement( const std::string& _value );
    #endif
    TiXmlElement( const TiXmlElement& );
    void operator=( const TiXmlElement& base );
    virtual ~TiXmlElement();
    const char* Attribute( const char* name ) const;
    const char* Attribute( const char* name, int* i ) const;
    const char* Attribute( const char* name, double* d ) const;
    int QueryIntAttribute( const char* name, int* _value ) const;
    int QueryDoubleAttribute( const char* name, double* _value ) const;
    int QueryFloatAttribute( const char* name, float* _value ) const {
        double d;
        int result = QueryDoubleAttribute( name, &d );
        if ( result == TIXML_SUCCESS ) {
            *_value = (float)d;
        }
        return result;
    }
    void SetAttribute( const char* name, const char * _value );
    #ifdef TIXML_USE_STL
    const char* Attribute( const std::string& name ) const              { return Attribute( name.c_str() ); }
    const char* Attribute( const std::string& name, int* i ) const      { return Attribute( name.c_str(), i ); }
    const char* Attribute( const std::string& name, double* d ) const   { return Attribute( name.c_str(), d ); }
    int QueryIntAttribute( const std::string& name, int* _value ) const { return QueryIntAttribute( name.c_str(), _value ); }
    int QueryDoubleAttribute( const std::string& name, double* _value ) const { return QueryDoubleAttribute( name.c_str(), _value ); }
    void SetAttribute( const std::string& name, const std::string& _value );
    void SetAttribute( const std::string& name, int _value );
    #endif
    void SetAttribute( const char * name, int value );
    void SetDoubleAttribute( const char * name, double value );
    void RemoveAttribute( const char * name );
    #ifdef TIXML_USE_STL
    void RemoveAttribute( const std::string& name ) {   RemoveAttribute (name.c_str ());    }
    #endif
    const TiXmlAttribute* FirstAttribute() const    { return attributeSet.First(); }
    TiXmlAttribute* FirstAttribute()                { return attributeSet.First(); }
    const TiXmlAttribute* LastAttribute()   const   { return attributeSet.Last(); }
    TiXmlAttribute* LastAttribute()                 { return attributeSet.Last(); }
    const char* GetText() const;
    virtual TiXmlNode* Clone() const;
    virtual void Print(const Ptr<IO::TextWriter>& textWriter, int depth) const;
    virtual const char* Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding );
    virtual const TiXmlElement*     ToElement()     const { return this; }
    virtual TiXmlElement*           ToElement()           { return this; }
protected:
    void CopyTo( TiXmlElement* target ) const;
    void ClearThis();
    #ifdef TIXML_USE_STL
        virtual void StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag );
    #endif
    virtual void StreamOut( TIXML_OSTREAM * out ) const;
    const char* ReadValue( const char* in, TiXmlParsingData* prevData, TiXmlEncoding encoding );
private:
    TiXmlAttributeSet attributeSet;
};
class TiXmlComment : public TiXmlNode
{
public:
    TiXmlComment() : TiXmlNode( TiXmlNode::COMMENT ) {}
    TiXmlComment( const TiXmlComment& );
    void operator=( const TiXmlComment& base );
    virtual ~TiXmlComment() {}
    virtual TiXmlNode* Clone() const;
    virtual void Print(const Ptr<IO::TextWriter>& textWriter, int depth) const;
    virtual const char* Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding );
    virtual const TiXmlComment*  ToComment() const { return this; }
    virtual TiXmlComment*  ToComment() { return this; }
protected:
    void CopyTo( TiXmlComment* target ) const;
    #ifdef TIXML_USE_STL
        virtual void StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag );
    #endif
    virtual void StreamOut( TIXML_OSTREAM * out ) const;
private:
};
class TiXmlText : public TiXmlNode
{
    friend class TiXmlElement;
public:
    TiXmlText (const char * initValue ) : TiXmlNode (TiXmlNode::TEXT)
    {
        SetValue( initValue );
        cdata = false;
    }
    virtual ~TiXmlText() {}
    #ifdef TIXML_USE_STL
    TiXmlText( const std::string& initValue ) : TiXmlNode (TiXmlNode::TEXT)
    {
        SetValue( initValue );
        cdata = false;
    }
    #endif
    TiXmlText( const TiXmlText& copy ) : TiXmlNode( TiXmlNode::TEXT )   { copy.CopyTo( this ); }
    void operator=( const TiXmlText& base )                             { base.CopyTo( this ); }
    virtual void Print(const Ptr<IO::TextWriter>& textWriter, int depth) const;
    bool CDATA()                    { return cdata; }
    void SetCDATA( bool _cdata )    { cdata = _cdata; }
    bool Raw()                      { return raw; }
    void SetRaw( bool _raw )        { raw = _raw; }
    virtual const char* Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding );
    virtual const TiXmlText* ToText() const { return this; }
    virtual TiXmlText*       ToText()       { return this; }
protected :
    virtual TiXmlNode* Clone() const;
    void CopyTo( TiXmlText* target ) const;
    virtual void StreamOut ( TIXML_OSTREAM * out ) const;
    bool Blank() const;
    #ifdef TIXML_USE_STL
        virtual void StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag );
    #endif
private:
    bool cdata;
    bool raw;
};
class TiXmlDeclaration : public TiXmlNode
{
public:
    TiXmlDeclaration()   : TiXmlNode( TiXmlNode::DECLARATION ) {}
#ifdef TIXML_USE_STL
    TiXmlDeclaration(   const std::string& _version,
                        const std::string& _encoding,
                        const std::string& _standalone );
#endif
    TiXmlDeclaration(   const char* _version,
                        const char* _encoding,
                        const char* _standalone );
    TiXmlDeclaration( const TiXmlDeclaration& copy );
    void operator=( const TiXmlDeclaration& copy );
    virtual ~TiXmlDeclaration() {}
    const char *Version() const         { return version.c_str (); }
    const char *Encoding() const        { return encoding.c_str (); }
    const char *Standalone() const      { return standalone.c_str (); }
    virtual TiXmlNode* Clone() const;
    virtual void Print(const Ptr<IO::TextWriter>& textWriter, int depth) const;
    virtual const char* Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding );
    virtual const TiXmlDeclaration* ToDeclaration() const { return this; }
    virtual TiXmlDeclaration*       ToDeclaration()       { return this; }
protected:
    void CopyTo( TiXmlDeclaration* target ) const;
    #ifdef TIXML_USE_STL
        virtual void StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag );
    #endif
    virtual void StreamOut ( TIXML_OSTREAM * out) const;
private:
    TIXML_STRING version;
    TIXML_STRING encoding;
    TIXML_STRING standalone;
};
class TiXmlUnknown : public TiXmlNode
{
public:
    TiXmlUnknown() : TiXmlNode( TiXmlNode::UNKNOWN )    {}
    virtual ~TiXmlUnknown() {}
    TiXmlUnknown( const TiXmlUnknown& copy ) : TiXmlNode( TiXmlNode::UNKNOWN )      { copy.CopyTo( this ); }
    void operator=( const TiXmlUnknown& copy )                                      { copy.CopyTo( this ); }
    virtual TiXmlNode* Clone() const;
    virtual void Print(const Ptr<IO::TextWriter>& textWriter, int depth) const;
    virtual const char* Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding );
    virtual const TiXmlUnknown*     ToUnknown()     const { return this; }
    virtual TiXmlUnknown*           ToUnknown()     { return this; }
protected:
    void CopyTo( TiXmlUnknown* target ) const;
    #ifdef TIXML_USE_STL
        virtual void StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag );
    #endif
    virtual void StreamOut ( TIXML_OSTREAM * out ) const;
private:
};
class TiXmlDocument : public TiXmlNode
{
public:
    TiXmlDocument();
    TiXmlDocument( const TiXmlDocument& copy );
    void operator=( const TiXmlDocument& copy );
    virtual ~TiXmlDocument() {}
    bool LoadStream(const Ptr<IO::Stream>& stream, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING );
    bool SaveStream(const Ptr<IO::Stream>& stream) const;
    #ifdef TIXML_USE_STL
    bool LoadFile( const std::string& filename, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING )
    {
        StringToBuffer f( filename );
        return ( f.buffer && LoadFile( f.buffer, encoding ));
    }
    bool SaveFile( const std::string& filename ) const
    {
        StringToBuffer f( filename );
        return ( f.buffer && SaveFile( f.buffer ));
    }
    #endif
    virtual const char* Parse( const char* p, TiXmlParsingData* data = 0, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING );
    const TiXmlElement* RootElement() const     { return FirstChildElement(); }
    TiXmlElement* RootElement()                 { return FirstChildElement(); }
    bool Error() const                      { return error; }
    const char * ErrorDesc() const  { return errorDesc.c_str (); }
    int ErrorId()   const               { return errorId; }
    int ErrorRow()  { return errorLocation.row+1; }
    int ErrorCol()  { return errorLocation.col+1; }
    void SetTabSize( int _tabsize )     { tabsize = _tabsize; }
    int TabSize() const { return tabsize; }
    void ClearError()                       {   error = false;
                                                errorId = 0;
                                                errorDesc = "";
                                                errorLocation.row = errorLocation.col = 0;
                                            }
    virtual void Print(const Ptr<IO::TextWriter>& textWriter, int depth = 0) const;
    void SetError( int err, const char* errorLocation, TiXmlParsingData* prevData, TiXmlEncoding encoding );
    virtual const TiXmlDocument*    ToDocument()    const { return this; }
    virtual TiXmlDocument*          ToDocument()          { return this; }
protected :
    virtual void StreamOut ( TIXML_OSTREAM * out) const;
    virtual TiXmlNode* Clone() const;
    #ifdef TIXML_USE_STL
        virtual void StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag );
    #endif
private:
    void CopyTo( TiXmlDocument* target ) const;
    bool error;
    int  errorId;
    TIXML_STRING errorDesc;
    int tabsize;
    TiXmlCursor errorLocation;
    bool useMicrosoftBOM;
};
class TiXmlHandle
{
public:
    TiXmlHandle( TiXmlNode* _node )                 { this->node = _node; }
    TiXmlHandle( const TiXmlHandle& ref )           { this->node = ref.node; }
    TiXmlHandle operator=( const TiXmlHandle& ref ) { this->node = ref.node; return *this; }
    TiXmlHandle FirstChild() const;
    TiXmlHandle FirstChild( const char * value ) const;
    TiXmlHandle FirstChildElement() const;
    TiXmlHandle FirstChildElement( const char * value ) const;
    TiXmlHandle Child( const char* value, int index ) const;
    TiXmlHandle Child( int index ) const;
    TiXmlHandle ChildElement( const char* value, int index ) const;
    TiXmlHandle ChildElement( int index ) const;
    #ifdef TIXML_USE_STL
    TiXmlHandle FirstChild( const std::string& _value ) const               { return FirstChild( _value.c_str() ); }
    TiXmlHandle FirstChildElement( const std::string& _value ) const        { return FirstChildElement( _value.c_str() ); }
    TiXmlHandle Child( const std::string& _value, int index ) const         { return Child( _value.c_str(), index ); }
    TiXmlHandle ChildElement( const std::string& _value, int index ) const  { return ChildElement( _value.c_str(), index ); }
    #endif
    TiXmlNode* Node() const         { return node; }
    TiXmlElement* Element() const   { return ( ( node && node->ToElement() ) ? node->ToElement() : 0 ); }
    TiXmlText* Text() const         { return ( ( node && node->ToText() ) ? node->ToText() : 0 ); }
    TiXmlUnknown* Unknown() const           { return ( ( node && node->ToUnknown() ) ? node->ToUnknown() : 0 ); }
private:
    TiXmlNode* node;
};
#endif