#pragma once
#include "core/refcounted.h"
#include "resourceid.h"
namespace Resources
{
class ResourceLoader;
class ResourceMemoryCache;
class ResourceServer;
class ResourceContainer;
class Resource
{
public:
    enum State
    {
        Pending,
        Loaded,
        Failed,
        Unloaded
    };
    Resource();
    virtual ~Resource();
protected:
    friend class ResourceLoader;
    friend class ResourceMemoryCache;
    friend class ResourceServer;
    friend class ResourceContainer;
};
}