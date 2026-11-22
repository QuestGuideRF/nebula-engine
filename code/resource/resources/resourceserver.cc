//------------------------------------------------------------------------------
// resourceserver.cc
// (C) 2017-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "resourceserver.h"
#include "profiling/profiling.h"

#if NEBULA_DEBUG
#include "core/sysfunc.h"
#endif
namespace Resources
{

__ImplementClass(Resources::ResourceServer, 'RMGR', Core::RefCounted);
__ImplementInterfaceSingleton(Resources::ResourceServer);

int32_t ResourceServer::UniquePoolCounter = 0;
//------------------------------------------------------------------------------
/**
*/
ResourceServer::ResourceServer()
{
    __ConstructSingleton;
    this->open = false;
}

//------------------------------------------------------------------------------
/**
*/
ResourceServer::~ResourceServer()
{
    __DestructSingleton;
    n_assert(!this->open); // обязательно позвоните close, прежде чем уничтожать объект
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceServer::Open()
{
    n_assert(!this->open);
    this->loaders.Reserve(256); // младшие 8 бит идентификатора ресурса могут достигать только 256
    this->open = true;
    UniquePoolCounter = 0;
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceServer::Close()
{
    n_assert(this->open);

#if NEBULA_DEBUG
    // сообщать о любых ресурсах, которые не были выгружены
    bool hasLeaks = false;
    Core::SysFunc::DebugOut("\n\n******** NEBULA RESOURCE MANAGER ********\n Начало отчета об утечке ресурсов:");
    for (IndexT i = 0; i < this->loaders.Size(); i++)
    {
        Ptr<ResourceLoader> pool = this->loaders[i];
        if (!pool.isvalid())
        {
            continue;
        }
        for (auto & kvp : pool->ids)
        {
            const auto resource = pool->resources[kvp.Value()];
            if (pool->GetUsage(kvp.Value()) != 0)
            {
                const Resources::ResourceName& name = kvp.Key();
                Util::String msg = Util::String::Sprintf("Resource <%s> (id %d) from pool %d is not unloaded, usage is '%d'\n", name.Value(), resource.resourceId, pool->uniqueId, pool->GetUsage(kvp.Value()));
                Core::SysFunc::DebugOut(msg.AsCharPtr());
                hasLeaks = true;
            }
        }       
    }

    if (!hasLeaks)
    {
        Util::String msg = Util::String::Sprintf("\n\n******** NEBULA RESOURCE MANAGER ********\n Все ресурсы были правильно освобождены!\n\n");
        Core::SysFunc::DebugOut(msg.AsCharPtr());
    }

#endif
    this->loaders.Clear();
    this->extensionMap.Clear();
    this->open = false;
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceServer::RegisterStreamLoader(const Util::StringAtom& ext, const Core::Rtti& loaderClass)
{
    n_assert(this->open);
    n_assert(loaderClass.IsDerivedFrom(ResourceLoader::RTTI));
    void* obj = loaderClass.Create();
    Ptr<ResourceLoader> loader((ResourceLoader*)obj);
    loader->uniqueId = UniquePoolCounter++;
    loader->Setup();
    this->loaders.Append(loader);
    this->extensionMap.Add(ext, this->loaders.Size() - 1);
    this->typeMap.Add(&loaderClass, this->loaders.Size() - 1);
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceServer::DeregisterStreamLoader(const Util::StringAtom& ext, const Core::Rtti& loaderClass)
{
    n_assert(this->open);
    n_assert(loaderClass.IsDerivedFrom(ResourceLoader::RTTI));
    n_assert(this->extensionMap.Contains(ext));
    n_assert(this->typeMap.Contains(&loaderClass));

    IndexT loaderIdx = this->extensionMap[ext];
    n_assert(this->typeMap[&loaderClass] == loaderIdx);
    Ptr<ResourceLoader> loader = this->loaders[loaderIdx];
    this->loaders[loaderIdx] = nullptr;
    this->extensionMap.Erase(ext);
    this->typeMap.Erase(&loaderClass);
    
    loader->ClearPendingUnloads();
    for (auto& kvp : loader->ids)
    {
        const auto resource = loader->resources[kvp.Value()];
        if (loader->GetUsage(kvp.Value()) != 0)
        {
            const Resources::ResourceName& name = kvp.Key();
            Util::String msg = Util::String::Sprintf("Resource <%s> (id %d) from pool %d is not unloaded, usage is '%d'\n", name.Value(), resource.resourceId, loader->uniqueId, loader->GetUsage(kvp.Value()));
            Core::SysFunc::DebugOut(msg.AsCharPtr());
        }
    }
    loader = nullptr;
}

//------------------------------------------------------------------------------
/**
*/
void 
ResourceServer::LoadDefaultResources()
{
    n_assert(this->open);
    IndexT i;
    for (i = 0; i < this->loaders.Size(); i++)
    {
        this->loaders[i]->LoadFallbackResources();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceServer::Update(IndexT frameIndex)
{
    N_SCOPE(Update, Resources);
    IndexT i;
    for (i = 0; i < this->loaders.Size(); i++)
    {
        const Ptr<ResourceLoader>& loader = this->loaders[i];
        loader->Update(frameIndex);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceServer::DiscardResources(const Util::StringAtom& tag)
{
    IndexT i;
    for (i = 0; i < this->loaders.Size(); i++)
    {
        const Ptr<ResourceLoader>& loader = this->loaders[i];
        loader->DiscardByTag(tag);
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
ResourceServer::HasPendingResources()
{
    IndexT i;
    for (i = 0; i < this->loaders.Size(); i++)
    {
        const Ptr<ResourceLoader>& loader = this->loaders[i];
        if (loader->IsA(ResourceLoader::RTTI))
        {
            const Ptr<ResourceLoader>& pool = loader.cast<ResourceLoader>();
            if (!pool->pendingLoads.IsEmpty())
                return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
Core::Rtti*
ResourceServer::GetType(const Resources::ResourceId id)
{
    // получить идентификатор загрузчика
    const Ids::Id8 loaderid = id.loaderIndex;

    // получить загрузчик ресурсов по расширению
    n_assert(this->loaders.Size() > loaderid);
    const Ptr<ResourceLoader>& loader = this->loaders[loaderid];
    return loader->GetRtti();
}

//------------------------------------------------------------------------------
/**
*/
void 
ResourceServer::WaitForLoaderThread()
{
    for (const Ptr<ResourceLoader>& loader : this->loaders)
    {
        loader->streamerThread->Wait();
    }
}

} // Ресурсы пространства имен
