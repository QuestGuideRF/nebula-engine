#pragma once
#include "core/types.h"
namespace IO
{
#pragma pack(push,1)
struct BXMLFileHeader
{
    uint magic;
    uint numAttrs;
    uint numNodes;
    uint numStrings;
};
struct BXMLFileAttr
{
    ushort nameIndex;
    ushort valueIndex;
};
struct BXMLFileNode
{
    ushort nameIndex;
    ushort firstChildIndex;
    ushort nextSiblingIndex;
    ushort parentIndex;
    uint attrIndex;
    uint numAttrs;
};
#pragma pack(pop)
}