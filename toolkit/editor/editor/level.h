#pragma once
#include "core/refcounted.h"
#include "memdb/database.h"
#include "game/entity.h"
namespace Editor
{
class Level;
struct Sublevel
{
    Util::String alias;
    Math::vec3 position;
    Math::quat orientation;
    Math::vec3 scale;
    Ptr<Level> level;
};
class Level : public Core::RefCounted
{
    __DeclareClass(Level);
public:
    Level();
    virtual ~Level();
    void Clear();
    bool LoadLevel(const Util::String& name);
    bool SaveLevelAs(const Util::String& name);
    static bool Export(const Util::String& name);
    const Util::String& GetName() const;
    void RemoveReference(const Util::String& level);
private:
    Util::String name;
    Util::Array<Game::Entity> entities;
};
}