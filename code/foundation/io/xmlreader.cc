//------------------------------------------------------------------------------
//  xmlreader.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/xmlreader.h"
#if !__OSX__
#include "math/vec4.h"
#include "math/vec2.h"
#include "math/mat4.h"
#include "math/transform44.h"
#endif

namespace IO
{
__ImplementClass(IO::XmlReader, 'XMLR', IO::StreamReader);

// Этот статический объект устанавливает TinyXml при запуске приложения.
// (установите для конденсации пробелов значение false). Кажется, что все непросто,
// alternative way (see TinyXml docs for details)
XmlReader::TinyXmlInitHelper XmlReader::initTinyXml;

using namespace Util;
#if !__OSX__
using namespace Math;
#endif
    
//------------------------------------------------------------------------------
/**
*/
XmlReader::XmlReader() :
    xmlDocument(0),
    curNode(0)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
XmlReader::~XmlReader()
{
    if (this->IsOpen())
    {
        this->Close();
    }
}

//------------------------------------------------------------------------------
/**
    Opens the stream and reads the content of the stream into
    TinyXML.
*/
bool
XmlReader::Open()
{
    n_assert(0 == this->xmlDocument);
    n_assert(0 == this->curNode);

    if (StreamReader::Open())
    {
        // создать объект XML-документа
        this->xmlDocument = new TiXmlDocument;
        if (!this->xmlDocument->LoadStream(this->stream))
        {
            // не удалось проанализировать структуру XML, предоставьте отзыв
            const URI& uri = this->stream->GetURI();
            if (uri.IsValid())
            {
                n_error("XmlReader::Open(): не удалось открыть поток как XML '%s'\nОшибка TinyXML: %s!", uri.AsString().AsCharPtr(), this->xmlDocument->ErrorDesc());
            }
            else
            {
                n_error("XmlReader::Open(): не удалось открыть поток в формате XML (недопустимый URI)!\nОшибка TinyXML: %s", this->xmlDocument->ErrorDesc());
            }
            delete this->xmlDocument;
            this->xmlDocument = 0;
            return false;
        }

        // поскольку документ XML теперь загружен, мы можем закрыть исходный поток
        if (!this->streamWasOpen)
        {
            this->stream->Close();
        }

        // установить текущий узел в качестве корневого узла
        this->curNode = this->xmlDocument->RootElement();
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
XmlReader::Close()
{
    n_assert(0 != this->xmlDocument);

    // удалить XML-документ
    delete this->xmlDocument;
    this->xmlDocument = 0;
    this->curNode = 0;

    StreamReader::Close();
}

//------------------------------------------------------------------------------
/**
    Этот метод возвращает номер строки текущего узла.
*/
int
XmlReader::GetCurrentNodeLineNumber() const
{
    n_assert(this->curNode);
    return this->curNode->Row();
}

//------------------------------------------------------------------------------
/**
    This method finds an xml node by path name. It can handle absolute
    paths and paths relative to the current node. All the usual file system
    path conventions are valid: "/" is the path separator, "." is the
    current directory, ".." the parent directory.
*/
TiXmlNode*
XmlReader::FindNode(const String& path) const
{
    n_assert(this->IsOpen());
    n_assert(path.IsValid());

    bool absPath = (path[0] == '/');
    Array<String> tokens = path.Tokenize("/");

    // получить начальный узел (корневой или текущий узел)
    TiXmlNode* node;
    if (absPath)
    {
        node = this->xmlDocument;
    }
    else
    {
        n_assert(0 != this->curNode);
        node = this->curNode;
    }

    // перебирать компоненты пути
    int i;
    int num = tokens.Size();
    for (i = 0; i < num; i++)
    {
        const String& cur = tokens[i];
        if ("." == cur)
        {
            // ничего не делать
        }
        else if (".." == cur)
        {
            // перейти в родительский каталог
            node = node->Parent();
            if (node == this->xmlDocument)
            {
                n_error("XmlStream::FindNode(%s): точки пути выше корневого узла!", path.AsCharPtr());
                return 0;
            }
        }
        else
        {
            // найти дочерний узел
            node = node->FirstChild(cur.AsCharPtr());
            if (0 == node)
            {
                return 0;
            }
        }
    }
    return node;
}

//------------------------------------------------------------------------------
/**
    This method returns true if the node identified by path exists. Path
    follows the normal filesystem path conventions, "/" is the separator,
    ".." is the parent node, "." is the current node. An absolute path
    starts with a "/", a relative path doesn't.
*/
bool
XmlReader::HasNode(const String& n) const
{
    return (this->FindNode(n) != 0);
}

//------------------------------------------------------------------------------
/**
    Получите короткое имя (без пути) текущего узла.
*/
String
XmlReader::GetCurrentNodeName() const
{
    n_assert(this->IsOpen());
    n_assert(0 != this->curNode);
    return String(this->curNode->Value());
}

//------------------------------------------------------------------------------
/**
    This returns the full absolute path of the current node. Path components
    are separated by slashes.
*/
String
XmlReader::GetCurrentNodePath() const
{
    n_assert(this->IsOpen());
    n_assert(0 != this->curNode);

    // построить восходящий массив имен узлов
    Array<String> components;
    TiXmlNode* node = this->curNode;
    while (node != this->xmlDocument)
    {
        components.Append(node->Value());
        node = node->Parent();
    }

    // построить путь сверху вниз
    String path = "/";
    int i;
    for (i = components.Size() - 1; i >= 0; --i)
    {
        path.Append(components[i]);
        if (i > 0)
        {
            path.Append("/");
        }
    }
    return path;
}

//------------------------------------------------------------------------------
/**
    Сбрасывает средство чтения XML на корневой узел.
*/
void 
XmlReader::SetToRoot()
{
    // установить текущий узел в качестве корневого узла
    this->curNode = this->xmlDocument->RootElement();

    n_assert(this->curNode);
}

//------------------------------------------------------------------------------
/**
    Set the node pointed to by the path string as current node. The path
    may be absolute or relative, following the usual filesystem path 
    conventions. Separator is a slash.
*/
void
XmlReader::SetToNode(const String& path)
{
    n_assert(this->IsOpen());
    n_assert(path.IsValid());
    TiXmlNode* n = this->FindNode(path);
    if (n)
    {
        this->curNode = n->ToElement();
    }
    else
    {
        n_error("XmlReader::SetToNode(%s): найденный узел!", path.AsCharPtr());
    }
}

//------------------------------------------------------------------------------
/**
    Sets the current node to the first child node. If no child node exists,
    the current node will remain unchanged and the method will return false.
    If name is a valid string, only child element matching the name will 
    be returned. If name is empty, all child nodes will be considered.
*/
bool
XmlReader::SetToFirstChild(const String& name)
{
    n_assert(this->IsOpen());
    n_assert(0 != this->curNode);
    TiXmlElement* child = 0;
    if (name.IsEmpty())
    {
        child = this->curNode->FirstChildElement();
    }
    else
    {
        child = this->curNode->FirstChildElement(name.AsCharPtr());
    }
    if (child)
    {
        this->curNode = child;
        return true;
    }
    else
    {
        return false;
    }
}

//------------------------------------------------------------------------------
/**
    Sets the current node to the next sibling. If no more children exist,
    the current node will be reset to the parent node and the method will 
    return false. If name is a valid string, only child element matching the 
    name will be returned. If name is empty, all child nodes will be considered.
*/
bool
XmlReader::SetToNextChild(const String& name)
{
    n_assert(this->IsOpen());
    n_assert(0 != this->curNode);

    TiXmlElement* sib = 0;
    if (name.IsEmpty())
    {
        sib = this->curNode->NextSiblingElement();
    }
    else
    {
        sib = this->curNode->NextSiblingElement(name.AsCharPtr());
    }
    if (sib)
    {
        this->curNode = sib;
        return true;
    }
    else
    {
        this->SetToParent();
        return false;
    }
}

//------------------------------------------------------------------------------
/**
    Sets the current node to its parent. If no parent exists, the
    current node will remain unchanged and the method will return false.
*/
bool
XmlReader::SetToParent()
{
    n_assert(this->IsOpen());
    n_assert(0 != this->curNode);
    TiXmlNode* parent = this->curNode->Parent();
    if (parent)
    {
        this->curNode = parent->ToElement();
        return true;
    }
    else
    {
        return false;
    }
}

//------------------------------------------------------------------------------
/**
    Возвращайте true, если атрибут с заданным именем существует на текущем узле.
*/
bool
XmlReader::HasAttr(const char* name) const
{
    n_assert(this->IsOpen());
    n_assert(0 != this->curNode);
    n_assert(0 != name);
    return (0 != this->curNode->Attribute(name));
}

//------------------------------------------------------------------------------
/**
    Возвращает массив с именами всех атрибутов на текущем узле.
*/
Array<String>
XmlReader::GetAttrs() const
{
    n_assert(this->IsOpen());
    n_assert(0 != this->curNode);
    Array<String> res;
    const TiXmlAttribute * attr = this->curNode->FirstAttribute();
    while(0 != attr)
    {
        res.Append(attr->Name());
        attr = attr->Next();
    }
    return res;
}

//------------------------------------------------------------------------------
/**
    Return the provided attribute as string. If the attribute does not exist
    the method will fail hard (use HasAttr() to check for its existance).
*/
String
XmlReader::GetString(const char* name) const
{
    n_assert(this->IsOpen());
    n_assert(0 != this->curNode);
    n_assert(0 != name);
    String str;
    const char* val = this->curNode->Attribute(name);    
    if (0 == val)
    {
        n_error("XmlReader: атрибут «%s» не существует в %s на узле «%s»!", name, this->stream->GetURI().AsString().AsCharPtr(), this->curNode->Value());
    }
    else
    {
        str = val;
    }
    return str;
}

//------------------------------------------------------------------------------
/**
    Return the provided attribute as a bool. If the attribute does not exist
    the method will fail hard (use HasAttr() to check for its existance).
*/
bool
XmlReader::GetBool(const char* name) const
{
    return this->GetString(name).AsBool();
}

//------------------------------------------------------------------------------
/**
    Return the provided attribute as int. If the attribute does not exist
    the method will fail hard (use HasAttr() to check for its existance).
*/
int
XmlReader::GetInt(const char* name) const
{
    return this->GetString(name).AsInt();
}

//------------------------------------------------------------------------------
/**
    Return the provided attribute as float. If the attribute does not exist
    the method will fail hard (use HasAttr() to check for its existance).
*/
float
XmlReader::GetFloat(const char* name) const
{
    return this->GetString(name).AsFloat();
}

#if !__OSX__    
//------------------------------------------------------------------------------
/**
    Return the provided attribute as vec2. If the attribute does not exist
    the method will fail hard (use HasAttr() to check for its existance).
*/
vec2
XmlReader::GetVec2(const char* name) const
{
    return this->GetString(name).AsVec2();
}

//------------------------------------------------------------------------------
/**
    Return the provided attribute as vec4. If the attribute does not exist
    the method will fail hard (use HasAttr() to check for its existance).
*/
vec4
XmlReader::GetVec4(const char* name) const
{
#if NEBULA_XMLREADER_LEGACY_VECTORS
    const String& vec4String = this->GetString(name);
    Array<String> tokens = vec4String.Tokenize(", \t");
    if (tokens.Size() == 3)
    {
        return vec4(tokens[0].AsFloat(), tokens[1].AsFloat(), tokens[2].AsFloat(), 0);
    }    
#endif
    return this->GetString(name).AsVec4();
}

//------------------------------------------------------------------------------
/**
    Return the provided attribute as mat4. If the attribute does not exist
    the method will fail hard (use HasAttr() to check for its existance).
*/
mat4
XmlReader::GetMat4(const char* name) const
{
    return this->GetString(name).AsMat4();
}

//------------------------------------------------------------------------------
/**
Return the provided attribute as transform44. If the attribute does not exist
the method will fail hard (use HasAttr() to check for its existance).
*/
transform44
XmlReader::GetTransform44(const char* name) const
{
    return this->GetString(name).AsTransform44();
}
#endif
    
//------------------------------------------------------------------------------
/**
    Верните предоставленный необязательный атрибут в виде строки. Если атрибут не
    существуют, будет возвращено значение по умолчанию.
*/
String
XmlReader::GetOptString(const char* name, const String& defaultValue) const
{
    if (this->HasAttr(name))
    {
        return this->GetString(name);
    }
    else
    {
        return defaultValue;
    }
}

//------------------------------------------------------------------------------
/**
    Верните предоставленный необязательный атрибут как bool. Если атрибут не
    существуют, будет возвращено значение по умолчанию.
*/
bool
XmlReader::GetOptBool(const char* name, bool defaultValue) const
{
    if (this->HasAttr(name))
    {
        return this->GetBool(name);
    }
    else
    {
        return defaultValue;
    }
}

//------------------------------------------------------------------------------
/**
    Верните предоставленный необязательный атрибут как int. Если атрибут не
    существуют, будет возвращено значение по умолчанию.
*/
int
XmlReader::GetOptInt(const char* name, int defaultValue) const
{
    if (this->HasAttr(name))
    {
        return this->GetInt(name);
    }
    else
    {
        return defaultValue;
    }
}

//------------------------------------------------------------------------------
/**
    Верните предоставленный необязательный атрибут как число с плавающей запятой. Если атрибут не
    существуют, будет возвращено значение по умолчанию.
*/
float
XmlReader::GetOptFloat(const char* name, float defaultValue) const
{
    if (this->HasAttr(name))
    {
        return this->GetFloat(name);
    }
    else
    {
        return defaultValue;
    }
}

#if !__OSX__    
//------------------------------------------------------------------------------
/**
    Верните предоставленный необязательный атрибут как vec2. Если атрибут не
    существуют, будет возвращено значение по умолчанию.
*/
vec2
XmlReader::GetOptVec2(const char* name, const vec2& defaultValue) const
{
    if (this->HasAttr(name))
    {
        return this->GetVec2(name);
    }
    else
    {
        return defaultValue;
    }
}

//------------------------------------------------------------------------------
/**
    Верните предоставленный необязательный атрибут как vec4. Если атрибут не
    существуют, будет возвращено значение по умолчанию.
*/
vec4
XmlReader::GetOptVec4(const char* name, const vec4& defaultValue) const
{
    if (this->HasAttr(name))
    {
        return this->GetVec4(name);
    }
    else
    {
        return defaultValue;
    }
}

//------------------------------------------------------------------------------
/**
    Верните предоставленный необязательный атрибут как mat4. Если атрибут не
    существуют, будет возвращено значение по умолчанию.
*/
mat4
XmlReader::GetOptMat4(const char* name, const mat4& defaultValue) const
{
    if (this->HasAttr(name))
    {
        return this->GetMat4(name);
    }
    else
    {
        return defaultValue;
    }
}

//------------------------------------------------------------------------------
/**
Верните предоставленный необязательный атрибут как Transform44. Если атрибут не
существуют, будет возвращено значение по умолчанию.
*/
transform44
XmlReader::GetOptTransform44(const char* name, const transform44& defaultValue) const
{
    if (this->HasAttr(name))
    {
        return this->GetTransform44(name);
    }
    else
    {
        return defaultValue;
    }
}
#endif
    
//------------------------------------------------------------------------------
/**
*/
bool
XmlReader::HasContent() const
{
    n_assert(this->IsOpen());
    n_assert(0 != this->curNode);
    TiXmlNode* child = this->curNode->FirstChild();
    return child && (child->Type() == TiXmlNode::TEXT);
}

//------------------------------------------------------------------------------
/**
*/
String
XmlReader::GetContent() const
{
    n_assert(this->IsOpen());
    n_assert(this->curNode);
    TiXmlNode* child = this->curNode->FirstChild();
    n_assert(child->Type() == TiXmlNode::TEXT);
    return child->Value();
}

//------------------------------------------------------------------------------
/**
*/
bool
XmlReader::HasComment() const
{
    n_assert(this->IsOpen());
    n_assert(this->curNode);
    TiXmlNode* prevSibling = this->curNode->PreviousSibling();
    return prevSibling && (prevSibling->Type() == TiXmlNode::COMMENT);
}

//------------------------------------------------------------------------------
/**
*/
Util::String
XmlReader::GetComment() const
{
    n_assert(this->IsOpen());
    n_assert(this->curNode);
    TiXmlNode* prevSibling = this->curNode->PreviousSibling();
    n_assert(prevSibling->Type() == TiXmlNode::COMMENT);
    return prevSibling->Value();
}

} // пространство имен ввода-вывода
