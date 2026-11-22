//------------------------------------------------------------------------------
//  bxmlloaderutil.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/util/bxmlloaderutil.h"
#include "util/string.h"

namespace IO
{
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
BXmlLoaderUtil::BXmlLoaderUtil() :
    buffer(0),
    bufSize(0),
    header(0),
    attrs(0),
    nodes(0),
    curNodeIndex(InvalidNodeIndex)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
BXmlLoaderUtil::~BXmlLoaderUtil()
{
    // нам не принадлежит буферная память, поэтому мы ничего не освобождаем
}

//------------------------------------------------------------------------------
/**
*/
void
BXmlLoaderUtil::SetupFromFileInMemory(void* buf, Stream::Size size)
{
    n_assert(!this->IsValid());
    n_assert(0 != buf);
    n_assert(size_t(size) > sizeof(BXMLFileHeader));
    this->buffer = buf;
    this->bufSize = size;

    // установка указателей в буфер
    ubyte* ubPtr = (ubyte*) buf;
    this->header = (BXMLFileHeader*) ubPtr;
    n_assert(this->header->magic == 'BXML');
    n_assert(this->header->numStrings > 0);

    ubPtr += sizeof(BXMLFileHeader);

    this->attrs = (BXMLFileAttr*) ubPtr;    
    ubPtr += sizeof(BXMLFileAttr) * this->header->numAttrs;

    this->nodes = (BXMLFileNode*) ubPtr;
    ubPtr += sizeof(BXMLFileNode) * this->header->numNodes;

    // настройка таблицы строк
    this->stringTable.Reserve(this->header->numStrings);
    uint i;
    for (i = 0; i < this->header->numStrings; i++)
    {
        this->stringTable.Append((const char*)ubPtr);
        ubPtr += String::StrLen((const char*)ubPtr) + 1;
    }

    // установить индекс текущего узла в корневой узел
    this->curNodeIndex = 0;
}

//------------------------------------------------------------------------------
/**
*/
void
BXmlLoaderUtil::Discard()
{
    n_assert(this->IsValid());
    
    // ПРИМЕЧАНИЕ. Мы не владеем буфером, поэтому не освобождайте его.
    this->buffer = 0;
    this->bufSize = 0;
    this->header = 0;
    this->attrs = 0;
    this->nodes = 0;
    this->stringTable.Clear();
    this->curNodeIndex = InvalidNodeIndex;
}

//------------------------------------------------------------------------------
/**
    ПРИМЕЧАНИЕ. childName может иметь значение 0 или указывать на пустую строку.
*/
ushort
BXmlLoaderUtil::FindChildNodeIndex(ushort nodeIndex, const char* childName) const
{
    n_assert((nodeIndex < this->header->numNodes) && (0 != childName));
    ushort childNodeIndex = this->nodes[nodeIndex].firstChildIndex;
    if (InvalidNodeIndex != childNodeIndex) do
    {
        const char* curChildName = this->stringTable[this->nodes[childNodeIndex].nameIndex];
        if ((0 == childName[0]) || (0 == String::StrCmp(curChildName, childName)))
        {
            return childNodeIndex;
        }
        childNodeIndex = this->nodes[childNodeIndex].nextSiblingIndex;
    }
    while (InvalidNodeIndex != childNodeIndex);
    
    // провал: не найден
    return InvalidNodeIndex;
}

//------------------------------------------------------------------------------
/**
    ПРИМЕЧАНИЕ. childName может иметь значение 0 или указывать на пустую строку.
*/
ushort
BXmlLoaderUtil::FindSiblingNodeIndex(ushort nodeIndex, const char* sibName) const
{
    n_assert((nodeIndex < this->header->numNodes) && (0 != sibName));
    ushort sibNodeIndex = this->nodes[nodeIndex].nextSiblingIndex;
    if (InvalidNodeIndex != sibNodeIndex) do
    {
        const char* curSibName = this->stringTable[this->nodes[sibNodeIndex].nameIndex];
        if ((0 == sibName[0]) || (0 == String::StrCmp(curSibName, sibName)))
        {
            return sibNodeIndex;
        }
        sibNodeIndex = this->nodes[sibNodeIndex].nextSiblingIndex;
    }
    while (InvalidNodeIndex != sibNodeIndex);
    
    // провал: не найден
    return InvalidNodeIndex;
}

//------------------------------------------------------------------------------
/**
*/
ushort
BXmlLoaderUtil::GetParentNodeIndex(ushort nodeIndex) const
{
    n_assert(nodeIndex < this->header->numNodes);
    return this->nodes[nodeIndex].parentIndex;
}

//------------------------------------------------------------------------------
/**
*/
const char*
BXmlLoaderUtil::GetNodeName(ushort nodeIndex) const
{
    n_assert(nodeIndex < this->header->numNodes);
    return this->stringTable[this->nodes[nodeIndex].nameIndex];
}

//------------------------------------------------------------------------------
/**
*/
ushort
BXmlLoaderUtil::FindNodeIndex(const char* path) const
{
    n_assert(0 != path);
    
    // найти индекс начального узла
    bool isAbsPath = (path[0] == '/');
    ushort nodeIndex;
    if (isAbsPath)
    {
        nodeIndex = 0;
    }
    else
    {
        nodeIndex = this->curNodeIndex;
    }

    // ярлык, если путь на самом деле не путь, а одно имя узла
    if (0 == String::StrChr(path, '/'))
    {
        return this->FindChildNodeIndex(nodeIndex, path);
    }
    else
    {
        // if really a path, need to split into path tokens
        Array<String> tokens = String(path).Tokenize("/");

        // проверьте, действителен ли корневой узел
        if (isAbsPath && (tokens[0] != this->stringTable[this->nodes[0].nameIndex]))
        {
            return InvalidNodeIndex;
        }
        IndexT i = isAbsPath ? 1 : 0;
        SizeT num = tokens.Size();
        for (; i < num; i++)
        {
            const String& curToken = tokens[i];
            if (curToken == ".")
            {
                // ничего не делать
            }
            else if (curToken == "..")
            {
                // перейти в родительский каталог
                nodeIndex = this->nodes[nodeIndex].parentIndex;
                if (InvalidNodeIndex == nodeIndex)
                {
                    n_error("BXmlLoaderUtil::FindNode(%s): точки пути выше корневого узла!", path);
                    return InvalidNodeIndex;
                }
            }
            else
            {
                nodeIndex = this->FindChildNodeIndex(nodeIndex, curToken.AsCharPtr());
                if (InvalidNodeIndex == nodeIndex)
                {
                    return InvalidNodeIndex;
                }
            }
        }
        return nodeIndex;
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
BXmlLoaderUtil::SetToFirstChild(const char* name)
{
    n_assert(InvalidNodeIndex != this->curNodeIndex);
    ushort index = this->FindChildNodeIndex(this->curNodeIndex, name);
    if (InvalidNodeIndex != index)
    {
        this->curNodeIndex = index;
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
bool
BXmlLoaderUtil::SetToNextChild(const char* name)
{
    n_assert(InvalidNodeIndex != this->curNodeIndex);
    ushort index = this->FindSiblingNodeIndex(this->curNodeIndex, name);
    if (InvalidNodeIndex != index)
    {
        this->curNodeIndex = index;
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
*/
bool
BXmlLoaderUtil::SetToParent()
{
    n_assert(InvalidNodeIndex != this->curNodeIndex);
    ushort index = this->nodes[this->curNodeIndex].parentIndex;
    if (InvalidNodeIndex != index)
    {
        this->curNodeIndex = index;
        return true;
    }
    else
    {
        return false;
    }
}

//------------------------------------------------------------------------------
/**
    Returns a local attribute index by name, this is NOT an index
    into the global attributes array!
*/
uint
BXmlLoaderUtil::FindAttrIndex(const char* attrName) const
{
    n_assert((0 != attrName) && (InvalidNodeIndex != this->curNodeIndex));
    uint firstAttrIndex = this->nodes[this->curNodeIndex].attrIndex;
    uint numAttrs = this->nodes[this->curNodeIndex].numAttrs;
    uint i;
    for (i = 0; i < numAttrs; i++)
    {
        const char* curAttrName = this->stringTable[this->attrs[firstAttrIndex + i].nameIndex];
        if (0 == String::StrCmp(curAttrName, attrName))
        {
            return i;
        }
    }
    // провал: не найден
    return InvalidAttrIndex;
}

//------------------------------------------------------------------------------
/**
*/
uint
BXmlLoaderUtil::GetNumAttrs() const
{
    n_assert(InvalidNodeIndex != this->curNodeIndex);
    return this->nodes[this->curNodeIndex].numAttrs;
}

//------------------------------------------------------------------------------
/**
*/
const char*
BXmlLoaderUtil::GetAttrName(uint attrIndex) const
{
    n_assert(InvalidNodeIndex != this->curNodeIndex);
    uint globalAttrIndex = this->nodes[this->curNodeIndex].attrIndex + attrIndex;
    return this->stringTable[this->attrs[globalAttrIndex].nameIndex];
}

//------------------------------------------------------------------------------
/**
*/
const char*
BXmlLoaderUtil::GetAttrValue(uint attrIndex) const
{
    n_assert(InvalidNodeIndex != this->curNodeIndex);
    uint globalAttrIndex = this->nodes[this->curNodeIndex].attrIndex + attrIndex;
    return this->stringTable[this->attrs[globalAttrIndex].valueIndex];
}

} // пространство имен ввода-вывода
