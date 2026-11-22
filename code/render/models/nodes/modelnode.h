#pragma once
#include "core/refcounted.h"
#include "util/stringatom.h"
#include "io/binaryreader.h"
#include "math/bbox.h"
#include "ids/id.h"
#include "memory/arenaallocator.h"
#include "models/model.h"
#include "materials/material.h"
#include "coregraphics/primitivegroup.h"
#include "util/delegate.h"
namespace Characters
{
class CharacterContext;
}
namespace Particles
{
class ParticleContext;
}
namespace CoreGraphics
{
struct ResourceTableId;
struct CmdBufferId;
}
namespace Models
{
struct ModelId;
class ModelLoader;
class ModelNode
{
public:
    ModelNode();
    virtual ~ModelNode();
    const Util::Array<ModelNode*>& GetChildren() const;
    const NodeType GetType() const;
    const NodeBits GetBits() const;
    virtual bool GetImplicitHierarchyActivation() const;
    const Util::StringAtom& GetName() const;
    const uint32_t HashCode() const;
    virtual std::function<void(const CoreGraphics::CmdBufferId)> GetApplyFunction();
    virtual std::function<const CoreGraphics::PrimitiveGroup()> GetPrimitiveGroupFunction();
    virtual void Unload();
protected:
    friend class ModelLoader;
    friend class ModelContext;
    friend class ModelServer;
    friend class CharacterNode;
    friend class CharacterSkinNode;
    friend class Characters::CharacterContext;
    friend class Particles::ParticleContext;
    virtual bool Load(const Util::FourCC& fourcc, const Util::StringAtom& tag, const Ptr<IO::BinaryReader>& reader, bool immediate);
    virtual void OnFinishedLoading(ModelStreamingData* streamingData);
    virtual void Discard();
    Util::StringAtom name;
    NodeType type;
    NodeBits bits;
    Models::ModelNode* parent;
    Util::Array<Models::ModelNode*> children;
    Math::bbox boundingBox;
    Util::StringAtom tag;
    IndexT uniqueId;
    static IndexT ModelNodeUniqueIdCounter;
};
inline const Util::Array<ModelNode*>&
ModelNode::GetChildren() const
{
    return this->children;
}
inline const NodeType
ModelNode::GetType() const
{
    return this->type;
}
inline const NodeBits
ModelNode::GetBits() const
{
    return this->bits;
}
inline const Util::StringAtom&
ModelNode::GetName() const
{
    return this->name;
}
inline const uint32_t
ModelNode::HashCode() const
{
    return (uint32_t)this->uniqueId;
}
inline std::function<void(const CoreGraphics::CmdBufferId)>
ModelNode::GetApplyFunction()
{
    return nullptr;
}
inline std::function<const CoreGraphics::PrimitiveGroup()>
ModelNode::GetPrimitiveGroupFunction()
{
    return nullptr;
}
}