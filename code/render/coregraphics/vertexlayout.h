#pragma once
#include "ids/id.h"
#include "coregraphics/vertexcomponent.h"
#include "resources/resourceid.h"
namespace CoreGraphics
{
RESOURCE_ID_TYPE(VertexLayoutId);
static const IndexT MaxNumVertexStreams = 16;
struct VertexLayoutCreateInfo
{
    Util::StringAtom name;
    Util::Array<VertexComponent> comps;
};
struct VertexLayoutInfo
{
    Util::StringAtom name;
    SizeT vertexByteSize;
    Util::Array<VertexComponent> comps;
};
const VertexLayoutId CreateVertexLayout(const VertexLayoutCreateInfo& info);
void DestroyVertexLayout(const VertexLayoutId id);
const SizeT VertexLayoutGetSize(const VertexLayoutId id);
const SizeT VertexLayoutGetStreamSize(const VertexLayoutId id, IndexT stream);
const Util::Array<VertexComponent>& VertexLayoutGetComponents(const VertexLayoutId id);
const Util::StringAtom& VertexLayoutGetName(const VertexLayoutId id);
enum class VertexLayoutType
{
    Invalid,
    Normal,
    Colors,
    SecondUV,
    Skin,
    Particle,
    NumTypes
};
#pragma pack(push, 1)
struct BaseVertex
{
    float position[3];
    short uv[2];
};
struct NormalVertex
{
    Math::byte4 normal;
    Math::byte4 tangent;
};
struct SecondUVVertex : NormalVertex
{
    ushort uv2[2];
};
struct ColorVertex : NormalVertex
{
    Math::byte4u color;
};
struct SkinVertex : NormalVertex
{
    float skinWeights[4];
    Math::byte4u skinIndices;
};
#pragma pack(pop)
}