#pragma once
#include "core/refcounted.h"
#include "io/jsonreader.h"
#include "game/entity.h"
#include "util/guid.h"
#include "util/hashtable.h"
namespace Game
{
class World;
}
namespace BaseGameFeature
{
class LevelParser : public Core::RefCounted
{
    __DeclareClass(LevelParser);
public:
    LevelParser();
    virtual ~LevelParser();
    void SetWorld(Game::World* world);
    Util::Array<Game::Entity> LoadJsonLevel(const Ptr<IO::JsonReader>& reader);
protected:
    Game::Entity LoadEntity(const Ptr<IO::JsonReader> & reader);
    void LoadComponents(const Ptr<IO::JsonReader>& reader, Game::Entity entity);
    virtual void BeginLoad(){}
    virtual void AddEntity(Game::Entity entity, Util::Guid const& guid){}
    virtual void SetName(Game::Entity entity, const Util::String & name){}
    virtual void CommitEntity(Game::Entity entity) {}
    virtual void CommitLevel(){}
private:
    Util::Array<Util::String> invalidAttrs;
    Util::HashTable<Util::Guid, Game::Entity> guidToEntity;
    Game::World* world;
};
}