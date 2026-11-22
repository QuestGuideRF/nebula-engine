#pragma once
#include "core/refcounted.h"
#include "entity.h"
#include "entitypool.h"
#include "component.h"
#include "memdb/tablesignature.h"
#include "util/queue.h"
#include "category.h"
#include "processorid.h"
#include "processor.h"
#include "memory/arenaallocator.h"
#include "frameevent.h"
#include "util/fourcc.h"
namespace MemDb
{
class Database;
}
namespace Util
{
class Blob;
}
namespace Game
{
class PackedLevel;
struct EntityCreateInfo
{
    TemplateId templateId = TemplateId::Invalid();
    bool immediate = false;
};
World* GetWorld(WorldHash worldHash);
World* GetWorld(WorldId worldId);
class World
{
public:
    World(WorldHash hash, WorldId id);
    ~World();
    WorldHash GetWorldHash() const;
    WorldId GetWorldId() const;
    Entity CreateEntity(bool immediate = true);
    Entity CreateEntity(EntityCreateInfo const& info);
    void DeleteEntity(Entity entity);
    bool IsValid(Entity entity) const;
    bool HasInstance(Entity entity) const;
    EntityMapping GetEntityMapping(Entity entity) const;
    template <typename TYPE>
    TYPE* AddComponent(Entity entity);
    template <typename TYPE>
    void AddComponent(Entity entity, TYPE const& component);
    void* AddComponent(Entity entity, ComponentId component);
    template <typename TYPE>
    bool HasComponent(Entity entity) const;
    bool HasComponent(Entity entity, ComponentId component) const;
    template <typename TYPE>
    void RemoveComponent(Entity entity);
    void RemoveComponent(Entity entity, ComponentId component);
    template <typename TYPE>
    void SetComponent(Entity entity, TYPE const& value);
    template <typename TYPE>
    TYPE GetComponent(Entity entity);
    void MarkAsModified(Game::Entity entity);
    Dataset Query(Filter filter);
    Dataset Query(Filter filter, Util::Array<MemDb::TableId>& tids);
    ComponentDecayBuffer const GetDecayBuffer(ComponentId component);
    PackedLevel* PreloadLevel(Util::String const& path);
    void UnloadLevel(PackedLevel* level);
    void ExportLevel(Util::String const& path);
    FramePipeline& GetFramePipeline();
    Ptr<MemDb::Database> GetDatabase();
    MemDb::RowId GetInstance(Entity entity) const;
    void SetComponentValue(Entity entity, ComponentId component, void* value, uint64_t size);
    void ReinitializeComponent(Entity entity, ComponentId component, void* value, uint64_t size);
    MemDb::TableId CreateEntityTable(EntityTableCreateInfo const& info);
    static void Override(World* src, World* dst);
    Entity AllocateEntityId();
    void DeallocateEntityId(Entity entity);
    MemDb::RowId AllocateInstance(Entity entity, MemDb::TableId table, Util::Blob const* const data = nullptr);
    MemDb::RowId AllocateInstance(Entity entity, BlueprintId blueprint);
    MemDb::RowId AllocateInstance(Entity entity, TemplateId templateId, bool performInitialize);
    void FinalizeAllocate(Entity entity);
    void DeallocateInstance(MemDb::TableId table, MemDb::RowId instance);
    void DeallocateInstance(Entity entity);
    void Defragment(MemDb::TableId tableId);
    void* GetInstanceBuffer(MemDb::TableId const tableId, uint16_t partitionId, ComponentId const component);
    void* GetColumnData(MemDb::TableId const tableId, uint16_t partitionId, MemDb::ColumnIndex const column);
    void ExecuteAddComponentCommands();
    bool componentInitializationEnabled = true;
    void RenderDebug();
private:
    friend class GameServer;
    friend class BlueprintManager;
    friend class PackedLevel;
    struct AllocateInstanceCommand
    {
        Game::Entity entity = Game::Entity::Invalid();
        TemplateId tid = TemplateId::Invalid();
    };
    struct DeallocInstanceCommand
    {
        Game::Entity entity = Game::Entity::Invalid();
    };
    struct AddStagedComponentCommand
    {
        Game::Entity entity = Game::Entity::Invalid();
        ComponentId componentId = ComponentId::Invalid();
        SizeT dataSize = 0;
        void* data = nullptr;
    };
    struct RemoveComponentCommand
    {
        Entity entity = Game::Entity::Invalid();
        ComponentId componentId = ComponentId::Invalid();
    };
    void Start();
    void BeginFrame();
    void SimFrame();
    void EndFrame();
    void OnLoad();
    void OnSave();
    void ManageEntities();
    void Reset();
    void PrefilterProcessors();
    void ClearDecayBuffers();
    void ExecuteRemoveComponentCommands();
    SizeT GetNumInstances(MemDb::TableId tid);
    MemDb::RowId Migrate(Entity entity, MemDb::TableId newTable);
    void Migrate(
        Util::Array<Entity> const& entities,
        MemDb::TableId fromTable,
        MemDb::TableId newTable,
        Util::FixedArray<MemDb::RowId>& newInstances
    );
    void DecayComponent(ComponentId component, MemDb::TableId tableId, MemDb::ColumnIndex column, MemDb::RowId instance);
    void MoveInstance(MemDb::Table::Partition* partition, MemDb::RowId from, MemDb::RowId to);
    void InitializeAllComponents(Entity entity, MemDb::TableId tableId, MemDb::RowId row);
    void AddStagedComponentsToEntity(Entity entity, AddStagedComponentCommand* cmds, SizeT numCmds);
    void RemoveComponentsFromEntity(Entity entity, RemoveComponentCommand* cmds, SizeT numCmds);
    EntityPool pool;
    SizeT numEntities;
    Util::Array<EntityMapping> entityMap;
    Ptr<MemDb::Database> db;
    WorldHash hash;
    WorldId worldId;
    Util::HashTable<BlueprintId, MemDb::TableId> blueprintToTableMap;
    Util::Queue<AllocateInstanceCommand> allocQueue;
    Util::Queue<DeallocInstanceCommand> deallocQueue;
    Util::Array<AddStagedComponentCommand> addStagedQueue;
    Util::Array<RemoveComponentCommand> removeComponentQueue;
    Memory::ArenaAllocator<4096_KB> componentStageAllocator;
    bool cacheValid = false;
    FramePipeline pipeline;
    MemDb::TableId defaultTableId;
    Util::FixedArray<ComponentDecayBuffer> componentDecayTable;
};
inline WorldHash
World::GetWorldHash() const
{
    return this->hash;
}
inline WorldId
World::GetWorldId() const
{
    return this->worldId;
}
template <typename TYPE>
inline void
World::SetComponent(Entity entity, TYPE const& value)
{
#if NEBULA_DEBUG
    n_assert2(
        !this->pipeline.IsRunningAsync(),
        "Компоненты настроек в произвольных объектах при выполнении асинхронного процессора в настоящее время не поддерживаются!"
    );
#endif
#if NEBULA_DEBUG
    n_assert2(
        sizeof(TYPE) == MemDb::AttributeRegistry::TypeSize(Game::GetComponentId<TYPE>()),
        "SetComponent: тип предоставленного значения не соответствует размеру данного ComponentId."
    );
#endif
    EntityMapping mapping = this->GetEntityMapping(entity);
    TYPE* ptr = (TYPE*)this->GetInstanceBuffer(mapping.table, mapping.instance.partition, GetComponentId<TYPE>());
    *(ptr + mapping.instance.index) = value;
}
template <typename TYPE>
inline TYPE
World::GetComponent(Entity entity)
{
#if NEBULA_DEBUG
    n_assert2(
        !this->pipeline.IsRunningAsync(),
        "Получение компонентов из сущностей во время выполнения асинхронного процессора в настоящее время не поддерживается!"
    );
#endif
#if NEBULA_DEBUG
    n_assert2(
        sizeof(TYPE) == MemDb::AttributeRegistry::TypeSize(Game::GetComponentId<TYPE>()),
        "GetComponent: тип предоставленного значения не соответствует размеру данного ComponentId."
    );
#endif
    EntityMapping mapping = this->GetEntityMapping(entity);
    TYPE* ptr = (TYPE*)this->GetInstanceBuffer(mapping.table, mapping.instance.partition, GetComponentId<TYPE>());
    return *(ptr + mapping.instance.index);
}
template <typename TYPE>
inline void
World::RemoveComponent(Entity entity)
{
#if NEBULA_DEBUG
    n_assert2(
        !this->pipeline.IsRunningAsync(),
        "Удаление компонентов из сущностей во время выполнения асинхронного процессора в настоящее время не поддерживается!"
    );
#endif
    RemoveComponentCommand cmd = {
        .entity = entity,
        .componentId = Game::GetComponentId<TYPE>(),
    };
    this->removeComponentQueue.Append(cmd);
}
template <typename TYPE>
inline bool
World::HasComponent(Game::Entity entity) const
{
    return this->HasComponent(entity, Game::GetComponentId<TYPE>());
}
template <typename TYPE>
inline TYPE*
World::AddComponent(Entity entity)
{
#if NEBULA_DEBUG
    n_assert2(
        !this->pipeline.IsRunningAsync(), "Добавление компонента к объектам в асинхронном процессоре в настоящее время не поддерживается!"
    );
#endif
    Game::ComponentId id = Game::GetComponentId<TYPE>();
#if NEBULA_DEBUG
    n_assert(MemDb::AttributeRegistry::TypeSize(id) == sizeof(TYPE));
#endif
    TYPE* data = this->componentStageAllocator.Alloc<TYPE>();
    const void* defaultValue = MemDb::AttributeRegistry::DefaultValue(id);
    Memory::Copy(defaultValue, data, sizeof(TYPE));
    AddStagedComponentCommand cmd = {
        .entity = entity,
        .componentId = id,
        .dataSize = sizeof(TYPE),
        .data = data,
    };
    this->addStagedQueue.Append(cmd);
    MemDb::Attribute* attr = MemDb::AttributeRegistry::GetAttribute(id);
    ComponentInterface* cInterface;
    cInterface = static_cast<ComponentInterface*>(attr);
    if (cInterface->Init != nullptr)
    {
        cInterface->Init(this, entity, data);
    }
    return data;
}
template <typename TYPE>
inline void
World::AddComponent(Entity entity, TYPE const& component)
{
#if NEBULA_DEBUG
    n_assert2(
        !this->pipeline.IsRunningAsync(), "Добавление компонента к объектам в асинхронном процессоре в настоящее время не поддерживается!"
    );
#endif
    Game::ComponentId id = Game::GetComponentId<TYPE>();
#if NEBULA_DEBUG
    n_assert(MemDb::AttributeRegistry::TypeSize(id) == sizeof(TYPE));
#endif
    TYPE* data = this->componentStageAllocator.Alloc<TYPE>();
    Memory::Copy(&component, data, sizeof(TYPE));
    AddStagedComponentCommand cmd = {
        .entity = entity,
        .componentId = id,
        .dataSize = sizeof(TYPE),
        .data = data,
    };
    this->addStagedQueue.Append(cmd);
    MemDb::Attribute* attr = MemDb::AttributeRegistry::GetAttribute(id);
    ComponentInterface* cInterface;
    cInterface = static_cast<ComponentInterface*>(attr);
    if (cInterface->Init != nullptr)
    {
        cInterface->Init(this, entity, data);
    }
}
template <>
Game::Position World::GetComponent(Entity entity);
template <>
Game::Orientation World::GetComponent(Entity entity);
template <>
Game::Scale World::GetComponent(Entity entity);
template <>
void World::SetComponent(Entity entity, Game::Position const&);
template <>
void World::SetComponent(Entity entity, Game::Orientation const&);
template <>
void World::SetComponent(Entity entity, Game::Scale const&);
}