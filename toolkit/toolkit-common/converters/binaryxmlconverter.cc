//------------------------------------------------------------------------------
//  binaryxmlconverter.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "binaryxmlconverter.h"
#include "io/ioserver.h"
#include "io/binarywriter.h"

namespace ToolkitUtil
{
using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
BinaryXmlConverter::BinaryXmlConverter() :
    platform(Platform::Win32)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
BinaryXmlConverter::~BinaryXmlConverter()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Процедура преобразования верхнего уровня, рекурсивно конвертирующая файлы XML из
    исходный каталог в целевой каталог.
*/
bool
BinaryXmlConverter::ConvertDir(const Util::String& srcDir, const Util::String& dstDir, Logger& logger)
{
    IoServer* ioServer = IoServer::Instance();
    this->srcDir = srcDir;
    this->dstDir = dstDir;

    // убедитесь, что каталоги действительны
    if (!this->srcDir.IsValid())
    {
        logger.Error("Исходный каталог не установлен!\n");
        return false;
    }
    if (!this->dstDir.IsValid())
    {
        logger.Error("Целевой каталог не установлен!\n");
        return false;
    }
    if (!ioServer->DirectoryExists(this->srcDir))
    {
        logger.Error("Исходный каталог «%s» не существует!\n", this->srcDir.AsCharPtr());
        return false;
    }

    // рекурсивно конвертировать XML-файлы
    if (!this->RecurseConvert(this->srcDir, this->dstDir, logger))
    {
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
BinaryXmlConverter::RecurseConvert(const String& sourceDir, const String& destDir, Logger& logger)
{
    IoServer* ioServer = IoServer::Instance();

    // убедитесь, что каталог назначения существует
    if (!ioServer->DirectoryExists(destDir))
    {
        if (!ioServer->CreateDirectory(destDir))
        {
            logger.Error("Не удалось создать целевой каталог «%s»!\n", destDir.AsCharPtr());
            return false;
        }
    }

    // for each XML file...
    Array<String> srcFiles = ioServer->ListFiles(sourceDir, "*.xml");
    IndexT i;
    for (i = 0; i < srcFiles.Size(); i++)
    {
        String srcFile = sourceDir + "/" + srcFiles[i];
        String dstFile = destDir + "/" + srcFiles[i];
        dstFile.StripFileExtension();
        dstFile.Append(".bxml");
        if (!this->ConvertFile(srcFile, dstFile, logger))
        {
            // продолжить, даже если один файл не удалось преобразовать
            logger.Error("НЕ удалось ПРЕОБРАЗОВАТЬ '%s'!!!\n", srcFile.AsCharPtr());
        }
    }

    // рекурсия в подкаталог
    Array<String> dirs = ioServer->ListDirectories(sourceDir, "*");
    for (i = 0; i < dirs.Size(); i++)
    {
        if ((dirs[i] != ".svn") && (dirs[i] != "CVS"))
        {
            String srcSubDir = sourceDir + "/" + dirs[i];
            String dstSubDir = destDir + "/" + dirs[i];
            if (!this->RecurseConvert(srcSubDir, dstSubDir, logger))
            {
                return false;
            }
        }
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
BinaryXmlConverter::ConvertFile(const String& srcFile, const String& dstFile, Logger& logger)
{
    if (!this->LoadFile(srcFile, logger))
    {   
        return false;
    }
    if (!this->ValidateInternalRepresentation(srcFile, logger))
    {
        return false;
    }
    this->BuildFileArrays();
    if (!this->SaveFile(dstFile, logger))
    {
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
BinaryXmlConverter::ConvertStream(const Ptr<IO::MemoryStream>& stream, const Util::String& dstFile, Logger& logger)
{
    Ptr<XmlReader> xmlReader = XmlReader::Create();
    xmlReader->SetStream(stream);
    if (!xmlReader->Open())
    {
        return false;
    }

    // рекурсивно анализировать содержимое XML
    if (!this->RecurseParseXml(xmlReader, this->rootNode, logger))
    {
        logger.Error("Ошибка анализа XML-потока!\n");
        return false;
    }
    xmlReader->Close();

    this->BuildFileArrays();
    if (!this->SaveFile(dstFile, logger))
    {
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
BinaryXmlConverter::ClearUniqueStrings()
{
    this->stringTable.Clear();
}

//------------------------------------------------------------------------------
/**
*/
void
BinaryXmlConverter::AddUniqueString(const StringAtom& str)
{
    if (InvalidIndex == this->stringTable.BinarySearchIndex(str))
    {
        this->stringTable.InsertSorted(str);
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
BinaryXmlConverter::LoadFile(const String& srcFile, Logger& logger)
{
    // очистить таблицу уникальных строк
    this->ClearUniqueStrings();

    // файл с открытым исходным кодом в формате XML
    IoServer* ioServer = IoServer::Instance();
    Ptr<Stream> stream = ioServer->CreateStream(srcFile);
    Ptr<XmlReader> xmlReader = XmlReader::Create();
    xmlReader->SetStream(stream);
    if (!xmlReader->Open())
    {
        logger.Error("Не удалось открыть исходный файл в формате XML: '%s'\n", srcFile.AsCharPtr());
        return false;
    }

    // рекурсивно анализировать содержимое XML
    if (!this->RecurseParseXml(xmlReader, this->rootNode, logger))
    {
        logger.Error("Ошибка анализа XML-файла '%s'!\n", srcFile.AsCharPtr());
        return false;
    }

    // success
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
BinaryXmlConverter::RecurseParseXml(const Ptr<XmlReader>& xmlReader, Node& node, Logger& logger)
{
    // проанализировать имя узла
    node.name = xmlReader->GetCurrentNodeName();
    node.attrs.Clear();
    node.childNodes.Clear();
    this->AddUniqueString(node.name);

    // анализировать атрибуты узла
    Array<String> attrs = xmlReader->GetAttrs();
    if (attrs.Size() > 0)
    {
        IndexT attrIndex;
        node.attrs.Reserve(attrs.Size());
        for (attrIndex = 0; attrIndex < attrs.Size(); attrIndex++)
        {
            Attr attr;
            attr.name  = attrs[attrIndex];
            attr.value = xmlReader->GetString(attrs[attrIndex].AsCharPtr());
            this->AddUniqueString(attr.name);
            this->AddUniqueString(attr.value);
            node.attrs.Append(attr);
        }
    }

    // анализировать дочерние узлы
    if (xmlReader->SetToFirstChild()) do
    {
        Node newNode;
        node.childNodes.Append(newNode);
        if (!this->RecurseParseXml(xmlReader, node.childNodes.Back(), logger))
        {
            return false;
        }
    }
    while (xmlReader->SetToNextChild());

    // done
    return true;
}

//------------------------------------------------------------------------------
/**
*/
SizeT
BinaryXmlConverter::RecurseCountAttrs(const Node& node, SizeT numAttrs)
{
    numAttrs += node.attrs.Size();
    IndexT i;
    for (i = 0; i < node.childNodes.Size(); i++)
    {
        numAttrs = this->RecurseCountAttrs(node.childNodes[i], numAttrs);
    }
    return numAttrs;
}

//------------------------------------------------------------------------------
/**
*/
SizeT
BinaryXmlConverter::RecurseCountNodes(const Node& node, SizeT numNodes)
{
    numNodes += node.childNodes.Size();
    IndexT i;
    for (i = 0; i < node.childNodes.Size(); i++)
    {
        numNodes = this->RecurseCountNodes(node.childNodes[i], numNodes);
    }
    return numNodes;
}

//------------------------------------------------------------------------------
/**
*/
bool
BinaryXmlConverter::ValidateInternalRepresentation(const String& srcFile, Logger& logger)
{
    if (this->stringTable.Size() >= 65536)
    {
        logger.Error("Слишком много уникальных строк (>=65536) в '%s'!\n", srcFile.AsCharPtr());
        return false;
    }
    SizeT numNodes = this->RecurseCountNodes(this->rootNode, 0);
    if (numNodes >= 65535)
    {
        logger.Error("Слишком много узлов XML в '%s' (должно быть < 65535)!\n", srcFile.AsCharPtr());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
BinaryXmlConverter::BuildFileArrays()
{
    this->fileAttrArray.Clear();
    this->fileNodeArray.Clear();
    this->RecurseAddFileNode(this->rootNode, InvalidNodeIndex, InvalidNodeIndex);
}

//------------------------------------------------------------------------------
/**
*/
ushort
BinaryXmlConverter::RecurseAddFileNode(const Node& node, ushort prevSiblingIndex, ushort parentNodeIndex)
{
    // создайте новый BXMLNode и добавьте его в массив узлов
    BXMLFileNode bxmlNode;
    bxmlNode.nameIndex = this->stringTable.BinarySearchIndex(node.name);
    bxmlNode.firstChildIndex = InvalidNodeIndex;
    bxmlNode.nextSiblingIndex = InvalidNodeIndex;
    bxmlNode.parentIndex = parentNodeIndex;
    if (node.attrs.Size() > 0)
    {
        bxmlNode.attrIndex = this->fileAttrArray.Size();
        bxmlNode.numAttrs = node.attrs.Size();
    }
    else
    {
        bxmlNode.attrIndex = InvalidAttrIndex;
        bxmlNode.numAttrs = 0;
    }
    ushort myNodeIndex = this->fileNodeArray.Size();
    this->fileNodeArray.Append(bxmlNode);

    // узлы, зависящие от патча (родительский или предыдущий брат
    if (prevSiblingIndex != InvalidNodeIndex)
    {
        this->fileNodeArray[prevSiblingIndex].nextSiblingIndex = myNodeIndex;
    }
    else if (parentNodeIndex != InvalidNodeIndex)
    {
        this->fileNodeArray[parentNodeIndex].firstChildIndex = myNodeIndex;
    }

    // добавить атрибуты
    IndexT attrIndex;
    for (attrIndex = 0; attrIndex < node.attrs.Size(); attrIndex++)
    {
        BXMLFileAttr bxmlAttr;
        bxmlAttr.nameIndex = this->stringTable.BinarySearchIndex(node.attrs[attrIndex].name);
        bxmlAttr.valueIndex = this->stringTable.BinarySearchIndex(node.attrs[attrIndex].value);
        this->fileAttrArray.Append(bxmlAttr);
    }

    // рекурсия на дочерние узлы...
    ushort prevChildIndex = InvalidNodeIndex;
    IndexT childIndex;
    for (childIndex = 0; childIndex < node.childNodes.Size(); childIndex++)
    {
        prevChildIndex = this->RecurseAddFileNode(node.childNodes[childIndex], prevChildIndex, myNodeIndex);
    }

    // return my own node index
    return myNodeIndex;
}
 
//------------------------------------------------------------------------------
/**
*/
bool
BinaryXmlConverter::SaveFile(const String& dstFile, Logger& logger)
{
    Ptr<Stream> stream = IoServer::Instance()->CreateStream(dstFile);
    Ptr<BinaryWriter> writer = BinaryWriter::Create();
    writer->SetStream(stream);
    writer->SetStreamByteOrder(Platform::GetPlatformByteOrder(this->platform));
    if (writer->Open())
    {
        // написать заголовок
        BXMLFileHeader header = { 0 };
        header.magic = 'BXML';
        header.numAttrs = this->fileAttrArray.Size();
        header.numNodes = this->fileNodeArray.Size();
        header.numStrings = this->stringTable.Size();
        writer->WriteUInt(header.magic);
        writer->WriteUInt(header.numAttrs);
        writer->WriteUInt(header.numNodes);
        writer->WriteUInt(header.numStrings);

        // записать массив атрибутов
        IndexT attrIndex;
        for (attrIndex = 0; attrIndex < this->fileAttrArray.Size(); attrIndex++)
        {
            const BXMLFileAttr& attr = this->fileAttrArray[attrIndex];
            writer->WriteUShort(attr.nameIndex);
            writer->WriteUShort(attr.valueIndex);
        }

        // записать массив узлов
        IndexT nodeIndex;
        for (nodeIndex = 0; nodeIndex < this->fileNodeArray.Size(); nodeIndex++)
        {
            const BXMLFileNode& node = this->fileNodeArray[nodeIndex];
            writer->WriteUShort(node.nameIndex);
            writer->WriteUShort(node.firstChildIndex);
            writer->WriteUShort(node.nextSiblingIndex);
            writer->WriteUShort(node.parentIndex);
            writer->WriteUInt(node.attrIndex);
            writer->WriteUInt(node.numAttrs);
        }

        // наконец, напишите таблицу строк
        IndexT strIndex;
        for (strIndex = 0; strIndex < this->stringTable.Size(); strIndex++)
        {
            const char* strPtr = this->stringTable[strIndex].Value();
            writer->WriteRawData(strPtr, String::StrLen(strPtr) + 1);
        }

        // сделанный!
        writer->Close();
    }
    return true;
}

} // пространство имен ToolkitUtil