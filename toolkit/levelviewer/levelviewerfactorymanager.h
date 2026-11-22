#pragma once
#include "managers/factorymanager.h"
namespace LevelViewer
{
class LevelViewerFactoryManager : public BaseGameFeature::FactoryManager
{
    __DeclareClass(LevelViewerFactoryManager);
    __DeclareSingleton(LevelViewerFactoryManager);
public:
    LevelViewerFactoryManager();
    virtual ~LevelViewerFactoryManager();
    virtual void AddProperties(const Ptr<Game::Entity>& entity, const Util::String& categoryName, bool isMaster) const;
};
}