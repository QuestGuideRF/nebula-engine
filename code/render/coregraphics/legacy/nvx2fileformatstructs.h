#pragma once
#include "core/types.h"
namespace CoreGraphics
{
#pragma pack(push, 1)
#define NEBULA_NVX2_MAGICNUMBER 'NVX2'
struct Nvx2Header
{
    uint magic;
    uint numGroups;
    uint numVertices;
    uint vertexWidth;
    uint numIndices;
    uint numEdges;
    uint vertexComponentMask;
};
struct Nvx2Group
{
    uint firstVertex;
    uint numVertices;
    uint firstTriangle;
    uint numTriangles;
    uint firstEdge;
    uint numEdges;
};
#pragma pack(pop)
}