#pragma once
#include "ids/id.h"
#include "util/stringatom.h"
#include "io/stream.h"
#include "util/set.h"
#include "resource.h"
#include "threading/safequeue.h"
#include "threading/threadid.h"
#include "ids/idpool.h"
#include <tuple>
#include <functional>
namespace Resources
{
enum LoadFlags
{
    None = 0x0,
    Create = 0x1,
    Update = 0x2
};
__ImplementEnumBitOperators(LoadFlags);
class Resource;
class ResourceLoaderThread;
class ResourceLoader : public Core::RefCounted
{
    __DeclareAbstractClass(ResourceLoader);
public:
    ResourceLoader();
    virtual ~ResourceLoader();
    virtual void Setup();
    virtual void Discard();
    virtual void LoadFallbackResources();
    void ClearPendingUnloads();
    Resources::ResourceId CreateResource(const Resources::ResourceName& res, const void* loadInfo, SizeT loadInfoSize, const Util::StringAtom& tag, std::function<void(const Resources::ResourceId)> success, std::function<void(const Resources::ResourceId)> failed, bool immediate, bool stream);
    void DiscardResource(const Resources::ResourceId id);
    void DiscardByTag(const Util::StringAtom& tag);
    void CreateListener(const Resources::ResourceId res, std::function<void(const Resources::ResourceId)> success, std::function<void(const Resources::ResourceId)> failed);
    const Resources::ResourceName& GetName(const Resources::ResourceId id) const;
    const uint32_t GetUsage(const Resources::ResourceId id) const;
    const Util::StringAtom GetTag(const Resources::ResourceId id) const;
    const Resource::State GetState(const Resources::ResourceId id) const;
    const Resources::ResourceId GetId(const Resources::ResourceName& name) const;
    const Util::Dictionary<Resources::ResourceName, Ids::Id32>& GetResources() const;
    const bool HasResource(const Resources::ResourceId id) const;
    const int32_t& GetUniqueId() const;
    void ReloadResource(const Resources::ResourceName& res, std::function<void(const Resources::ResourceId)> success, std::function<void(const Resources::ResourceId)> failed);
    void ReloadResource(const Resources::ResourceId& id, std::function<void(const Resources::ResourceId)> success, std::function<void(const Resources::ResourceId)> failed);
    void SetMinLod(const Resources::ResourceId& id, const float lod, bool immediate);
    struct _PendingResourceLoad
    {
        Ids::Id32 entry;
        Util::StringAtom tag;
        bool inflight;
        bool immediate;
        bool reload;
        IndexT frame;
        float lod;
        LoadFlags flags;
        _PendingResourceLoad() : entry(-1) {};
    };
    struct _LoadMetaData
    {
        void* data;
        SizeT size;
    };
    struct _StreamData
    {
        Ptr<IO::Stream> stream;
        void* data;
    };
    struct ResourceInitOutput
    {
        ResourceUnknownId id;
        _StreamData loaderStreamData;
        ResourceInitOutput()
            : id(InvalidResourceUnknownId)
        {
        }
    };
    struct ResourceStreamOutput
    {
        uint loadedBits = 0x0;
        uint pendingBits = 0x0;
    };
    struct LoadState
    {
        uint requestedBits;
        uint pendingBits;
        uint loadedBits;
    };
    struct ResourceLoadJob
    {
        Util::String name;
        Util::String tag;
        Resource::State state;
        ResourceId id;
        LoadState loadState;
        _StreamData streamData;
        _LoadMetaData metadata;
        bool immediate;
        float lod;
        IndexT frameIndex;
        LoadFlags flags;
        static ResourceLoadJob FromPending(ResourceLoader* loader, IndexT frameIndex, const _PendingResourceLoad& load)
        {
            ResourceLoadJob job;
            job.name = loader->names[load.entry].Value();
            job.state = loader->states[load.entry];
            job.id = loader->resources[load.entry];
            job.loadState = loader->loadStates[load.entry];
            job.streamData = loader->streamDatas[load.entry];
            job.metadata = loader->metaData[load.entry];
            job.tag = load.tag.Value();
            job.immediate = load.immediate;
            job.lod = load.lod;
            job.frameIndex = frameIndex;
            job.flags = load.flags;
            return job;
        };
    };
    struct ResourceLoadOutput
    {
        _StreamData streamData;
        LoadState loadState;
        Resource::State state;
        ResourceId id;
        ResourceLoadJob remainderJob;
        void UpdateLoaderState(ResourceLoader* loader) const
        {
            loader->streamDatas[id.loaderInstanceId] = this->streamData;
            loader->loadStates[id.loaderInstanceId] = this->loadState;
            loader->states[id.loaderInstanceId] = this->state;
            loader->resources[id.loaderInstanceId] = this->id;
        }
    };
protected:
    friend class ResourceServer;
    friend void ApplyLoadOutput(ResourceLoader* loader, const ResourceLoader::ResourceLoadOutput& output);
    friend void DispatchJob(ResourceLoader* loader, const ResourceLoader::ResourceLoadJob& job);
    friend ResourceLoadOutput _LoadInternal(ResourceLoader* loader, ResourceLoadJob res);
    virtual void UpdateLoaderSyncState();
    struct _PendingStreamLod
    {
        Resources::ResourceId id;
        float lod;
        bool immediate;
        _PendingStreamLod() : id(ResourceId::Invalid()) {};
    };
    struct _PendingResourceUnload
    {
        Resources::ResourceId resourceId;
    };
    struct _Callbacks
    {
        std::function<void(const Resources::ResourceId)> success;
        std::function<void(const Resources::ResourceId)> failed;
    };
    static const uint32_t ResourceIndexGrow = 512;
    virtual ResourceInitOutput InitializeResource(const ResourceLoadJob& job, const Ptr<IO::Stream>& stream) = 0;
    virtual ResourceStreamOutput StreamResource(const ResourceLoadJob& job);
    virtual Resource::State ReloadFromStream(const Resources::ResourceId id, const Ptr<IO::Stream>& stream);
    virtual uint LodMask(const _StreamData& stream, float lod, bool async) const;
    virtual void RequestLOD(const Ids::Id32 entry, float lod) const;
    virtual void Unload(const Resources::ResourceId id) = 0;
    virtual void Update(IndexT frameIndex);
    void SetupIdFromEntry(const Ids::Id32 entry, ResourceId& cacheEntry);
    void RunCallbacks(Resource::State status, const Resources::ResourceId id);
    void EnqueueJob(const std::function<void()>& func);
    struct _PlaceholderResource
    {
        Resources::ResourceName placeholderName;
        Resources::ResourceId placeholderId;
    };
    Util::FixedArray<_PlaceholderResource> placeholders;
    Resources::ResourceId GetPlaceholder(const Resources::ResourceName& name);
    Util::StringAtom placeholderResourceName;
    Util::StringAtom failResourceName;
    Resources::ResourceId placeholderResourceId;
    Resources::ResourceId failResourceId;
    bool async;
    Ptr<ResourceLoaderThread> streamerThread;
    std::function<void()> preJobFunc;
    std::function<void()> postJobFunc;
    Util::StringAtom streamerThreadName;
    Util::Array<IndexT> pendingLoads;
    Util::Array<_PendingResourceUnload> pendingUnloads;
    Util::Array<_PendingStreamLod> pendingStreamLods;
    Threading::SafeQueue<_PendingStreamLod> pendingStreamQueue;
    Threading::SafeQueue<ResourceLoadOutput> loadOutputs;
    Util::Array<ResourceLoadJob> dependentJobs;
    Util::Dictionary<Resources::ResourceName, uint32_t> ids;
    Ids::IdPool resourceInstanceIndexPool;
    Util::FixedArray<Resources::ResourceName> names;
    Util::FixedArray<uint32_t> usage;
    Util::FixedArray<Util::StringAtom> tags;
    Util::FixedArray<Resource::State> states;
    Util::FixedArray<LoadState> loadStates;
    Util::FixedArray<ResourceId> resources;
    Util::FixedArray<Util::Array<_Callbacks>> callbacks;
    Util::FixedArray<_PendingResourceLoad> loads;
    Util::FixedArray<_LoadMetaData> metaData;
    Util::FixedArray<_StreamData> streamDatas;
    uint32_t uniqueResourceId;
    int32_t uniqueId;
    Threading::CriticalSection asyncSection;
    Threading::ThreadId creatorThread;
};
inline const Resources::ResourceName&
ResourceLoader::GetName(const Resources::ResourceId id) const
{
    return this->names[id.loaderInstanceId];
}
inline const uint32_t
ResourceLoader::GetUsage(const Resources::ResourceId id) const
{
    return this->usage[id.loaderInstanceId];
}
inline const Util::StringAtom
ResourceLoader::GetTag(const Resources::ResourceId id) const
{
    return this->tags[id.loaderInstanceId];
}
inline const Resources::Resource::State
ResourceLoader::GetState(const Resources::ResourceId id) const
{
    return this->states[id.loaderInstanceId];
}
inline const Resources::ResourceId
ResourceLoader::GetId(const Resources::ResourceName& name) const
{
    IndexT i = this->ids.FindIndex(name);
    if (i == InvalidIndex)  return Resources::ResourceId::Invalid();
    else                    return this->resources[this->ids.ValueAtIndex(i)];
}
inline const Util::Dictionary<Resources::ResourceName, Ids::Id32>&
ResourceLoader::GetResources() const
{
    return this->ids;
}
inline const bool
ResourceLoader::HasResource(const Resources::ResourceId id) const
{
    return this->names.Size() > (SizeT)id.loaderInstanceId;
}
inline const int32_t&
ResourceLoader::GetUniqueId() const
{
    return this->uniqueId;
}
}