#pragma once
#include "core/refcounted.h"
#include "timing/time.h"
#include "ids/id.h"
#include "ids/idallocator.h"
#include "ids/idgenerationpool.h"
#include "util/stringatom.h"
#include "util/arraystack.h"
#include "graphicsentity.h"
#include "coregraphics/window.h"
#define __DeclarePluginContext() \
private:\
    static Graphics::GraphicsContextState __state;\
    static Graphics::GraphicsContextFunctionBundle __bundle;\
public:\
    static void RegisterEntity(const Graphics::GraphicsEntityId id);\
    static void DeregisterEntity(const Graphics::GraphicsEntityId id);\
    static bool IsEntityRegistered(const Graphics::GraphicsEntityId id);\
    static void Destroy(); \
    static Graphics::ContextEntityId GetContextId(const Graphics::GraphicsEntityId id); \
    static const Graphics::ContextEntityId& GetContextIdRef(const Graphics::GraphicsEntityId id); \
    static void BeginBulkRegister(); \
    static void EndBulkRegister(); \
private:
#define __DeclareContext() \
    __DeclarePluginContext(); \
    static void Defragment();
#define __ImplementPluginContext(ctx) \
Graphics::GraphicsContextState ctx::__state; \
Graphics::GraphicsContextFunctionBundle ctx::__bundle; \
void ctx::RegisterEntity(const Graphics::GraphicsEntityId id) \
{\
    Graphics::GraphicsContext::InternalRegisterEntity(id, std::forward<Graphics::GraphicsContextState>(__state));\
}\
\
void ctx::DeregisterEntity(const Graphics::GraphicsEntityId id)\
{\
    Graphics::GraphicsContext::InternalDeregisterEntity(id, std::forward<Graphics::GraphicsContextState>(__state));\
}\
\
bool ctx::IsEntityRegistered(const Graphics::GraphicsEntityId id)\
{\
    return __state.entitySliceMap.Contains(id);\
}\
void ctx::Destroy()\
{\
    Graphics::GraphicsServer::Instance()->UnregisterGraphicsContext(&__bundle);\
}\
Graphics::ContextEntityId ctx::GetContextId(const Graphics::GraphicsEntityId id)\
{\
    IndexT idx = __state.entitySliceMap.FindIndex(id); \
    if (idx == InvalidIndex) return Graphics::InvalidContextEntityId; \
    else return __state.entitySliceMap.ValueAtIndex(id, idx); \
}\
const Graphics::ContextEntityId& ctx::GetContextIdRef(const Graphics::GraphicsEntityId id)\
{\
    IndexT idx = __state.entitySliceMap.FindIndex(id); \
    n_assert(idx != InvalidIndex); \
    return __state.entitySliceMap.ValueAtIndex(id, idx); \
}\
void ctx::BeginBulkRegister()\
{\
    __state.entitySliceMap.BeginBulkAdd();\
}\
void ctx::EndBulkRegister()\
{\
    __state.entitySliceMap.EndBulkAdd();\
}
#define __ImplementContext(ctx, idAllocator) \
__ImplementPluginContext(ctx);\
void ctx::Defragment()\
{\
    Graphics::GraphicsContext::InternalDefragment(idAllocator, std::forward<Graphics::GraphicsContextState>(__state));\
}
#define __CreatePluginContext() \
    __state.Alloc = Alloc; \
    __state.Dealloc = Dealloc;
#define __CreateContext() \
    __CreatePluginContext() \
    __state.Defragment = Defragment;
namespace Graphics
{
class View;
class Stage;
struct FrameContext;
struct GraphicsContextFunctionBundle
{
    void(*OnRenderDebug)(uint32_t flags);
    void(*OnStageCreated)(const Ptr<Graphics::Stage>& stage);
    void(*OnDiscardStage)(const Ptr<Graphics::Stage>& stage);
    void(*OnViewCreated)(const Ptr<Graphics::View>& view);
    void(*OnDiscardView)(const Ptr<Graphics::View>& view);
    void(*OnAttachEntity)(Graphics::GraphicsEntityId entity);
    void(*OnRemoveEntity)(Graphics::GraphicsEntityId entity);
    void(*OnWindowResized)(const CoreGraphics::WindowId windowId, SizeT width, SizeT height);
    GraphicsContextFunctionBundle() : OnRenderDebug(nullptr), OnStageCreated(nullptr), OnDiscardStage(nullptr), OnViewCreated(nullptr), OnDiscardView(nullptr),
        OnAttachEntity(nullptr), OnRemoveEntity(nullptr), OnWindowResized(nullptr)
    {
    };
};
ID_32_TYPE(ContextEntityId)
struct GraphicsContextState
{
    Util::StackArray<GraphicsEntityId, 8> delayedRemoveQueue;
    Util::Array<GraphicsEntityId> entities;
    Util::HashTable<GraphicsEntityId, ContextEntityId, 128, 64> entitySliceMap;
    ContextEntityId(*Alloc)();
    void(*Dealloc)(ContextEntityId id);
    void(*Defragment)();
    void(*OnInstanceMoved)(uint32_t toIndex, uint32_t fromIndex);
    void(*OnDefragment)(uint32_t toIndex, uint32_t fromIndex);
    void CleanupDelayedRemoveQueue()
    {
        while (!this->delayedRemoveQueue.IsEmpty())
        {
            Graphics::GraphicsEntityId eid = this->delayedRemoveQueue[0];
            IndexT index = this->entitySliceMap.FindIndex(eid);
            n_assert(index != InvalidIndex);
            auto cid = this->entitySliceMap.ValueAtIndex(eid.id, index);
            this->Dealloc(cid);
            this->entitySliceMap.EraseIndex(eid, index);
            this->delayedRemoveQueue.EraseIndexSwap(0);
        }
    }
};
class GraphicsContext
{
public:
    GraphicsContext();
    virtual ~GraphicsContext();
protected:
    friend class GraphicsServer;
    static void InternalRegisterEntity(const Graphics::GraphicsEntityId id, Graphics::GraphicsContextState&& state);
    static void InternalDeregisterEntity(const Graphics::GraphicsEntityId id, Graphics::GraphicsContextState&& state);
    template<class ID_ALLOCATOR>
    static void InternalDefragment(ID_ALLOCATOR& allocator, Graphics::GraphicsContextState&& state);
};
template<class ID_ALLOCATOR>
inline void
GraphicsContext::InternalDefragment(ID_ALLOCATOR& allocator, Graphics::GraphicsContextState&& state)
{
    auto& freeIds = allocator.FreeIds();
    Ids::Id32 index;
    Ids::Id32 oldIndex;
    Graphics::GraphicsEntityId lastId;
    IndexT mapIndex;
    uint32_t dataSize;
    SizeT size = freeIds.Size();
    for (SizeT i = size - 1; i >= 0; --i)
    {
        index = freeIds.Back();
        freeIds.EraseBack();
        dataSize = (uint32_t)allocator.Size();
        if (index >= dataSize)
        {
            continue;
        }
        oldIndex = dataSize - 1;
        lastId = state.entities[oldIndex].id;
        if (state.OnInstanceMoved != nullptr)
            state.OnInstanceMoved(index, oldIndex);
        allocator.EraseIndexSwap(index);
        state.entities.EraseIndexSwap(index);
        mapIndex = state.entitySliceMap.FindIndex(lastId);
        if (mapIndex != InvalidIndex)
        {
            state.entitySliceMap.ValueAtIndex(lastId, mapIndex) = index;
        }
        else
        {
            freeIds.Append(index);
            i++;
        }
    }
    freeIds.Clear();
}
}