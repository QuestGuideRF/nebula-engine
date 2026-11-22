#pragma once
#include "core/ptr.h"
#include "toolkit-common/platform.h"
#include "toolkit-common/logger.h"
#include "util/string.h"
#include "util/stringatom.h"
#include "io/xmlreader.h"
#include "io/memorystream.h"
#include "io/util/bxmlfilestructs.h"
namespace ToolkitUtil
{
class BinaryXmlConverter
{
public:
    BinaryXmlConverter();
    ~BinaryXmlConverter();
    void SetPlatform(Platform::Code platform);
    bool ConvertDir(const Util::String& srcDir, const Util::String& dstDir, Logger& logger);
    bool ConvertFile(const Util::String& srcFile, const Util::String& dstFile, Logger& logger);
    bool ConvertStream(const Ptr<IO::MemoryStream>& stream, const Util::String& dstFile, Logger& logger);
private:
    static const ushort InvalidNodeIndex = 0xffff;
    static const ushort InvalidAttrIndex = 0xffff;
    struct Attr
    {
        Util::StringAtom name;
        Util::StringAtom value;
    };
    struct Node
    {
        Util::StringAtom name;
        Util::Array<Attr> attrs;
        Util::Array<Node> childNodes;
    };
    bool RecurseConvert(const Util::String& srcDir, const Util::String& dstDir, Logger& logger);
    bool LoadFile(const Util::String& path, Logger& logger);
    bool ValidateInternalRepresentation(const Util::String& srcFile, Logger& logger);
    bool SaveFile(const Util::String& path, Logger& logger);
    void ClearUniqueStrings();
    void AddUniqueString(const Util::StringAtom& str);
    bool RecurseParseXml(const Ptr<IO::XmlReader>& xmlReader, Node& curNode, Logger& logger);
    SizeT RecurseCountAttrs(const Node& node, SizeT numAttrs);
    SizeT RecurseCountNodes(const Node& node, SizeT numNodes);
    void BuildFileArrays();
    ushort RecurseAddFileNode(const Node& node, ushort prevSiblingIndex, ushort parentNodeIndex);
    Platform::Code platform;
    Util::String srcDir;
    Util::String dstDir;
    Util::Array<Util::StringAtom> stringTable;
    Node rootNode;
    Util::Array<IO::BXMLFileAttr> fileAttrArray;
    Util::Array<IO::BXMLFileNode> fileNodeArray;
};
inline void
BinaryXmlConverter::SetPlatform(Platform::Code p)
{
    this->platform = p;
}
}