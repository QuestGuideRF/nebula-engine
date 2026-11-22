#pragma once
#include "util/stringatom.h"
#include "ids/id.h"
#include "core/debug.h"
#include "ids/idgenerationpool.h"
namespace Resources
{
typedef Util::StringAtom ResourceName;
ID_24_8_24_8_NAMED_TYPE(ResourceId, loaderInstanceId, loaderIndex, resourceId, generation, loader, resource);
ID_24_8_NAMED_TYPE(ResourceUnknownId, resourceId, generation, id);
}
#define RESOURCE_ID_TYPE(type) struct type : public Resources::ResourceUnknownId { \
        constexpr type() {};\
        constexpr type(const Resources::ResourceUnknownId& res) : Resources::ResourceUnknownId(res) {};\
        constexpr type(const Resources::ResourceId& res) : Resources::ResourceUnknownId(res.resourceId, res.generation) {};\
        constexpr type(const Ids::Id24 id, const Ids::Id8 generation) : Resources::ResourceUnknownId(id, generation) {};\
        constexpr type(const Ids::Id32 id) : Resources::ResourceUnknownId(Ids::Index(id), Ids::Generation(id)) {};\
    }; \
    static constexpr type Invalid##type = Resources::InvalidResourceUnknownId;