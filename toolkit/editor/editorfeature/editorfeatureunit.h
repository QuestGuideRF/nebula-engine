#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "game/featureunit.h"
namespace EditorFeature
{
class EditorFeatureUnit : public Game::FeatureUnit
{
    __DeclareClass(EditorFeatureUnit);
    __DeclareSingleton(EditorFeatureUnit);
public:
    EditorFeatureUnit();
    ~EditorFeatureUnit();
    virtual void OnAttach();
    virtual void OnActivate();
    virtual void OnDeactivate();
    virtual void OnEndFrame();
    virtual void OnRenderDebug();
    virtual void OnFrame();
private:
};
}