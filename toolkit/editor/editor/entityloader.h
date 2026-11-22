#pragma once
#include "basegamefeature/levelparser.h"
namespace Editor
{
bool SaveEntities(const char* uri);
class EntityLoader : public BaseGameFeature::LevelParser
{
    __DeclareClass(EntityLoader)
public:
    EntityLoader();
    ~EntityLoader();
private:
    virtual void BeginLoad() override;
    virtual void AddEntity(Game::Entity entity, Util::Guid const& guid) override;
    virtual void SetName(Game::Entity entity, const Util::String& name) override;
    virtual void CommitEntity(Game::Entity entity) override;
    virtual void CommitLevel() override;
};
}