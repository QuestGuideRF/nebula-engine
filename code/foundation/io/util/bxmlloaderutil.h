#pragma once
#include "core/types.h"
#include "io/stream.h"
#include "io/util/bxmlfilestructs.h"
#include "util/array.h"
namespace IO
{
class BXmlLoaderUtil
{
public:
    static const ushort InvalidNodeIndex = 0xffff;
    static const uint InvalidAttrIndex = 0xffffffff;
    BXmlLoaderUtil();
    ~BXmlLoaderUtil();
    void SetupFromFileInMemory(void* buf, Stream::Size size);
    void Discard();
    bool IsValid() const;
    ushort FindNodeIndex(const char* path) const;
    void SetCurrentNodeIndex(ushort index);
    ushort GetCurrentNodeIndex() const;
    const char* GetCurrentNodeName() const;
    bool SetToFirstChild(const char* name);
    bool SetToNextChild(const char* name);
    bool SetToParent();
    uint FindAttrIndex(const char* attrName) const;
    uint GetNumAttrs() const;
    const char* GetAttrName(uint attrIndex) const;
    const char* GetAttrValue(uint attrIndex) const;
    ushort FindChildNodeIndex(ushort nodeIndex, const char* childName) const;
    ushort FindSiblingNodeIndex(ushort nodeIndex, const char* sibName) const;
    ushort GetParentNodeIndex(ushort nodeIndex) const;
    const char* GetNodeName(ushort nodeIndex) const;
private:
    void* buffer;
    Stream::Size bufSize;
    BXMLFileHeader* header;
    BXMLFileAttr* attrs;
    BXMLFileNode* nodes;
    Util::Array<const char*> stringTable;
    ushort curNodeIndex;
};
inline bool
BXmlLoaderUtil::IsValid() const
{
    return (0 != this->buffer);
}
inline void
BXmlLoaderUtil::SetCurrentNodeIndex(ushort i)
{
    n_assert((0 != this->header) && (i < this->header->numNodes));
    this->curNodeIndex = i;
}
inline ushort
BXmlLoaderUtil::GetCurrentNodeIndex() const
{
    return this->curNodeIndex;
}
inline const char*
BXmlLoaderUtil::GetCurrentNodeName() const
{
    n_assert(InvalidNodeIndex != this->curNodeIndex);
    return this->stringTable[this->nodes[this->curNodeIndex].nameIndex];
}
}