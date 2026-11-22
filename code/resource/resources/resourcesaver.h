#pragma once
#include "core/refcounted.h"
#include "io/uri.h"
#include "resourceid.h"
namespace Resources
{
class ResourceSaver : public Core::RefCounted
{
    __DeclareAbstractClass(ResourceSaver);
public:
    ResourceSaver();
    virtual ~ResourceSaver();
    enum SaveStatus
    {
        Success,
        Failed
    };
    virtual SaveStatus Save(const Resources::ResourceId id, IO::URI path) = 0;
};
}