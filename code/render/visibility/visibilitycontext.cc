//------------------------------------------------------------------------------
//  visobservercontext.cc
//  (C) 2018-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "visibilitycontext.h"
#include "graphics/graphicsserver.h"

#include "graphics/cameracontext.h"
#include "particles/particlecontext.h"
#include "lighting/lightcontext.h"
#include "lighting/lightprobecontext.h"
#include "models/modelcontext.h"
#include "particles/particlecontext.h"
#include "characters/charactercontext.h"

#include "systems/boxsystem.h"
#include "systems/octreesystem.h"
#include "systems/portalsystem.h"
#include "systems/quadtreesystem.h"
#include "systems/bruteforcesystem.h"

#include "profiling/profiling.h"

#include "util/randomnumbertable.h"

#include "jobs2/jobs2.h"

#ifndef PUBLIC_BUILD
#include "imgui.h"
#endif

namespace Visibility
{

ObserverContext::ObserverAllocator ObserverContext::observerAllocator;
ObservableContext::ObservableAllocator ObservableContext::observableAllocator;

Util::Array<VisibilitySystem*> ObserverContext::systems;

static Util::Queue<Threading::Event*> waitEvents;

__ImplementContext(ObserverContext, ObserverContext::observerAllocator)

struct ObservableGlobalState
{
    ObserverContext::VisibilityResultArray visibilityResults;
} ObservableState;

//------------------------------------------------------------------------------
/**
*/
void
ObserverContext::Setup(const Graphics::GraphicsEntityId id, VisibilityEntityType entityType, bool isOrtho)
{
    const Graphics::ContextEntityId cid = GetContextId(id);
    observerAllocator.Set<Observer_EntityType>(cid.id, entityType);
    observerAllocator.Set<Observer_EntityId>(cid.id, id);
    observerAllocator.Set<Observer_IsOrtho>(cid.id, isOrtho);
    observerAllocator.Set<Observer_ResultArray>(cid.id, ObservableState.visibilityResults);
}

//------------------------------------------------------------------------------
/**
*/
void
ObserverContext::MakeDependency(const Graphics::GraphicsEntityId a, const Graphics::GraphicsEntityId b, const DependencyMode mode)
{
    const Graphics::ContextEntityId cid = GetContextId(b);
    observerAllocator.Set<Observer_Dependency>(cid.id, a);
    observerAllocator.Set<Observer_DependencyMode>(cid.id, mode);
}

//------------------------------------------------------------------------------
/**
*/
void
ObserverContext::RunVisibilityTests(const Graphics::FrameContext& ctx)
{
    N_SCOPE(RunVisibilityTests, Visibility);

    const Models::ModelContext::ModelInstance::Renderable& NodeInstances = Models::ModelContext::GetModelRenderables();

    Util::Array<Math::mat4>& observerTransforms = observerAllocator.GetArray<Observer_Matrix>();
    const Util::Array<bool>& observerIsOrthogonal = observerAllocator.GetArray<Observer_IsOrtho>();
    const Util::Array<Graphics::GraphicsEntityId>& observerIds = observerAllocator.GetArray<Observer_EntityId>();
    const Util::Array<VisibilityEntityType>& observerTypes = observerAllocator.GetArray<Observer_EntityType>();
    Util::Array<VisibilityResultArray>& observerResults = observerAllocator.GetArray<Observer_ResultArray>();

    IndexT i;
    for (i = 0; i < observerIds.Size(); i++)
    {
        const Graphics::GraphicsEntityId id = observerIds[i];
        const VisibilityEntityType type = observerTypes[i];

        if (id == Graphics::GraphicsEntityId::Invalid())
            continue;

        switch (type)
        {
        case Camera:
            observerTransforms[i] = Graphics::CameraContext::GetViewProjection(id);
            break;
        case Light:
            observerTransforms[i] = Lighting::LightContext::GetObserverTransform(id);
            break;
        case LightProbe:
            observerTransforms[i] = Graphics::LightProbeContext::GetTransform(id);
            break;
        default: n_error("необработанное перечисление"); break;
        }
    }

    // сбросить все списки, чтобы все объекты были видны
    for (i = 0; i < observerResults.Size(); i++)
    {
        VisibilityDrawList& visibilities = observerAllocator.Get<Observer_DrawList>(i);
        observerResults[i].Fill(0, observerResults[i].Size(), Math::ClipStatus::Type::Outside);
        visibilities.visibilityTable.Clear();
        visibilities.drawPackets.Clear();
    }

    // подготовить системы видимости
    if (observerTransforms.Size() > 0)
    {
        for (i = 0; i < ObserverContext::systems.Size(); i++)
        {
            VisibilitySystem* sys = ObserverContext::systems[i];
            sys->PrepareObservers(observerTransforms.Begin(), observerIsOrthogonal.Begin(), observerResults.Begin(), observerTransforms.Size());
        }
    }

    // настроить наблюдаемые объекты
    const Util::Array<Graphics::GraphicsEntityId>& ids = ObservableContext::observableAllocator.GetArray<Observable_EntityId>();
    static Util::Array<uint32_t> nodes;
    nodes.Clear();
    nodes.Resize(observerResults[0].Size());

    static Threading::AtomicCounter idCounter;
    idCounter = 1;
    if (nodes.Size() > 0)
    {
        Threading::AtomicCounter counter = 0;

        // Запустите задание для сбора идентификаторов узлов модели.
        Jobs2::JobDispatch(
            [
                idData = ids.Begin()
                , nodeData = nodes.Begin()
                , counter
            ]
        (SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset) mutable
        {
            N_SCOPE(VisibilityIdCollectJob, Graphics);
            for (IndexT i = 0; i < groupSize; i++)
            {
                // Получить индекс элемента
                IndexT index = i + invocationOffset;
                if (index >= totalJobs)
                    return;

                // Получить диапазон узлов и обновить буфер идентификаторов
                Graphics::GraphicsEntityId gid = idData[index];
                if (gid == Graphics::InvalidGraphicsEntityId)
                    continue;

                const Models::NodeInstanceRange& NodeInstances = Models::ModelContext::GetModelRenderableRange(idData[index]);
                const uint numNodes = NodeInstances.end - NodeInstances.begin;
                uint offset = Threading::Interlocked::Add(&counter, numNodes);
                for (IndexT j = NodeInstances.begin; j < NodeInstances.end; j++)
                    nodeData[offset++] = j;
            }
        }, ids.Size(), 1024, {}, &idCounter, nullptr);

        for (i = 0; i < ObserverContext::systems.Size(); i++)
        {
            VisibilitySystem* sys = ObserverContext::systems[i];
            sys->PrepareEntities(NodeInstances.nodeBoundingBoxes.Begin(), nodes.Begin(), ids.Begin(), reinterpret_cast<uint32_t*>(NodeInstances.nodeFlags.Begin()), nodes.Size());
        }
    }

    // запустить все системы видимости
    const Threading::AtomicCounter* prevSystemCounters = nullptr;
    if ((observerTransforms.Size() > 0) && (NodeInstances.nodeBoundingBoxes.Size() > 0))
    {
        for (i = 0; i < ObserverContext::systems.Size(); i++)
        {
            VisibilitySystem* sys = ObserverContext::systems[i];

            // Подождите, пока преобразования и обновления ограничительной рамки завершится, прежде чем мы приступим к тестированию видимости.
            sys->Run(prevSystemCounters, { &idCounter, &Particles::ParticleContext::ConstantUpdateCounter, &Models::ModelContext::TransformsUpdateCounter });
            prevSystemCounters = sys->GetCompletionCounters();
        }
    }

    static Threading::AtomicCounter completionCounter;
    completionCounter = observerResults.Size();
    Threading::Event* finishedEvent = nullptr;

    if (NodeInstances.nodeStates.Size() > 0)
        finishedEvent = new Threading::Event;

    for (i = 0; i < observerResults.Size(); i++)
    {
        // раннее прерывание пустых запросов видимости
        if (NodeInstances.nodeStates.Size() == 0)
        {
            continue;
        }

        const VisibilityResultArray& results = observerResults[i];
        VisibilityDrawList& visibilities = observerAllocator.Get<Observer_DrawList>(i);
        Memory::ArenaAllocator<1024>& allocator = observerAllocator.Get<Observer_DrawListAllocator>(i);

        // Прежде чем создавать наши отрисовки, нам нужно сначала дождаться выделения констант.
        // Для частиц это делается до видимости, поэтому здесь мы можем это опустить.
        Util::FixedArray<const Threading::AtomicCounter*, true> waitCounters =
        {
            &prevSystemCounters[i],
            &Models::ModelContext::ConstantsUpdateCounter,
            &Characters::CharacterContext::ConstantUpdateCounter,
        };

        Jobs2::JobDispatch(
            [
                statuses = results.ConstBegin()
                , ids = nodes.ConstBegin()
                , drawList = &visibilities
                , allocator = &allocator
                , renderables = &NodeInstances
            ]
        (SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset)
        {
            N_SCOPE(VisibilitySortJob, Graphics);
            allocator->Release();

            // рассчитать количество моделей
            uint32_t numNodeInstances = totalJobs;

            if (numNodeInstances == 0)
                return;

            uint64_t visibleCounter = 0;
            Util::FixedArray<uint64_t, true> indexBuffer(numNodeInstances);
            Util::FixedArray<Math::ClipStatus::Type, true> clipStatuses(numNodeInstances);
            for (uint32_t i = 0; i < numNodeInstances; i++)
            {
                // Убедитесь, что мы не превышаем количество битов в индексном буфере, зарезервированном для фактического экземпляра узла.
                n_assert(ids[i] < 0xFFFFFFFF);
                clipStatuses[visibleCounter] = statuses[i];
                indexBuffer[visibleCounter] = ids[i];
                visibleCounter++;
            }

            // перебрать каждый узел и присвоить им соответствующий вес
            uint32_t i = 0;
            while (i < visibleCounter)
            {
                n_assert(indexBuffer[i] < 0x00000000FFFFFFFF);
                uint64_t index = indexBuffer[i] & 0x00000000FFFFFFFF;
                Math::ClipStatus::Type clipStatus = clipStatuses[i];

                // If not visible nor active, erase item from index list
                if (!AllBits(renderables->nodeFlags[index], Models::NodeInstanceFlags::NodeInstance_Active)
                    || clipStatus == Math::ClipStatus::Outside)
                {
                    indexBuffer[i] = indexBuffer[visibleCounter - 1];
                    clipStatuses[i] = clipStatuses[visibleCounter - 1];
                    visibleCounter--;
                    continue;
                }
                else
                {
                    // Set the node visible flag (use this to figure out if a node is seen by __any__ observer)
                    renderables->nodeFlags[index] = SetBits(renderables->nodeFlags[index], Models::NodeInstanceFlags::NodeInstance_Visible);
                }

                // Get sort id and combine with index to get full sort id
                uint64_t sortId = renderables->nodeSortId[index];
                indexBuffer[i] = sortId | index;
                i++;
            }

            if (visibleCounter == 0)
                return; // рано выходить

            // отсортировать индексный буфер
            std::qsort(indexBuffer.Begin(), visibleCounter, sizeof(uint64_t), [](const void* a, const void* b)
            {
                uint64_t arg1 = *static_cast<const uint64_t*>(a);
                uint64_t arg2 = *static_cast<const uint64_t*>(b);
                return (arg1 > arg2) - (arg1 < arg2);
            });

            // Теперь разрешите индексный буфер в команды рисования.
            uint32_t numDraws = 0;
            const uint32_t numPackets = visibleCounter;
            drawList->drawPackets.Reserve(numPackets);

            // Выделите одну команду, которую мы можем
            ObserverContext::VisibilityBatchCommand* cmd = nullptr;
            CoreGraphics::MeshId mesh = CoreGraphics::InvalidMeshId;
            Materials::MaterialId mat = Materials::InvalidMaterialId;
            static auto NullDrawModifiers = Util::MakeTuple(UINT32_MAX, UINT32_MAX);
            Util::Tuple<uint32_t, uint32_t> drawModifiers = NullDrawModifiers;
            const MaterialTemplates::Entry* currentMaterialType = nullptr;

            for (uint32_t i = 0; i < numPackets; i++)
            {
                uint32_t index = indexBuffer[i] & 0x00000000FFFFFFFF;

                // Если новый материал, добавьте новую запись в таблицу поиска.
                auto otherMaterialType = renderables->nodeMaterialTemplates[index];
                if (currentMaterialType != otherMaterialType)
                {
                    // Добавьте новую команду рисования и получите ссылку на нее.
                    cmd = &drawList->visibilityTable.Emplace(otherMaterialType);

                    // Настройка начального состояния для команды
                    cmd->packetOffset = numDraws;
                    cmd->numDrawPackets = 0;

                    mesh = CoreGraphics::InvalidMeshId;
                    drawModifiers = NullDrawModifiers;
                    currentMaterialType = otherMaterialType;
                }
                n_assert(cmd != nullptr);

                // If a new node (resource), add a model apply command
                auto otherMesh = renderables->nodeMeshes[index];
                auto otherMat = renderables->nodeMaterials[index];
                if (mesh != otherMesh || mat != otherMat)
                {
                    ObserverContext::VisibilityModelCommand& batchCmd = cmd->models.Emplace();

                    // Смещение команды соответствует тому месту в пакете VisibilityBatchCommand, где должна применяться модель.
                    batchCmd.offset = cmd->packetOffset + cmd->numDrawPackets;
                    batchCmd.mesh = otherMesh;
                    batchCmd.primitiveGroup = renderables->nodePrimitiveGroup[index];
                    batchCmd.material = otherMat;

#if NEBULA_GRAPHICS_DEBUG
                    batchCmd.nodeName = renderables->nodeNames[index];
#endif
                    mesh = otherMesh;
                    mat = otherMat;
                }

                // If a new set of draw modifiers (instance count and base instance) are used, insert a new draw command
                auto otherDrawModifiers = renderables->nodeDrawModifiers[index];
                if (drawModifiers != otherDrawModifiers)
                {
                    ObserverContext::VisibilityDrawCommand& drawCmd = cmd->draws.Emplace();
                    drawCmd.offset = cmd->packetOffset + cmd->numDrawPackets;
                    drawCmd.numInstances = Util::Get<0>(otherDrawModifiers);
                    drawCmd.baseInstance = Util::Get<1>(otherDrawModifiers);

                    drawModifiers = otherDrawModifiers;
                }

                // выделить память для отрисовки пакета
                void* mem = allocator->Alloc(sizeof(Models::ShaderStateNode::DrawPacket));

                // обновить пакет и добавить в список
                Models::ShaderStateNode::DrawPacket* packet = reinterpret_cast<Models::ShaderStateNode::DrawPacket*>(mem);
                packet->numOffsets = renderables->nodeStates[index].resourceTableOffsets.Size();
                packet->table = renderables->nodeStates[index].resourceTables[CoreGraphics::GetBufferedFrameIndex()];
                packet->materialInstance = renderables->nodeStates[index].materialInstance;
#ifndef PUBLIC_BUILD
                packet->boundingBox = renderables->nodeBoundingBoxes[index];
                packet->nodeInstanceHash = index;
#endif
                memcpy(packet->offsets, renderables->nodeStates[index].resourceTableOffsets.Begin(), renderables->nodeStates[index].resourceTableOffsets.ByteSize());
                packet->slot = NEBULA_DYNAMIC_OFFSET_GROUP;
                drawList->drawPackets.Append(packet);
                cmd->numDrawPackets++;
                numDraws++;
            }
        }, nodes.Size(), waitCounters, &completionCounter, finishedEvent);
    }

    if (finishedEvent != nullptr)
        waitEvents.Enqueue(finishedEvent);
}

//------------------------------------------------------------------------------
/**
*/
void
ObserverContext::GenerateDrawLists(const Graphics::FrameContext& ctx)
{
    N_SCOPE(GenerateDrawLists, Visibility);
}

//------------------------------------------------------------------------------
/**
*/
void
ObserverContext::Create()
{
    __CreateContext();
#ifndef PUBLIC_BUILD
    __bundle.OnRenderDebug = ObserverContext::OnRenderDebug;
#endif

    Graphics::GraphicsServer::Instance()->RegisterGraphicsContext(&__bundle, &__state);
    __CreateContext();
}

//------------------------------------------------------------------------------
/**
*/
void
ObserverContext::Discard()
{
    for (int i = 0; i < ObserverContext::systems.Size(); i++)
    {
        delete ObserverContext::systems[i];
    }

    ObserverContext::systems.Clear();
    Graphics::GraphicsServer::Instance()->UnregisterGraphicsContext(&__bundle);
}

//------------------------------------------------------------------------------
/**
*/
VisibilitySystem*
ObserverContext::CreateBoxSystem(const BoxSystemLoadInfo& info)
{
    BoxSystem* system = new BoxSystem;
    system->Setup(info);
    ObserverContext::systems.Append(system);
    return system;
}

//------------------------------------------------------------------------------
/**
*/
VisibilitySystem*
ObserverContext::CreatePortalSystem(const PortalSystemLoadInfo& info)
{
    PortalSystem* system = new PortalSystem;
    system->Setup(info);
    ObserverContext::systems.Append(system);
    return system;
}

//------------------------------------------------------------------------------
/**
*/
VisibilitySystem*
ObserverContext::CreateOctreeSystem(const OctreeSystemLoadInfo& info)
{
    OctreeSystem* system = new OctreeSystem;
    system->Setup(info);
    ObserverContext::systems.Append(system);
    return system;
}

//------------------------------------------------------------------------------
/**
*/
VisibilitySystem*
ObserverContext::CreateQuadtreeSystem(const QuadtreeSystemLoadInfo & info)
{
    QuadtreeSystem* system = new QuadtreeSystem;
    system->Setup(info);
    ObserverContext::systems.Append(system);
    return system;
}

//------------------------------------------------------------------------------
/**
*/
VisibilitySystem*
ObserverContext::CreateBruteforceSystem(const BruteforceSystemLoadInfo& info)
{
    BruteforceSystem* system = new BruteforceSystem;
    system->Setup(info);
    ObserverContext::systems.Append(system);
    return system;
}

//------------------------------------------------------------------------------
/**
*/
void
ObserverContext::WaitForVisibility(const Graphics::FrameContext& ctx)
{
    while (waitEvents.Size() > 0)
    {
        auto ev = waitEvents.Dequeue();
        ev->Wait();
        delete ev;
    }
}

#ifndef PUBLIC_BUILD
//------------------------------------------------------------------------------
/**
*/
void
ObserverContext::OnRenderDebug(uint32_t flags)
{
    while (waitEvents.Size() > 0)
    {
        auto ev = waitEvents.Dequeue();
        ev->Wait();
        delete ev;
    }

    static int visIndex = 0;
    static int atomIndex = 0;
    static bool singleAtomMode = false;

    auto const& foo = observerAllocator.GetArray<Observer_DrawList>();
    if (singleAtomMode)
    {
        if (!foo[visIndex].drawPackets.IsEmpty())
        {
            atomIndex = Math::clamp(atomIndex, 0, foo[visIndex].drawPackets.Size() - 1);
            auto const& a = foo[visIndex].drawPackets[atomIndex];
            CoreGraphics::RenderShape shape;
            Math::vec4 color = {
#ifndef PUBLIC_BUILD
                    Util::RandomNumberTable::Rand(a->nodeInstanceHash),
                    Util::RandomNumberTable::Rand(a->nodeInstanceHash + 1),
                    Util::RandomNumberTable::Rand(a->nodeInstanceHash + 2),
#endif
                    1
            };
            shape.SetupSimpleShape(CoreGraphics::RenderShape::Box, CoreGraphics::RenderShape::RenderFlag(CoreGraphics::RenderShape::CheckDepth | CoreGraphics::RenderShape::Wireframe), color, a->boundingBox.to_mat4());
            CoreGraphics::ShapeRenderer::Instance()->AddShape(shape);
        }
    }
    else
    {
        for (auto const& a : foo[visIndex].drawPackets)
        {
            CoreGraphics::RenderShape shape;
            Math::vec4 color = {
#ifndef PUBLIC_BUILD
                Util::RandomNumberTable::Rand(a->nodeInstanceHash),
                Util::RandomNumberTable::Rand(a->nodeInstanceHash + 1),
                Util::RandomNumberTable::Rand(a->nodeInstanceHash + 2),
#endif
                1
            };
            shape.SetupSimpleShape(CoreGraphics::RenderShape::Box, CoreGraphics::RenderShape::RenderFlag(CoreGraphics::RenderShape::CheckDepth | CoreGraphics::RenderShape::Wireframe), color, a->boundingBox.to_mat4());
            CoreGraphics::ShapeRenderer::Instance()->AddShape(shape);
        }
    }

    Util::Array<VisibilityResultArray>& vis = observerAllocator.GetArray<Observer_ResultArray>();
    Util::FixedArray<SizeT> insideCounters(vis.Size(), 0);
    Util::FixedArray<SizeT> clippedCounters(vis.Size(), 0);
    Util::FixedArray<SizeT> totalCounters(vis.Size(), 0);
    for (IndexT i = 0; i < vis.Size(); i++)
    {
        auto res = vis[i];
        for (IndexT j = 0; j < res.Size(); j++)
            switch (res[j])
            {
            case Math::ClipStatus::Inside:
                insideCounters[i]++;
                totalCounters[i]++;
                break;
            case Math::ClipStatus::Clipped:
                clippedCounters[i]++;
                totalCounters[i]++;
                break;
            default:
                break;
            }
    }
    if (ImGui::Begin("Visibility"))
    {
        ImGui::Checkbox("Режим одного атома", &singleAtomMode);
        if (singleAtomMode)
        {
            ImGui::SliderInt("AtomIndex", &atomIndex, 0, (int)foo[visIndex].drawPackets.Size() - 1);
        }
        ImGui::SliderInt("visIndex", &visIndex, 0, (int)foo.size() - 1);
        for (IndexT i = 0; i < vis.Size(); i++)
        {
            ImGui::Text("Entities visible for observer %d: %d (inside [%d], clipped [%d])", i, totalCounters[i], insideCounters[i], clippedCounters[i]);
        }
    }
    ImGui::End();
}
#endif

//------------------------------------------------------------------------------
/**
*/
const ObserverContext::VisibilityDrawList*
ObserverContext::GetVisibilityDrawList(const Graphics::GraphicsEntityId id)
{
    const Graphics::ContextEntityId cid = ObserverContext::GetContextId(id);
    if (cid == Graphics::InvalidContextEntityId)
        return nullptr;
    else
        return &observerAllocator.Get<Observer_DrawList>(cid.id);
}

//------------------------------------------------------------------------------
/**
*/
Graphics::ContextEntityId
ObserverContext::Alloc()
{
    return observerAllocator.Alloc();
}

//------------------------------------------------------------------------------
/**
*/
void
ObserverContext::Dealloc(Graphics::ContextEntityId id)
{
    VisibilityDrawList& draws = observerAllocator.Get<Observer_DrawList>(id.id);
    draws.visibilityTable.Clear();
    draws.drawPackets.Clear();
    observerAllocator.Dealloc(id.id);
}

__ImplementContext(ObservableContext, ObservableContext::observableAllocator);

//------------------------------------------------------------------------------
/**
*/
void
ObservableContext::Setup(const Graphics::GraphicsEntityId id, VisibilityEntityType entityType)
{
    const Graphics::ContextEntityId cid = ObservableContext::GetContextId(id);
    observableAllocator.Set<Observable_EntityId>(cid.id, id);

    if (entityType == Model || entityType == Particle)
    {
        // Получить диапазоны экземпляров узла
        const Models::NodeInstanceRange& nodeInstanceRange = Models::ModelContext::GetModelRenderableRange(id);

        // Создайте новый диапазон узлов, чтобы запомнить, к какому месту в контексте видимости принадлежит эта модель.
        SizeT numNodes = nodeInstanceRange.end - nodeInstanceRange.begin;
        observableAllocator.Set<Observable_NumNodes>(cid.id, numNodes);

        // Обновить глобальный реестр наблюдаемых
        ObservableState.visibilityResults.Reserve(numNodes);
        for (IndexT i = 0; i < numNodes; i++)
            ObservableState.visibilityResults.Append(Math::ClipStatus::Outside);

        // Затем обновите наблюдателей
        const Util::Array<ObserverContext::VisibilityResultArray>& visAllocators = ObserverContext::observerAllocator.GetArray<Observer_ResultArray>();
        for (IndexT i = 0; i < visAllocators.Size(); i++)
        {
            ObserverContext::VisibilityResultArray& alloc = visAllocators[i];
            alloc = ObservableState.visibilityResults;
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void
ObservableContext::Create()
{
    __CreateContext();
    Graphics::GraphicsServer::Instance()->RegisterGraphicsContext(&ObservableContext::__bundle, &ObservableContext::__state);
}

//------------------------------------------------------------------------------
/**
*/
Graphics::ContextEntityId
ObservableContext::Alloc()
{
    Ids::Id32 id = observableAllocator.Alloc();
    observableAllocator.Set<Observable_EntityId>(id, Graphics::GraphicsEntityId::Invalid());
    return id;
}

//------------------------------------------------------------------------------
/**
*/
void
ObservableContext::Dealloc(Graphics::ContextEntityId id)
{
    uint32_t numNodes = observableAllocator.Get<Observable_NumNodes>(id.id);

    ObservableState.visibilityResults.EraseRange(0, numNodes);
    // добавьте как можно больше атомов в каждый распределитель результатов видимости
    const Util::Array<ObserverContext::VisibilityResultArray>& visAllocators = ObserverContext::observerAllocator.GetArray<Observer_ResultArray>();
    for (IndexT i = 0; i < visAllocators.Size(); i++)
    {
        ObserverContext::VisibilityResultArray& alloc = visAllocators[i];
        alloc = ObservableState.visibilityResults;
    }
    observableAllocator.Dealloc(id.id);
}

} // Видимость пространства имен
