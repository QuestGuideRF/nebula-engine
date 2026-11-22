#pragma once
#include "core/types.h"
#include "coregraphics/indextype.h"
#include "coregraphics/gpubuffertypes.h"
#include "coregraphics/vertexlayout.h"
namespace CoreGraphics
{
#define NEBULA_NVX_MAGICNUMBER 'NVX3'
#pragma pack(push, 1)
struct Nvx3Header
{
    uint magic;
    uint meshDataOffset;
    uint numMeshes;
    uint meshletDataOffset;
    uint numMeshlets;
    uint vertexDataOffset;
    uint vertexDataSize;
    uint indexDataOffset;
    uint indexDataSize;
};
struct Nvx3VertexRange
{
    uint indexByteOffset;
    uint baseVertexByteOffset;
    uint attributesVertexByteOffset;
    uint firstGroupOffset;
    uint numGroups;
    CoreGraphics::IndexType::Code indexType;
    CoreGraphics::VertexLayoutType layout;
};
struct Nvx3Group
{
    uint primType;
    uint firstIndex;
    uint numIndices;
    uint firstMeshlet;
    uint numMeshlets;
};
struct Nvx3Meshlet
{
    uint indexOffset;
    uint firstIndex;
    uint numIndices;
};
struct Nvx3Elements
{
    CoreGraphics::Nvx3VertexRange* ranges;
    ubyte* vertexData;
    ubyte* indexData;
    CoreGraphics::Nvx3Meshlet* meshlets;
};
namespace Nvx3
{
    void FillNvx3Elements(char* basePointer, Nvx3Header* header, Nvx3Elements& OutElements);
};
#pragma pack(pop)
namespace Nvx3
{
inline void
FillNvx3Elements(char* basePointer, Nvx3Header* header, Nvx3Elements& OutElements)
{
    n_assert(header != nullptr);
    OutElements.ranges = (CoreGraphics::Nvx3VertexRange*)(basePointer + header->meshDataOffset);
    OutElements.vertexData = (ubyte*)(basePointer + header->vertexDataOffset);
    OutElements.indexData = (ubyte*)(basePointer + header->indexDataOffset);
    OutElements.meshlets = (CoreGraphics::Nvx3Meshlet*)(basePointer + header->meshletDataOffset);
}
}
}