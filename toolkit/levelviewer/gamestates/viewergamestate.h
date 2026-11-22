#pragma once
#include "statehandlers/gamestatehandler.h"
#include "graphics/spotlightentity.h"
#include "graphics/globallightentity.h"
#include "graphics/pointlightentity.h"
#include "graphics/modelentity.h"
#include "game/entity.h"
#include "posteffect/posteffectentity.h"
namespace Tools
{
class LevelViewerGameState : public BaseGameFeature::GameStateHandler
{
    __DeclareClass(LevelViewerGameState);
public:
    LevelViewerGameState();
    virtual ~LevelViewerGameState();
    virtual void OnStateEnter(const Util::String& prevState);
    virtual void OnStateLeave(const Util::String& nextState);
    virtual Util::String OnFrame();
    virtual void OnLoadBefore();
    virtual void OnLoadAfter();
    void HandleInput();
    void LoadLevel(const Util::String &level, bool applyTransform = false);
    void ReloadLevel(bool keepTransform);
private:
    Math::matrix44 focusTransform;
    Util::String lastLevel;
    bool applyTransform;
    bool entitiesLoaded;
};
}