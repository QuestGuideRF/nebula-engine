#pragma once
#include "ids/id.h"
#include "ids/idallocator.h"
#include "resources/resourceid.h"
#include "core/rttimacros.h"
#include "memory/rangeallocator.h"
#include <functional>
namespace Models
{
class ModelNode;
RESOURCE_ID_TYPE(ModelId);
enum NodeType
{
    CharacterNodeType,
    TransformNodeType,
    ShaderStateNodeType,
    PrimitiveNodeType,
    ParticleSystemNodeType,
    CharacterSkinNodeType,
    NumNodeTypes
};
enum NodeBits
{
    NoBits = N_BIT(0),
    HasTransformBit = N_BIT(1),
    HasStateBit = N_BIT(2)
};
__ImplementEnumBitOperators(NodeBits);
enum class LoadBits
{
    NoBits = N_BIT(0),
    MeshBit = N_BIT(1),
    AnimBit = N_BIT(2),
    MaterialBit = N_BIT(3)
};
__ImplementEnumBitOperators(LoadBits);
struct ModelStreamingData
{
    LoadBits requiredBits;
    LoadBits loadedBits;
};
struct NodeInstanceRange
{
    Memory::RangeAllocation allocation;
    SizeT begin, end;
};
struct ModelCreateInfo
{
    Math::bbox boundingBox;
    Util::Array<Models::ModelNode*> nodes;
};
const ModelId CreateModel(const ModelCreateInfo& info);
void DestroyModel(const ModelId id);
const Util::Array<Models::ModelNode*>& ModelGetNodes(const ModelId id);
const Math::bbox& ModelGetBoundingBox(const ModelId id);
enum
{
    Model_BoundingBox,
    Model_Nodes
};
typedef Ids::IdAllocator<
    Math::bbox,
    Util::Array<Models::ModelNode*>
> ModelAllocator;
extern ModelAllocator modelAllocator;
}