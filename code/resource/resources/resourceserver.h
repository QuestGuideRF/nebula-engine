#pragma once
#include <functional>
#include "core/refcounted.h"
#include "ids/id.h"
#include "core/singleton.h"
#include "resourceid.h"
#include "resourceloader.h"
#include "resourceloaderthread.h"
namespace Resources
{
class ResourceServer : public Core::RefCounted
{
    __DeclareClass(ResourceServer);
    __DeclareInterfaceSingleton(ResourceServer);
public:
    ResourceServer();
    virtual ~ResourceServer();
    void Open();
    void Close();
    void Update(IndexT frameIndex);
    Resources::ResourceId CreateResource(const ResourceName& res, std::function<void(const Resources::ResourceId)> success = nullptr, std::function<void(const Resources::ResourceId)> failed = nullptr, bool immediate = false, bool stream = true);
    Resources::ResourceId CreateResource(const ResourceName& res, const Util::StringAtom& tag, std::function<void(const Resources::ResourceId)> success = nullptr, std::function<void(const Resources::ResourceId)> failed = nullptr, bool immediate = false, bool stream = true);
    template<class METADATA> Resources::ResourceId CreateResource(const ResourceName& res, const METADATA& metaData, std::function<void(const Resources::ResourceId)> success = nullptr, std::function<void(const Resources::ResourceId)> failed = nullptr, bool immediate = false, bool stream = true);
    template<class METADATA> Resources::ResourceId CreateResource(const ResourceName& res, const METADATA& metaData, const Util::StringAtom& tag, std::function<void(const Resources::ResourceId)> success = nullptr, std::function<void(const Resources::ResourceId)> failed = nullptr, bool immediate = false, bool stream = true);
    void DiscardResource(const Resources::ResourceId res);
    void DiscardResources(const Util::StringAtom& tag);
    bool HasPendingResources();
    void ReloadResource(const ResourceName& res, std::function<void(const Resources::ResourceId)> success = nullptr, std::function<void(const Resources::ResourceId)> failed = nullptr);
    void SetMinLod(const ResourceId& id, float lod, bool immediate);
    void CreateResourceListener(const ResourceId& id, std::function<void(const Resources::ResourceId)> success, std::function<void(const Resources::ResourceId)> failed = nullptr);
    Core::Rtti* GetType(const Resources::ResourceId id);
    const Resources::ResourceName GetName(const Resources::ResourceId id) const;
    const Util::StringAtom GetTag(const Resources::ResourceId id) const;
    const Resource::State GetState(const Resources::ResourceId id) const;
    const SizeT GetUsage(const Resources::ResourceId id) const;
    bool HasResource(const Resources::ResourceId id) const;
    const Resources::ResourceId GetId(const Resources::ResourceName& name) const;
    void RegisterStreamLoader(const Util::StringAtom& ext, const Core::Rtti& loaderClass);
    void DeregisterStreamLoader(const Util::StringAtom& ext, const Core::Rtti& loaderClass);
    template <class POOL_TYPE> POOL_TYPE* GetStreamLoader() const;
    void WaitForLoaderThread();
    void LoadDefaultResources();
private:
    friend class ResourceLoader;
    bool open;
    Util::Dictionary<Util::StringAtom, IndexT> extensionMap;
    Util::Dictionary<const Core::Rtti*, IndexT> typeMap;
    Util::Array<Ptr<ResourceLoader>> loaders;
    static int32_t UniquePoolCounter;
};
inline Resources::ResourceId
Resources::ResourceServer::CreateResource(
    const ResourceName& id
    , std::function<void(const Resources::ResourceId)> success
    , std::function<void(const Resources::ResourceId)> failed
    , bool immediate
    , bool stream
)
{
    return this->CreateResource(id, "", success, failed, immediate, stream);
}
inline Resources::ResourceId
Resources::ResourceServer::CreateResource(
    const ResourceName& res
    , const Util::StringAtom& tag
    , std::function<void(const Resources::ResourceId)> success
    , std::function<void(const Resources::ResourceId)> failed
    , bool immediate
    , bool stream
)
{
    Util::String ext = res.AsString().GetFileExtension();
    IndexT i = this->extensionMap.FindIndex(ext);
    n_assert_fmt(i != InvalidIndex, "С расширением файла «%s» не связан ни один загрузчик ресурсов.", ext.AsCharPtr());
    const Ptr<ResourceLoader>& loader = this->loaders[this->extensionMap.ValueAtIndex(i)].downcast<ResourceLoader>();
    Resources::ResourceId id = loader->CreateResource(res, nullptr, 0, tag, success, failed, immediate, stream);
    return id;
}
template<class METADATA>
inline Resources::ResourceId
ResourceServer::CreateResource(
    const ResourceName& res
    , const METADATA& metaData
    , std::function<void(const Resources::ResourceId)> success
    , std::function<void(const Resources::ResourceId)> failed
    , bool immediate
    , bool stream
)
{
    return this->CreateResource(res, metaData, "", success, failed, immediate, stream);
}
template<class METADATA>
inline Resources::ResourceId
ResourceServer::CreateResource(
    const ResourceName& res
    , const METADATA& metaData
    , const Util::StringAtom& tag
    , std::function<void(const Resources::ResourceId)> success
    , std::function<void(const Resources::ResourceId)> failed
    , bool immediate
    , bool stream
)
{
    Util::String ext = res.AsString().GetFileExtension();
    IndexT i = this->extensionMap.FindIndex(ext);
    n_assert_fmt(i != InvalidIndex, "С расширением файла «%s» не связан ни один загрузчик ресурсов.", ext.AsCharPtr());
    const Ptr<ResourceLoader>& loader = this->loaders[this->extensionMap.ValueAtIndex(i)].downcast<ResourceLoader>();
    Resources::ResourceId id = loader->CreateResource(res, &metaData, sizeof(METADATA), tag, success, failed, immediate, stream);
    return id;
}
inline void
ResourceServer::ReloadResource(const ResourceName& res, std::function<void(const Resources::ResourceId)> success, std::function<void(const Resources::ResourceId)> failed)
{
    Util::String ext = res.AsString().GetFileExtension();
    IndexT i = this->extensionMap.FindIndex(ext);
    n_assert_fmt(i != InvalidIndex, "С расширением файла «%s» не связан ни один загрузчик ресурсов.", ext.AsCharPtr());
    const Ptr<ResourceLoader>& loader = this->loaders[this->extensionMap.ValueAtIndex(i)].downcast<ResourceLoader>();
    loader->ReloadResource(res, success, failed);
}
inline void
ResourceServer::SetMinLod(const ResourceId& id, float lod, bool immediate)
{
    const Ids::Id8 loaderid = id.loaderIndex;
    n_assert(this->loaders.Size() > loaderid);
    const Ptr<ResourceLoader>& loader = this->loaders[loaderid].downcast<ResourceLoader>();
    loader->SetMinLod(id, lod, immediate);
}
inline void
ResourceServer::CreateResourceListener(
    const ResourceId& id,
    std::function<void(const Resources::ResourceId)> success,
    std::function<void(const Resources::ResourceId)> failed
)
{
    const Ids::Id8 loaderid = id.loaderIndex;
    n_assert(this->loaders.Size() > loaderid);
    const Ptr<ResourceLoader>& loader = this->loaders[loaderid].downcast<ResourceLoader>();
    loader->CreateListener(id, success, failed);
}
inline void
ResourceServer::DiscardResource(const Resources::ResourceId id)
{
    const Ids::Id8 loaderid = id.loaderIndex;
    n_assert(this->loaders.Size() > loaderid);
    const Ptr<ResourceLoader>& loader = this->loaders[loaderid].downcast<ResourceLoader>();
    loader->DiscardResource(id);
}
inline const Resources::ResourceName
ResourceServer::GetName(const Resources::ResourceId id) const
{
    n_assert(this->loaders.Size() > id.loaderIndex);
    const Ptr<ResourceLoader>& loader = this->loaders[id.loaderIndex];
    return loader->GetName(id.resourceId);
}
inline const Util::StringAtom
ResourceServer::GetTag(const Resources::ResourceId id) const
{
    n_assert(this->loaders.Size() > id.loaderIndex);
    const Ptr<ResourceLoader>& loader = this->loaders[id.loaderIndex];
    return loader->GetTag(id.resourceId);
}
inline const Resources::Resource::State
ResourceServer::GetState(const Resources::ResourceId id) const
{
    n_assert(this->loaders.Size() > id.loaderIndex);
    const Ptr<ResourceLoader>& loader = this->loaders[id.loaderIndex];
    return loader->GetState(id.resourceId);
}
inline const SizeT
ResourceServer::GetUsage(const Resources::ResourceId id) const
{
    n_assert(this->loaders.Size() > id.loaderIndex);
    const Ptr<ResourceLoader>& loader = this->loaders[id.loaderIndex];
    return loader->GetUsage(id.resourceId);
}
inline bool
ResourceServer::HasResource(const Resources::ResourceId id) const
{
    if (this->loaders.Size() <= id.loaderIndex) return false;
    {
        const Ptr<ResourceLoader>& loader = this->loaders[id.loaderIndex];
        if (loader->HasResource(id)) return true;
        return false;
    }
}
inline const Resources::ResourceId
ResourceServer::GetId(const Resources::ResourceName& name) const
{
    IndexT i;
    for (i = 0; i < this->loaders.Size(); i++)
    {
        Resources::ResourceId id = this->loaders[i]->GetId(name);
        if (id != Resources::ResourceId::Invalid()) return id;
    }
    return Resources::ResourceId::Invalid();
}
template <class POOL_TYPE>
inline POOL_TYPE*
ResourceServer::GetStreamLoader() const
{
    static_assert(std::is_base_of<ResourceLoader, POOL_TYPE>::value, "Запрошенный тип не является пулом потоков");
    IndexT i = this->typeMap.FindIndex(&POOL_TYPE::RTTI);
    if (i != InvalidIndex)
    {
        return static_cast<POOL_TYPE*>(this->loaders[this->typeMap.ValueAtIndex(i)].get());
    }
    n_error("Для этого типа не зарегистрирован загрузчик");
    return nullptr;
}
inline Resources::ResourceId
CreateResource(
    const ResourceName& res
    , const Util::StringAtom& tag
    , std::function<void(const Resources::ResourceId)> success = nullptr
    , std::function<void(const Resources::ResourceId)> failed = nullptr
    , bool immediate = false
    , bool stream = true
)
{
    return ResourceServer::Instance()->CreateResource(res, tag, success, failed, immediate, stream);
}
template <class METADATA>
inline Resources::ResourceId
CreateResource(
    const ResourceName& res
    , const METADATA& metaData
    , const Util::StringAtom& tag
    , std::function<void(const Resources::ResourceId)> success = nullptr
    , std::function<void(const Resources::ResourceId)> failed = nullptr
    , bool immediate = false
    , bool stream = true
)
{
    return ResourceServer::Instance()->CreateResource(res, metaData, tag, success, failed, immediate, stream);
}
inline void
CreateResourceListener(
    const ResourceId& id
    , std::function<void(const Resources::ResourceId)> success
    , std::function<void(const Resources::ResourceId)> failed = nullptr
)
{
    return ResourceServer::Instance()->CreateResourceListener(id, success, failed);
}
inline void
SetMinLod(const ResourceId& id, float lod, bool immediate)
{
    return ResourceServer::Instance()->SetMinLod(id, lod, immediate);
}
inline void
DiscardResource(const Resources::ResourceId id)
{
    ResourceServer::Instance()->DiscardResource(id);
}
inline void
ReloadResource(const ResourceName& res)
{
    return ResourceServer::Instance()->ReloadResource(res);
}
inline void
WaitForLoaderThread()
{
    ResourceServer::Instance()->WaitForLoaderThread();
}
template <class POOL_TYPE>
inline POOL_TYPE*
GetStreamLoader()
{
    static_assert(std::is_base_of<ResourceLoader, POOL_TYPE>::value, "Аргумент шаблона не является типом ResourceCache!");
    return ResourceServer::Instance()->GetStreamLoader<POOL_TYPE>();
}
}