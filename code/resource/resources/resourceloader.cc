//------------------------------------------------------------------------------
// resourceloader.cc
// (C) 2017-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "resourceloader.h"
#include "io/ioserver.h"
#include "resourceserver.h"
#include "util/bit.h"

using namespace IO;
namespace Resources
{

__ImplementAbstractClass(Resources::ResourceLoader, 'RSLO', Core::RefCounted);
//------------------------------------------------------------------------------
/**
*/
ResourceLoader::ResourceLoader()
    : async(false)
{
    // может это самонадеянно, всего 1024 ожидающих ресурса (то есть фактических ресурсов) на один загрузчик?
    this->pendingLoads.Reserve(1024);
    this->pendingStreamLods.Reserve(1024);
    this->pendingStreamQueue.SetSignalOnEnqueueEnabled(false);
    this->creatorThread = Threading::Thread::GetMyThreadId();
}

//------------------------------------------------------------------------------
/**
*/
ResourceLoader::~ResourceLoader()
{
    // empty
}

//------------------------------------------------------------------------------
/**

*/
void
ResourceLoader::Setup()
{
    // реализовать настройки, специфичные для загрузчика, такие как идентификаторы ресурсов-заполнителей и ошибок, а также приемлемый класс ресурсов.
    this->uniqueResourceId = 0;

    // установите флаг async в конструкторе реализации вашего подкласса пула ресурсов
    if (this->async)
    {
        this->streamerThread = ResourceLoaderThread::Create();
        this->streamerThread->SetName(this->streamerThreadName.Value());
        this->streamerThread->Start();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceLoader::Discard()
{
    this->streamerThread->Stop();
    this->streamerThread = nullptr;
}

//------------------------------------------------------------------------------
/**
*/
void 
ResourceLoader::LoadFallbackResources()
{
    // загрузить все заполнители
    for (IndexT i = 0; i < this->placeholders.Size(); i++)
    {
        this->placeholders[i].placeholderId = this->CreateResource(this->placeholders[i].placeholderName, nullptr, 0, "system"_atm, nullptr, nullptr, true, false);
        n_assert(this->placeholders[i].placeholderId != Resources::InvalidResourceId);
    }

    // загрузить заполнитель, не загружайте его асинхронно
    if (this->placeholderResourceName.IsValid())
    {
        this->placeholderResourceId = this->CreateResource(this->placeholderResourceName, nullptr, 0, "system"_atm, nullptr, nullptr, true, false);
        n_assert(this->placeholderResourceId != Resources::InvalidResourceId);
    }

    // ошибка загрузки, не загружайте асинхронно
    if (this->failResourceName.IsValid())
    {
        this->failResourceId = this->CreateResource(this->failResourceName, nullptr, 0, "system"_atm, nullptr, nullptr, true, false);
        n_assert(this->failResourceId != Resources::InvalidResourceId);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceLoader::UpdateLoaderSyncState()
{
    // Empty
}

//------------------------------------------------------------------------------
/**
*/
ResourceLoader::ResourceStreamOutput
ResourceLoader::StreamResource(const ResourceLoadJob& job)
{
    // Предположим, что загрузчик не поддерживает потоковую передачу, при которой все данные загружаются при инициализации.
    return ResourceStreamOutput{ .loadedBits = 0xFFFFFFFF, .pendingBits = 0x0 };
}

//------------------------------------------------------------------------------
/**
*/
Resource::State
ResourceLoader::ReloadFromStream(const Resources::ResourceId id, const Ptr<IO::Stream>& stream)
{
    // Предположим, что загрузчик не поддерживает потоковую передачу, при которой все данные загружаются при инициализации.
    return Resource::Failed;
}

//------------------------------------------------------------------------------
/**
*/
uint
ResourceLoader::LodMask(const _StreamData& stream, float lod, bool async) const
{
    return 0xFFFFFFFF;
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceLoader::RequestLOD(const Ids::Id32 entry, float lod) const
{
    // ничего не делать
}

//------------------------------------------------------------------------------
/**
*/
void
ApplyLoadOutput(ResourceLoader* loader, const ResourceLoader::ResourceLoadOutput& output)
{
    output.UpdateLoaderState(loader);
    if (output.state == Resource::Loaded || output.state == Resource::Failed)
        loader->RunCallbacks(output.state, output.id);
    else
        loader->dependentJobs.Append(output.remainderJob);
}

//------------------------------------------------------------------------------
/**
*/
void
DispatchJob(ResourceLoader* loader, const ResourceLoader::ResourceLoadJob& job)
{
    if (loader->async && !job.immediate)
    {
        // Создать и отправить задание в поток
        auto jobFunc = [loader, job]() -> void
        {
            ResourceLoader::ResourceLoadOutput output = _LoadInternal(loader, job);
            loader->loadOutputs.Enqueue(output);
        };
        loader->EnqueueJob(jobFunc);
    }
    else
    {
        // Выполните немедленную загрузку и обновите состояние загрузчика.
        ResourceLoader::ResourceLoadOutput output = _LoadInternal(loader, job);
        ApplyLoadOutput(loader, output);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceLoader::ClearPendingUnloads()
{
    // пройти через ожидающие разгрузки
    for (IndexT i = this->pendingUnloads.Size() - 1; i >= 0; i--)
    {
        const _PendingResourceUnload& unload = this->pendingUnloads[i];
        if (this->states[unload.resourceId.loaderInstanceId] == Resource::Loaded)
        {
            n_assert(this->usage[unload.resourceId.loaderInstanceId] >= 0);
            this->usage[unload.resourceId.loaderInstanceId]--;
            if (this->usage[unload.resourceId.loaderInstanceId] == 0)
            {
                // выгрузить, если загружено
                this->states[unload.resourceId.loaderInstanceId] = Resource::Unloaded;
                this->Unload(unload.resourceId);

                Memory::Free(Memory::ScratchHeap, this->metaData[unload.resourceId.loaderInstanceId].data);

                // скиньте идентификатор ресурса
                this->resourceInstanceIndexPool.Dealloc(unload.resourceId.loaderInstanceId);
            }

            // remove pending unload if not Pending or Loaded (so explicitly Unloaded or Failed)
            this->pendingUnloads.EraseIndex(i);
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceLoader::Update(IndexT frameIndex)
{
    // Обновить состояние ресурсов туда и обратно.
    this->UpdateLoaderSyncState();

    // Перебирать асинхронные выходные данные и обновлять состояние загрузчика.
    Util::Array<ResourceLoadOutput, 128> asyncOutputs;
    this->loadOutputs.DequeueAll(asyncOutputs);
    for (auto output : asyncOutputs)
    {
        ApplyLoadOutput(this, output);
    }

    // Сделайте копию, поскольку ImmediateJob может добавить задания в список зависимых заданий.
    Util::FixedArray<ResourceLoadJob, true> dependencyJobs = this->dependentJobs;
    this->dependentJobs.Clear();
    for (const auto& job : dependencyJobs)
    {
        DispatchJob(this, job);
    }

    ClearPendingUnloads();

    for (IndexT i = 0; i < this->pendingLoads.Size(); i++)
    {
        _PendingResourceLoad& resourceLoad = this->loads[this->pendingLoads[i]];
        Resource::State state = this->states[resourceLoad.entry];

        // Пропустить множество уже готовых ресурсов
        n_assert(state == Resource::Pending);
        if (state == Resource::Loaded || state == Resource::Failed)
            continue;

        LoadState loadState = this->loadStates[resourceLoad.entry];
        ResourceLoadJob job = ResourceLoadJob::FromPending(this, frameIndex, resourceLoad);
        DispatchJob(this, job);

        resourceLoad.flags = LoadFlags::None;
    }
    this->pendingLoads.Clear();

    // пройти через ожидающие потоки lod
    Util::Array<_PendingStreamLod, 128> pendingStreams;
    this->pendingStreamQueue.DequeueAll(pendingStreams);
    this->pendingStreamLods.AppendArray(pendingStreams.Begin(), pendingStreams.Size()); 
    for (IndexT i = this->pendingStreamLods.Size() - 1; i >= 0; i--)
    {
        const _PendingStreamLod& streamLod = this->pendingStreamLods[i];

        if (this->states[streamLod.id.loaderInstanceId] == Resource::Loaded)
        {
            this->states[streamLod.id.loaderInstanceId] = Resource::Pending;

            _PendingResourceLoad& load = this->loads[streamLod.id.loaderInstanceId];
            load.lod = streamLod.lod;
            load.flags |= LoadFlags::Update;

            // Обновите состояние, чтобы продолжить трансляцию.
            this->pendingLoads.Append(streamLod.id.loaderInstanceId);

            this->pendingStreamLods.EraseIndex(i);
            i--;
        }
        else if (this->states[streamLod.id.loaderInstanceId] == Resource::Unloaded)
        {
            // Если ресурс был выгружен до начала потоковой передачи, удалите запрос.
            this->pendingStreamLods.EraseIndex(i);
            i--;
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceLoader::SetupIdFromEntry(const Ids::Id32 entry, ResourceId& cacheEntry)
{
    const Resource::State state = this->states[entry];
    switch (state)
    {
        case Resource::State::Loaded:
            cacheEntry.resourceId = this->resources[entry].resourceId;
            cacheEntry.generation = this->resources[entry].generation;
            break;
        case Resource::State::Failed:
            cacheEntry.resourceId = this->failResourceId.resourceId;
            cacheEntry.generation = this->failResourceId.generation;
            break;
        case Resource::State::Pending:
        case Resource::State::Unloaded:
            cacheEntry.resourceId = this->placeholderResourceId.resourceId;
            cacheEntry.generation = this->placeholderResourceId.generation;
            break;
    }
}

//------------------------------------------------------------------------------
/**
    Все обратные вызовы, ожидающие выполнения на ресурсе, должны находиться в критическом разделе!
*/
void
ResourceLoader::RunCallbacks(Resource::State status, const Resources::ResourceId id)
{
    Util::Array<_Callbacks>& cbls = this->callbacks[id.loaderInstanceId];
    IndexT i;
    for (i = cbls.Size() - 1; i >= 0; i--)
    {
        _Callbacks& cbl = cbls[i];
        if (status == Resource::Loaded && cbl.success != nullptr)	
            cbl.success(id);
        else if (status == Resource::Failed && cbl.failed != nullptr)
            cbl.failed(id);
        cbls.EraseIndexSwap(i);
        cbl = {};
    }
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceLoader::EnqueueJob(const std::function<void()>& func)
{
    this->streamerThread->jobs.Enqueue(func);
}

//------------------------------------------------------------------------------
/**
*/
Resources::ResourceId
ResourceLoader::GetPlaceholder(const Resources::ResourceName& name)
{
    n_assert(this->placeholders.Size() > 0);
    return this->placeholders[0].placeholderId;
}

//------------------------------------------------------------------------------
/**
*/
ResourceLoader::ResourceLoadOutput
_LoadInternal(ResourceLoader* loader, ResourceLoader::ResourceLoadJob job)
{
    ResourceLoader::ResourceStreamOutput streamResult;
    streamResult.pendingBits = job.loadState.pendingBits;
    streamResult.loadedBits = job.loadState.loadedBits;

    if (AllBits(job.flags, LoadFlags::Create))
    {
        // построить поток
        Ptr<Stream> stream = IO::IoServer::Instance()->CreateStream(job.name.AsCharPtr());
        stream->SetAccessMode(Stream::ReadAccess);
        if (stream->Open())
        {
            // Если новый ресурс, инициализируйте его
            ResourceLoader::ResourceInitOutput initResult = loader->InitializeResource(job, stream);
            job.streamData = initResult.loaderStreamData;
            job.id.resourceId = initResult.id.resourceId;
            job.id.generation = initResult.id.generation;

            // Получите запрошенные биты загрузки на основе данных потока.
            job.loadState.requestedBits = loader->LodMask(job.streamData, job.lod, !job.immediate);
            job.flags = LoadFlags::None;

            if (job.immediate)
            {
                if (initResult.id != InvalidResourceUnknownId)
                {
                    job.loadState.loadedBits = job.loadState.requestedBits;
                    job.state = Resource::Loaded;
                }
                else
                {
                    job.loadState.loadedBits = 0;
                    job.state = Resource::Failed;
                }
                goto skip_stream;
            }

            if (initResult.id == InvalidResourceUnknownId)
            {
                // Если инициализация не удалась, это означает, что файл недействителен или его невозможно найти.
                job.state = Resource::Failed;
                job.id.resourceId = loader->failResourceId.resourceId;
                job.id.generation = loader->failResourceId.generation;
                n_printf("[Загрузчик ресурсов] Не удалось загрузить ресурс %s\n", job.name.AsCharPtr());
                goto skip_stream;
            }
        }
        else
        {
            job.id.resourceId = loader->failResourceId.resourceId;
            job.id.generation = loader->failResourceId.generation;
            n_printf("[Загрузчик ресурсов] Не удалось открыть ресурс %s\n", job.name.AsCharPtr());
            job.state = Resource::Failed;
            goto skip_stream;
        }
    }

    if (AllBits(job.flags, LoadFlags::Update))
    {
        job.loadState.requestedBits |= loader->LodMask(job.streamData, job.lod, true);
    }

    if (job.loadState.requestedBits != job.loadState.loadedBits)
    {
        // В случае успеха начать потоковую передачу данных.
        streamResult = loader->StreamResource(job);
        job.loadState.pendingBits = streamResult.pendingBits;
        job.loadState.loadedBits = streamResult.loadedBits;
    }

    if (AllBits(streamResult.loadedBits, job.loadState.requestedBits))
    {
        job.state = Resource::Loaded;
    }
    else
    {
        job.state = Resource::Pending;
    }

skip_stream:
    /// Поставить в очередь вывод загрузки ресурсов
    ResourceLoader::ResourceLoadOutput output;
    output.streamData = job.streamData;
    output.loadState = job.loadState;
    output.state = job.state;
    output.id = job.id;
    output.remainderJob = job;

    return output;
}

//------------------------------------------------------------------------------
/**
*/
Resources::ResourceId
Resources::ResourceLoader::CreateResource(const ResourceName& res, const void* loadInfo, SizeT loadInfoSize, const Util::StringAtom& tag, std::function<void(const Resources::ResourceId)> success, std::function<void(const Resources::ResourceId)> failed, bool immediate, bool stream)
{
    // это утверждение, возможно, следует удалить в пользу помещения вещей в очередь, если оно вызывается из другого потока
    n_assert(Threading::Thread::GetMyThreadId() == this->creatorThread);

    // Сохраните путь к файлу как идентификатор файла.
    IO::URI path(res.Value());
    IndexT i = this->ids.FindIndex(path.GetHostAndLocalPath());

    // Установите возвращаемое значение в качестве заполнителя по умолчанию
    ResourceId ret = this->placeholderResourceId;

    if (i == InvalidIndex)
    {
        // выделить новый индекс для распределителя
        Ids::Id32 instanceId = this->resourceInstanceIndexPool.Alloc(); // это идентификатор контейнера

        // создайте новый идентификатор ресурса, при необходимости увеличьте список контейнеров
        if (instanceId >= (uint)this->names.Size())
        {
            this->names.Resize(this->names.Size() + ResourceIndexGrow);
            this->usage.Resize(this->usage.Size() + ResourceIndexGrow);
            this->tags.Resize(this->tags.Size() + ResourceIndexGrow);
            this->states.Resize(this->states.Size() + ResourceIndexGrow);
            this->loadStates.Resize(this->loadStates.Size() + ResourceIndexGrow);
            this->resources.Resize(this->resources.Size() + ResourceIndexGrow);
            this->callbacks.Resize(this->callbacks.Size() + ResourceIndexGrow);
            this->loads.Resize(this->loads.Size() + ResourceIndexGrow);
            this->metaData.Resize(this->metaData.Size() + ResourceIndexGrow);
            this->streamDatas.Resize(this->streamDatas.Size() + ResourceIndexGrow);
        }

        // добавить имя ресурса к идентификатору ресурса
        this->names[instanceId] = path.GetHostAndLocalPath();
        this->usage[instanceId] = 1;
        this->tags[instanceId] = tag;
        this->states[instanceId] = Resource::Pending;

        this->loadStates[instanceId] = LoadState{ .requestedBits = 0xFFFFFFFF, .pendingBits = 0x0, .loadedBits = 0x0 };

        // выделить метаданные, если они есть
        _LoadMetaData metaData;
        if (loadInfo != nullptr)
        {
            metaData.data = Memory::Alloc(Memory::ScratchHeap, loadInfoSize);
            metaData.size = loadInfoSize;
            memcpy(metaData.data, loadInfo, loadInfoSize);
        }
        else
        {
            metaData.data = nullptr;
            metaData.size = 0;
        }
        this->metaData[instanceId] = metaData;

        ret.loaderInstanceId = instanceId;
        ret.loaderIndex = this->uniqueId;
        this->resources[instanceId] = ret;

        _PendingResourceLoad pending;
        pending.entry = instanceId;
        pending.tag = tag;
        pending.inflight = false;
        pending.immediate = immediate;
        pending.reload = false;
        pending.lod = stream ? 1.0f : 0.0f;
        pending.flags = LoadFlags::Create;
        this->loads[instanceId] = pending;

        // добавить сопоставление между именем ресурса и загружаемым ресурсом
        this->ids.Add(path.GetHostAndLocalPath(), instanceId);

        if (immediate)
        {
            ResourceLoadJob job = ResourceLoadJob::FromPending(this, -1, pending);
            ResourceLoadOutput output = _LoadInternal(this, job);
            output.UpdateLoaderState(this);
            SetupIdFromEntry(output.id.loaderInstanceId, ret);
            if (output.state == Resource::Loaded && success != nullptr)
                success(ret);
            else if (output.state == Resource::Failed && failed != nullptr)
                failed(ret);
        }
        else
        {
            this->pendingLoads.Append(instanceId);
            if (success != nullptr || failed != nullptr)
            {
                // нам не нужно беспокоиться о теме, поскольку этот ресурс новый
                this->callbacks[instanceId].Append({ success, failed });
            }
        }
    }
    else // this means the resource container is already created, and it may or may not be pending
    {
        // Получить идентификатор ранее созданного ресурса
        Ids::Id32 instanceId = this->ids.ValueAtIndex(i);

        // резкое использование
        this->usage[instanceId]++;

        // запустите раздел async, загрузчик может изменить состояние ресурса
        ret = this->resources[instanceId];
        
        // Если ресурс не находится в режиме ожидания, немедленно вызовите обратный вызов сбоя или успеха.
        const Resource::State state = this->states[instanceId];
        if (state == Resource::Loaded && success != nullptr)
            success(ret);
        else if (state == Resource::Failed && failed != nullptr)
            failed(ret);
        else if (state == Resource::Pending)
        {
            // этот ресурс теперь должен быть в списке ожидания
            n_assert(i != InvalidIndex);

            // ожидающий ресурс может быть неактивен в потоке
            _PendingResourceLoad& pend = this->loads[instanceId];
            if (!pend.inflight)
            {
                // переверните флаг немедленной загрузки, это на тот случай, если мы решим выполнить более позднюю загрузку, используя немедленное переопределение
                pend.immediate = pend.immediate || immediate;
            }

            // поскольку мы находимся в состоянии ожидания и находимся внутри раздела async, это означает, что ресурс еще не загружен, а это означает, что можно безопасно добавить обратный вызов
            this->callbacks[instanceId].Append({ success, failed });
        }
        else if (state == Resource::Unloaded)
        {
            // выделить метаданные, если они есть
            _LoadMetaData metaData;
            if (loadInfo != nullptr)
            {
                metaData.data = Memory::Alloc(Memory::ScratchHeap, loadInfoSize);
                metaData.size = loadInfoSize;
                memcpy(metaData.data, loadInfo, loadInfoSize);
            }
            else
            {
                metaData.data = nullptr;
                metaData.size = 0;
            }
            this->metaData[instanceId] = metaData;

            _PendingResourceLoad pending;
            pending.entry = instanceId;
            pending.tag = tag;
            pending.inflight = false;
            pending.immediate = immediate;
            pending.reload = false;
            pending.lod = stream ? 1.0f : 0.0f;
            pending.flags = LoadFlags::Create;
            this->states[instanceId] = Resource::Pending;
            this->loads[instanceId] = pending;

            if (immediate)
            {
                ResourceLoadJob job = ResourceLoadJob::FromPending(this, -1, pending);
                ResourceLoadOutput output = _LoadInternal(this, job);
                output.UpdateLoaderState(this);
                SetupIdFromEntry(output.id.loaderInstanceId, ret);
                if (output.state == Resource::Loaded && success != nullptr)
                {
                    success(ret);
                }
                else if (output.state == Resource::Failed && failed != nullptr)
                {
                    ret = this->failResourceId;
                    failed(ret);
                }
            }
            else
            {
                this->pendingLoads.Append(instanceId);
                if (success != nullptr || failed != nullptr)
                {
                    // if unloaded, the callbacks array can safetly be assumed to be empty
                    this->callbacks[instanceId].Append({ success, failed });
                }
            }
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
*/
void
Resources::ResourceLoader::DiscardResource(const Resources::ResourceId id)
{
    n_assert(Threading::Thread::GetMyThreadId() == this->creatorThread);
    if (id != this->placeholderResourceId && id != this->failResourceId)
    {
        // добавить ожидающую выгрузку, она будет выгружена после загрузки
        this->pendingUnloads.Append({ id });
    }
#if N_DEBUG
    else
    {
        n_warning("Попытка удалить заполнитель или сбой ресурса!\n");
    }
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceLoader::DiscardByTag(const Util::StringAtom& tag)
{
    n_assert(Threading::Thread::GetMyThreadId() == this->creatorThread);
    IndexT i;
    for (i = 0; i < this->tags.Size(); i++)
    {
        if (this->tags[i] == tag)
        {
            // добавить ожидающую выгрузку, она будет выгружена после загрузки
            this->pendingUnloads.Append({ this->resources[this->ids[this->names[i]]] });
            this->tags[i] = "";
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceLoader::CreateListener(
    const Resources::ResourceId res,
    std::function<void(const Resources::ResourceId)> success,
    std::function<void(const Resources::ResourceId)> failed
)
{
    // это утверждение, возможно, следует удалить в пользу помещения вещей в очередь, если оно вызывается из другого потока
    n_assert(Threading::Thread::GetMyThreadId() == this->creatorThread);

    n_assert(this->states.Size() > res.loaderInstanceId);
    const auto state = this->states[res.loaderInstanceId];
    if (state == Resource::Loaded)
    {
        if (success != nullptr)
            success(res);
    }
    else if (state == Resource::Failed)
    {
        if (failed != nullptr)
            failed(this->failResourceId);
    }
    else
    {
        this->callbacks[res.loaderInstanceId].Append(_Callbacks {success, failed});
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
ResourceLoader::ReloadResource(const Resources::ResourceName& res, std::function<void(const Resources::ResourceId)> success, std::function<void(const Resources::ResourceId)> failed)
{
    n_assert(Threading::Thread::GetMyThreadId() == this->creatorThread);
    IndexT i = this->ids.FindIndex(res);
    if (i != InvalidIndex)
    {
        // получить идентификатор ресурса
        Resources::ResourceId ret = this->ids.ValueAtIndex(i);

        _PendingResourceLoad pending;
        pending.entry = ret.loaderInstanceId;
        pending.tag = this->tags[ret.loaderInstanceId];
        pending.inflight = false;
        pending.immediate = false;
        pending.reload = true;
        pending.lod = 1.0f;
        pending.flags = LoadFlags::Create;

        this->loads[ret.loaderInstanceId] = pending;
        this->states[ret.loaderInstanceId] = Resource::Pending;
        this->callbacks[ret.loaderInstanceId].Append({ success, failed });
    }
    else
    {
        n_warning("Ресурс '%s' должен быть загружен, прежде чем его можно будет перезагрузить\n", res.AsString().AsCharPtr());
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
ResourceLoader::ReloadResource(const Resources::ResourceId& id, std::function<void(const Resources::ResourceId)> success, std::function<void(const Resources::ResourceId)> failed)
{
    n_assert(Threading::Thread::GetMyThreadId() == this->creatorThread);
    n_assert_fmt(id != Resources::ResourceId::Invalid(), "Ресурс %d не загружен, он должен быть загружен, прежде чем его можно будет перезагрузить.", id.HashCode());

    _PendingResourceLoad pending;
    pending.entry = id.loaderInstanceId;
    pending.tag = this->tags[id.loaderInstanceId];
    pending.inflight = false;
    pending.immediate = false;
    pending.reload = true;
    pending.lod = 1.0f;
    pending.flags = LoadFlags::Create;

    this->loads[id.loaderInstanceId] = pending;
    this->states[id.loaderInstanceId] = Resource::Pending;
    this->callbacks[id.loaderInstanceId].Append({ success, failed });
}

//------------------------------------------------------------------------------
/**
*/
void 
ResourceLoader::SetMinLod(const Resources::ResourceId& id, const float lod, bool immediate)
{
    if (immediate)
    {
        n_assert(Threading::Thread::GetMyThreadId() == this->creatorThread);
        _PendingResourceLoad& load = this->loads[id.loaderInstanceId];
        load.lod = lod;
        load.flags |= LoadFlags::Update;
        load.immediate = immediate;
        ResourceLoadJob job = ResourceLoadJob::FromPending(this, -1, load);
        ResourceLoadOutput output = _LoadInternal(this, job);
        output.UpdateLoaderState(this);
    }
    else
    {
        _PendingStreamLod pending;
        pending.id = id;
        pending.lod = lod;
        pending.immediate = immediate;
        this->pendingStreamQueue.Enqueue(pending);
    }
}

} // Ресурсы пространства имен
