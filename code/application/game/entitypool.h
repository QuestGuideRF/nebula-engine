#pragma once
#include "api.h"
#include "category.h"
#include "util/queue.h"
namespace Game
{
class EntityPool
{
public:
    EntityPool();
    bool Allocate(Entity& e);
    void Deallocate(Entity e);
    bool IsValid(Entity e) const;
    Util::Array<uint16_t> generations;
    Util::Queue<uint32_t> freeIds;
};
}