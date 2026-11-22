#pragma once
#include "statehandlers/gamestatehandler.h"
#include "game/entity.h"
namespace Tools
{
class ReloadState : public BaseGameFeature::GameStateHandler
{
    __DeclareClass(ReloadState);
public:
    ReloadState();
    virtual ~ReloadState();
    virtual void OnStateEnter(const Util::String& prevState);
    virtual Util::String OnFrame();
private:
    Util::String nextLevelName;
};
}