#pragma once
#include "graphics/graphicscontext.h"
#include "core/singleton.h"
#include "resources/resourceid.h"
#include "resources/resourceserver.h"
#include "coregraphics/resourcetable.h"
#include "model.h"
#include "nodes/modelnode.h"
namespace Jobs
{
    struct JobFuncContext;
    struct JobSyncId;
};
namespace Materials
{
    struct MaterialId;
};
namespace CoreGraphics
{
    struct MeshId;
};
namespace Visibility
{
    class VisibilityContext;
};
namespace Raytracing
{
    class RaytracingContext;
};
namespace Models
{
enum class NodeInstanceFlags
{
    NodeInstance_Active = N_BIT(1)
    , NodeInstance_LodActive = N_BIT(2)
    , NodeInstance_AlwaysVisible = N_BIT(3)
    , NodeInstance_Visible = N_BIT(4)
    , NodeInstance_Moved = N_BIT(5)
};
__ImplementEnumBitOperators(NodeInstanceFlags);
class ModelContext : public Graphics::GraphicsContext
{
    __DeclareContext();
public:
    struct MaterialInstanceContext;
    ModelContext();
    virtual ~ModelContext();
    static void Create();
    static void Setup(const Graphics::GraphicsEntityId id, const Resources::ResourceName& name, const Util::StringAtom& tag, std::function<void()> finishedCallback);
    static void Setup(
        const Graphics::GraphicsEntityId id
        , const Math::mat4 transform
        , const Math::bbox& boundingBox
        , const Materials::MaterialId material
        , const CoreGraphics::MeshId mesh
        , const IndexT primitiveGroup
#if NEBULA_GRAPHICS_DEBUG
        , const Util::String debugName = ""
#endif
    );
    static void ChangeModel(const Graphics::GraphicsEntityId id, const Resources::ResourceName& name, const Util::StringAtom& tag, std::function<void()> finishedCallback);
    static const Models::ModelId GetModel(const Graphics::GraphicsEntityId id);
    static void SetTransform(const Graphics::GraphicsEntityId id, const Math::mat4& transform);
    static Math::mat4 GetTransform(const Graphics::GraphicsEntityId id);
    static Math::mat4 GetTransform(const Graphics::ContextEntityId id);
    static Math::bbox ComputeBoundingBox(const Graphics::GraphicsEntityId id);
    static IndexT GetNodeIndex(const Graphics::GraphicsEntityId id, const Util::StringAtom& name);
    static MaterialInstanceContext& SetupMaterialInstanceContext(const Graphics::GraphicsEntityId id, const IndexT nodeIndex, const MaterialTemplates::BatchGroup batch);
    static MaterialInstanceContext& SetupMaterialInstanceContext(const Graphics::GraphicsEntityId id, const MaterialTemplates::BatchGroup batch);
    static CoreGraphics::ConstantBufferOffset AllocateInstanceConstants(const Graphics::GraphicsEntityId id, const IndexT nodeIndex, const Materials::BatchIndex batch);
    static CoreGraphics::ConstantBufferOffset AllocateInstanceConstants(const Graphics::GraphicsEntityId id, const Materials::BatchIndex batch);
    static void SetAlwaysVisible(const Graphics::GraphicsEntityId id);
    static void UpdateTransforms(const Graphics::FrameContext& ctx);
    static void WaitForWork(const Graphics::FrameContext& ctx);
#ifndef PUBLIC_DEBUG
    static void OnRenderDebug(uint32_t flags);
#endif
    static const Models::ModelId GetModel(const Graphics::ContextEntityId id);
    struct NodeInstanceState
    {
        Util::FixedArray<CoreGraphics::ResourceTableId> resourceTables;
        Materials::MaterialInstanceId materialInstance;
        Util::FixedArray<uint32_t> resourceTableOffsets;
        IndexT objectConstantsIndex;
        IndexT instancingConstantsIndex;
        IndexT skinningConstantsIndex;
        IndexT particleConstantsIndex;
    };
    struct ModelInstance
    {
        struct Transformable
        {
            Util::PinnedArray<0xFFFF, Math::mat4> origTransforms;
            Util::PinnedArray<0xFFFF, Math::mat4> nodeTransforms;
            Util::PinnedArray<0xFFFF, uint32_t> nodeParents;
        } transformable;
        struct Renderable
        {
            Util::PinnedArray<0xFFFF, Math::bbox> origBoundingBoxes;
            Util::PinnedArray<0xFFFF, Math::bbox> nodeBoundingBoxes;
            Util::PinnedArray<0xFFFF, Util::Tuple<float, float>> nodeLodDistances;
            Util::PinnedArray<0xFFFF, float> nodeLods;
            Util::PinnedArray<0xFFFF, float> textureLods;
            Util::PinnedArray<0xFFFF, uint32_t> nodeTransformIndex;
            Util::PinnedArray<0xFFFF, uint64_t> nodeSortId;
            Util::PinnedArray<0xFFFF, NodeInstanceFlags> nodeFlags;
            Util::PinnedArray<0xFFFF, Materials::MaterialId> nodeMaterials;
            Util::PinnedArray<0xFFFF, const MaterialTemplates::Entry*> nodeMaterialTemplates;
            Util::PinnedArray<0xFFFF, NodeInstanceState> nodeStates;
            Util::PinnedArray<0xFFFF, Models::NodeType> nodeTypes;
            Util::PinnedArray<0xFFFF, Models::ModelNode*> nodes;
            Util::PinnedArray<0xFFFF, CoreGraphics::MeshId> nodeMeshes;
            Util::PinnedArray<0xFFFF, CoreGraphics::PrimitiveGroup> nodePrimitiveGroup;
            Util::PinnedArray<0xFFFF, Util::Tuple<uint32_t, uint32_t>> nodeDrawModifiers;
            Util::PinnedArray<0xFFFF, void*> nodeSpecialData;
#if NEBULA_GRAPHICS_DEBUG
            Util::PinnedArray<0xFFFF, Util::StringAtom> nodeNames;
#endif
        } renderable;
    };
    struct MaterialInstanceContext
    {
        Materials::BatchIndex batch;
        SizeT constantBufferSize;
    };
    static const NodeInstanceRange& GetModelRenderableRange(const Graphics::GraphicsEntityId id);
    static const NodeInstanceRange& GetModelTransformableRange(const Graphics::GraphicsEntityId id);
    static const Util::Array<NodeInstanceState>& GetModelRenderableStates();
    static const Util::Array<Math::bbox>& GetModelRenderableBoundingBoxes();
    static const Util::Array<NodeInstanceFlags>& GetModelRenderableFlags();
    static const ModelInstance::Renderable& GetModelRenderables();
    static const ModelInstance::Transformable& GetModelTransformables();
    static bool IsLoaded(const Graphics::GraphicsEntityId id);
    static Threading::AtomicCounter ConstantsUpdateCounter;
    static Threading::AtomicCounter TransformsUpdateCounter;
private:
    friend class Visibility::VisibilityContext;
    friend class Raytracing::RaytracingContext;
    static ModelInstance NodeInstances;
    static Memory::RangeAllocator TransformInstanceAllocator, RenderInstanceAllocator;
    enum
    {
        Model_Id,
        Model_NodeInstanceRoots,
        Model_NodeInstanceTransform,
        Model_NodeInstanceStates,
        Model_NodeLookup,
        Model_Transform,
        Model_Dirty
    };
    typedef Ids::IdAllocator<
        Resources::ResourceId,
        Util::Array<uint32_t>,
        NodeInstanceRange,
        NodeInstanceRange,
        Util::Dictionary<Util::StringAtom, IndexT>,
        Math::mat4,
        bool
    > ModelContextAllocator;
    static ModelContextAllocator modelContextAllocator;
    static Util::Dictionary<Models::ModelNode*, MaterialInstanceContext> materialInstanceContexts;
    static Threading::Event completionEvent;
    static Graphics::ContextEntityId Alloc();
    static void Dealloc(Graphics::ContextEntityId id);
};
inline Graphics::ContextEntityId
ModelContext::Alloc()
{
    return modelContextAllocator.Alloc();
}
inline void
ModelContext::Dealloc(Graphics::ContextEntityId id)
{
    Resources::ResourceId rid = modelContextAllocator.Get<Model_Id>(id.id);
    if (rid != Resources::InvalidResourceId)
        Resources::DiscardResource(rid);
    rid = Resources::InvalidResourceId;
    modelContextAllocator.Get<Model_NodeInstanceRoots>(id.id).Clear();
    modelContextAllocator.Get<Model_NodeLookup>(id.id).Clear();
    TransformInstanceAllocator.Dealloc(modelContextAllocator.Get<Model_NodeInstanceTransform>(id.id).allocation);
    RenderInstanceAllocator.Dealloc(modelContextAllocator.Get<Model_NodeInstanceStates>(id.id).allocation);
    modelContextAllocator.Dealloc(id.id);
}
}