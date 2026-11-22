#pragma once
#include "core/refcounted.h"
#include "memdb/database.h"
#include "game/entity.h"
namespace Game
{
class World;
class PackedLevel
{
public:
    Util::Array<Game::Entity> Instantiate() const;
private:
    friend class World;
    PackedLevel() {};
    ~PackedLevel()
    {
        for (auto& t : tables)
        {
            if (t.columns != nullptr)
                delete[] t.columns;
        }
    };
    Game::World* world;
    struct EntityGroup
    {
        MemDb::TableId dstTable;
        SizeT numRows;
        ubyte* columns = nullptr;
    };
    Util::Array<EntityGroup> tables;
};
}