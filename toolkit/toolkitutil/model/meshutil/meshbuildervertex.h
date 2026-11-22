#pragma once
#include "core/types.h"
#include "math/vec4.h"
#include "math/mat4.h"
#include "math/half.h"
#include "coregraphics/vertexlayout.h"
namespace ToolkitUtil
{
class MeshBuilderVertex
{
public:
    enum Components
    {
        Position = 0x0,
        Uvs = 0x1,
        Normals = 0x2,
        Tangents = 0x4,
        Color = 0x8,
        SecondUv = 0x10,
        SkinWeights = 0x20,
        SkinIndices = 0x40
    };
    typedef uint ComponentMask;
    enum Flag
    {
        Redundant = (1<<0),
    };
    typedef uint FlagMask;
    MeshBuilderVertex();
    void SetComponents(const ComponentMask componentMask);
    static const CoreGraphics::VertexLayoutType GetVertexLayoutType(const ComponentMask componentMask);
    static const SizeT GetSize(const ComponentMask componentMask);
    void SetPosition(const Math::vec4& pos);
    void SetUv(const Math::vec2& uv);
    void SetNormal(const Math::vec3& n);
    void SetTangent(const Math::vec3& t);
    void SetSign(const float f);
    void SetColor(const Math::vec4& c);
    void SetSecondaryUv(const Math::vec2& uv);
    void SetSkinWeights(const Math::vec4& w);
    void SetSkinIndices(const Math::uint4& indices);
    bool operator==(const MeshBuilderVertex& rhs) const;
    bool operator!=(const MeshBuilderVertex& rhs) const;
    bool operator<(const MeshBuilderVertex& rhs) const;
    bool operator>(const MeshBuilderVertex& rhs) const;
    void SetFlag(Flag f);
    void UnsetFlag(Flag f);
    bool CheckFlag(Flag f) const;
    int Compare(const MeshBuilderVertex& rhs) const;
    void Transform(const Math::mat4& m);
private:
    friend class MeshBuilder;
    friend class MeshBuilderSaver;
    friend class SkinPartitioner;
    friend class SkinFragment;
    friend class NFbxScene;
    friend class Scene;
    ComponentMask componentMask;
    FlagMask flagMask;
    struct VertexBase
    {
        Math::vec4 position;
        Math::vec2 uv;
    } base;
    struct VertexAttributes
    {
        struct Normal
        {
            Math::vec3 normal, tangent;
            float sign;
        } normal;
        struct SecondUV
        {
            Math::vec2 uv2;
        } secondUv;
        struct Color
        {
            Math::vec4 color;
        } color;
        struct Skin
        {
            Math::vec4 weights;
            Math::uint4 indices;
            Math::uint4 remapIndices;
        } skin;
    } attributes;
};
inline void
MeshBuilderVertex::SetComponents(const ComponentMask components)
{
    this->componentMask = components;
}
inline const CoreGraphics::VertexLayoutType
MeshBuilderVertex::GetVertexLayoutType(const ComponentMask componentMask)
{
    if (OnlyBits(componentMask,
                MeshBuilderVertex::Components::Position
                | MeshBuilderVertex::Components::Uvs
                | MeshBuilderVertex::Components::Normals
                | MeshBuilderVertex::Components::Tangents))
        return CoreGraphics::VertexLayoutType::Normal;
    else if (OnlyBits(componentMask,
                MeshBuilderVertex::Components::Position
                | MeshBuilderVertex::Components::Uvs
                | MeshBuilderVertex::Components::Normals
                | MeshBuilderVertex::Components::Tangents
                | MeshBuilderVertex::Components::Color))
        return CoreGraphics::VertexLayoutType::Colors;
    else if (OnlyBits(componentMask,
                MeshBuilderVertex::Components::Position
                | MeshBuilderVertex::Components::Uvs
                | MeshBuilderVertex::Components::Normals
                | MeshBuilderVertex::Components::Tangents
                | MeshBuilderVertex::Components::SkinIndices
                | MeshBuilderVertex::Components::SkinWeights))
        return CoreGraphics::VertexLayoutType::Skin;
    else if (OnlyBits(componentMask,
                MeshBuilderVertex::Components::Position
                | MeshBuilderVertex::Components::Uvs
                | MeshBuilderVertex::Components::Normals
                | MeshBuilderVertex::Components::Tangents
                | MeshBuilderVertex::Components::SecondUv))
        return CoreGraphics::VertexLayoutType::SecondUV;
    else
        return CoreGraphics::VertexLayoutType::Invalid;
}
inline const SizeT
MeshBuilderVertex::GetSize(const ComponentMask componentMask)
{
    SizeT ret = 0;
    CoreGraphics::VertexLayoutType type = GetVertexLayoutType(componentMask);
    switch (type)
    {
        case CoreGraphics::VertexLayoutType::Normal:
            ret += sizeof(CoreGraphics::NormalVertex);
            break;
        case CoreGraphics::VertexLayoutType::Colors:
            ret += sizeof(CoreGraphics::ColorVertex);
            break;
        case CoreGraphics::VertexLayoutType::Skin:
            ret += sizeof(CoreGraphics::SkinVertex);
            break;
        case CoreGraphics::VertexLayoutType::SecondUV:
            ret += sizeof(CoreGraphics::SecondUVVertex);
            break;
    }
    return ret;
}
inline void
MeshBuilderVertex::SetPosition(const Math::vec4& pos)
{
    this->base.position = pos;
}
inline void
MeshBuilderVertex::SetUv(const Math::vec2& uv)
{
    this->base.uv = uv;
}
inline void
MeshBuilderVertex::SetNormal(const Math::vec3& n)
{
    this->attributes.normal.normal = n;
}
inline void
MeshBuilderVertex::SetTangent(const Math::vec3& t)
{
    this->attributes.normal.tangent = t;
}
inline void
MeshBuilderVertex::SetSign(const float f)
{
    this->attributes.normal.sign = f;
}
inline void
MeshBuilderVertex::SetColor(const Math::vec4& c)
{
    this->attributes.color.color = c;
}
inline void
MeshBuilderVertex::SetSecondaryUv(const Math::vec2& uv)
{
    this->attributes.secondUv.uv2 = uv;
}
inline void
MeshBuilderVertex::SetSkinWeights(const Math::vec4& w)
{
    this->attributes.skin.weights = w;
}
inline void
MeshBuilderVertex::SetSkinIndices(const Math::uint4& indices)
{
    this->attributes.skin.indices = indices;
}
inline void
MeshBuilderVertex::SetFlag(Flag f)
{
    this->flagMask |= f;
}
inline void
MeshBuilderVertex::UnsetFlag(Flag f)
{
    this->flagMask &= ~f;
}
inline bool
MeshBuilderVertex::CheckFlag(Flag f) const
{
    return (0 != (this->flagMask & f));
}
}