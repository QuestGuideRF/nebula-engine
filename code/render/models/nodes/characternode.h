#pragma once
#include "models/nodes/transformnode.h"
#include "coregraphics/shader.h"
#include "coregraphics/buffer.h"
namespace Models
{
class CharacterNode : public Models::TransformNode
{
public:
    CharacterNode();
    virtual ~CharacterNode();
    void OnResourcesLoaded();
    bool Load(const Util::FourCC& fourcc, const Util::StringAtom& tag, const Ptr<IO::BinaryReader>& reader, bool immediate) override;
    void Discard() override;
    const Resources::ResourceName& GetAnimResource() const;
    const Resources::ResourceId GetAnimationResourceId() const;
    bool GetImplicitHierarchyActivation() const override;
private:
protected:
    Resources::ResourceName animResId;
    IndexT animIndex;
    Resources::ResourceName skeletonResId;
    IndexT skeletonIndex;
    Resources::ResourceName variationResId;
    Util::StringAtom tag;
    Resources::ResourceId managedAnimResource;
    Resources::ResourceId managedVariationResource;
    CoreGraphics::ShaderId sharedShader;
    CoreGraphics::BufferId cbo;
    IndexT cboIndex;
};
inline const Resources::ResourceName&
CharacterNode::GetAnimResource() const
{
    return this->animResId;
}
inline const Resources::ResourceId
CharacterNode::GetAnimationResourceId() const
{
    return this->managedAnimResource;
}
inline bool
CharacterNode::GetImplicitHierarchyActivation() const
{
    return true;
}
}