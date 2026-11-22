#pragma once
#include "core/refcounted.h"
#include "model/n3util/n3modeldata.h"
#include "math/bbox.h"
#include "io/stream.h"
#include "particles/emitterattrs.h"
namespace ToolkitUtil
{
class ModelConstants : public Core::RefCounted
{
    __DeclareClass(ModelConstants);
public:
    struct TransformNode
    {
        Util::String name;
        Util::String path;
        float LODMax;
        float LODMin;
        bool useLOD = false;
        ToolkitUtil::Transform transform;
        Math::bbox boundingBox;
    };
    struct ShapeNode : public TransformNode
    {
        Util::String meshResource;
        IndexT meshIndex;
        IndexT primitiveGroupIndex;
    };
    struct SkinNode : public ShapeNode
    {
        Util::Array<IndexT> fragmentJoints;
    };
    struct SkinSetNode : TransformNode
    {
        Util::Array<SkinNode> skinFragments;
    };
    struct CharacterNode
    {
        Util::String skeletonResource;
        IndexT skeletonIndex;
        Util::String animationResource;
        IndexT animationIndex;
        Util::String name;
    };
    struct PhysicsNode : public TransformNode
    {
        Util::String mesh;
        Util::String material;
        IndexT primitiveGroupIndex;
    };
    struct ParticleNode : public ShapeNode
    {
    };
    ModelConstants();
    virtual ~ModelConstants();
    void AddCharacterNode(const Util::String& name, const ModelConstants::CharacterNode& node);
    const ModelConstants::CharacterNode& GetCharacterNode(const Util::String& name) const;
    const Util::Array<ModelConstants::CharacterNode> GetCharacterNodes() const;
    bool HasCharacterNode(const Util::String& name);
    void DeleteCharacterNode(const Util::String& name);
    void AddShapeNode(const ModelConstants::ShapeNode& node);
    const Util::Array<ModelConstants::ShapeNode>& GetShapeNodes() const;
    void AddSkinSetNode(const ModelConstants::SkinSetNode& skinSet);
    const Util::Array<ModelConstants::SkinSetNode>& GetSkinSetNodes() const;
    void AddParticleNode(const Util::String& name, const ModelConstants::ParticleNode& node);
    const ModelConstants::ParticleNode& GetParticleNode(const Util::String& name);
    const Util::Array<ModelConstants::ParticleNode> GetParticleNodes() const;
    void ReplaceParticleNode(const Util::String& name, const ModelConstants::ParticleNode& node);
    bool HasParticleNode(const Util::String& name);
    void DeleteParticleNode(const Util::String& name);
    void AddPhysicsNode(const Util::String& name, const ModelConstants::PhysicsNode& node);
    const ModelConstants::PhysicsNode& GetPhysicsNode(const Util::String& name);
    const Util::Array<ModelConstants::PhysicsNode> GetPhysicsNodes() const;
    bool HasPhysicsNode(const Util::String& name);
    void DeletePhysicsNode(const Util::String& name);
    void SetGlobalBoundingBox(const Math::bbox& bbox);
    const Math::bbox& GetGlobalBoundingBox() const;
    void SetName(const Util::String& name);
    const Util::String& GetName() const;
    void Clear();
    void Save(const Ptr<IO::Stream>& stream);
    void Load(const Ptr<IO::Stream>& stream);
private:
    Util::String name;
    Util::String checksum;
    Math::bbox globalBoundingBox;
    Util::Array<ModelConstants::ShapeNode> shapeNodes;
    Util::Dictionary<Util::String, ModelConstants::CharacterNode> characterNodes;
    Util::Dictionary<Util::String, ModelConstants::ParticleNode> particleNodes;
    Util::Dictionary<Util::String, ModelConstants::PhysicsNode> physicsNodes;
    Util::Array<SkinSetNode> skinSetNodes;
    static const short Version = 4;
};
inline const Util::Array<ModelConstants::CharacterNode>
ModelConstants::GetCharacterNodes() const
{
    return this->characterNodes.ValuesAsArray();
}
inline const Util::Array<ModelConstants::ParticleNode>
ModelConstants::GetParticleNodes() const
{
    return this->particleNodes.ValuesAsArray();
}
inline const Util::Array<ModelConstants::PhysicsNode>
ModelConstants::GetPhysicsNodes() const
{
    return this->physicsNodes.ValuesAsArray();
}
inline void
ModelConstants::SetGlobalBoundingBox( const Math::bbox& bbox )
{
    this->globalBoundingBox = bbox;
}
inline const Math::bbox&
ModelConstants::GetGlobalBoundingBox() const
{
    return this->globalBoundingBox;
}
inline void
ModelConstants::SetName( const Util::String& name )
{
    this->name = name;
}
inline const Util::String&
ModelConstants::GetName() const
{
    return this->name;
}
}