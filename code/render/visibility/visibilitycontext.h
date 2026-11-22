#pragma once
#include "graphics/graphicscontext.h"
#include "visibility.h"
#include "jobs/jobs.h"
#include "visibility/systems/visibilitysystem.h"
#include "models/model.h"
#include "models/nodes/shaderstatenode.h"
#include "materials/materialtemplates.h"
#include "memory/arenaallocator.h"
#include "math/clipstatus.h"
#include "coregraphics/mesh.h"
namespace Models
{
    class ModelContext;
}
namespace Visibility
{
enum
{
    Observer_Matrix,
    Observer_IsOrtho,
    Observer_EntityId,
    Observer_EntityType,
    Observer_ResultArray,
    Observer_Dependency,
    Observer_DependencyMode,
    Observer_DrawList,
    Observer_DrawListAllocator,
};
enum
{
    ObservableAtom_NodeInstanceRange,
    ObservableAtom_GraphicsEntityId,
    ObservableAtom_Transform,
    ObservableAtom_Instance,
    ObservableAtom_VisibilityEntityType,
    ObservableAtom_Active
};
enum
{
    Observable_EntityId,
    Observable_NumNodes
};
enum DependencyMode
{
    DependencyMode_Total,
    DependencyMode_Masked
};
class ObserverContext : public Graphics::GraphicsContext
{
    __DeclareContext();
public:
    static void Setup(const Graphics::GraphicsEntityId id, VisibilityEntityType entityType, bool isOrtho = false);
    static void MakeDependency(const Graphics::GraphicsEntityId a, const Graphics::GraphicsEntityId b, const DependencyMode mode);
    static void RunVisibilityTests(const Graphics::FrameContext& ctx);
    static void GenerateDrawLists(const Graphics::FrameContext& ctx);
    static void Create();
    static void Discard();
    static VisibilitySystem* CreateBoxSystem(const BoxSystemLoadInfo& info);
    static VisibilitySystem* CreatePortalSystem(const PortalSystemLoadInfo& info);
    static VisibilitySystem* CreateOctreeSystem(const OctreeSystemLoadInfo& info);
    static VisibilitySystem* CreateQuadtreeSystem(const QuadtreeSystemLoadInfo& info);
    static VisibilitySystem* CreateBruteforceSystem(const BruteforceSystemLoadInfo& info);
    static void WaitForVisibility(const Graphics::FrameContext& ctx);
#ifndef PUBLIC_BUILD
    static void OnRenderDebug(uint32_t flags);
#endif
    typedef Ids::Id32 ModelAllocId;
    struct VisibilityModelCommand
    {
        uint32_t offset;
        CoreGraphics::MeshId mesh;
        CoreGraphics::PrimitiveGroup primitiveGroup;
        Materials::MaterialId material;
#if NEBULA_GRAPHICS_DEBUG
        Util::StringAtom nodeName;
#endif
    };
    struct VisibilityDrawCommand
    {
        uint32_t offset;
        uint32_t numInstances;
        uint32_t baseInstance;
    };
    struct VisibilityBatchCommand
    {
        uint32_t packetOffset;
        uint32_t numDrawPackets;
        Util::Array<VisibilityModelCommand, 256> models;
        Util::Array<VisibilityDrawCommand, 1024> draws;
    };
    struct VisibilityDrawList
    {
        Util::HashTable<const MaterialTemplates::Entry*, VisibilityBatchCommand> visibilityTable;
        Util::Array<Models::ShaderStateNode::DrawPacket*> drawPackets;
    };
    static const VisibilityDrawList* GetVisibilityDrawList(const Graphics::GraphicsEntityId id);
    static Jobs::JobSyncId jobInternalSync;
    static Jobs::JobSyncId jobInternalSync2;
    static Jobs::JobSyncId jobInternalSync3;
    static Jobs::JobSyncId jobHostSync;
    static Util::Queue<Jobs::JobId> runningJobs;
private:
    friend class ObservableContext;
    friend struct ObservableGlobalState;
    typedef Util::Array<Math::ClipStatus::Type> VisibilityResultArray;
    typedef Ids::IdAllocator<
        Math::mat4
        , bool
        , Graphics::GraphicsEntityId
        , VisibilityEntityType
        , VisibilityResultArray
        , Graphics::GraphicsEntityId
        , DependencyMode
        , VisibilityDrawList
        , Memory::ArenaAllocator<1024>
    > ObserverAllocator;
    static ObserverAllocator observerAllocator;
    static Graphics::ContextEntityId Alloc();
    static void Dealloc(Graphics::ContextEntityId id);
    static Util::Array<VisibilitySystem*> systems;
};
class ObservableContext : public Graphics::GraphicsContext
{
    __DeclareContext();
public:
    static void Setup(const Graphics::GraphicsEntityId id, VisibilityEntityType entityType);
    static void Create();
private:
    friend class ObserverContext;
    friend class Models::ModelContext;
    typedef Ids::IdAllocator<
        Graphics::GraphicsEntityId,
        uint32_t
    > ObservableAllocator;
    static ObservableAllocator observableAllocator;
    static Graphics::ContextEntityId Alloc();
    static void Dealloc(Graphics::ContextEntityId id);
};
}