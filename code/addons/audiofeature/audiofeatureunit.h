#pragma once
#include "game/featureunit.h"
#include "audio/audioserver.h"
#define TIMESOURCE_AUDIO 'AUTS'
namespace AudioFeature
{
class AudioFeatureUnit : public Game::FeatureUnit
{
    __DeclareClass(AudioFeatureUnit)
    __DeclareSingleton(AudioFeatureUnit)
public:
    AudioFeatureUnit();
    ~AudioFeatureUnit();
    void OnAttach();
    void OnActivate();
    void OnDeactivate();
    virtual void OnBeginFrame();
    virtual void OnRenderDebug();
private:
    Ptr<Audio::AudioServer> audioServer;
};
}