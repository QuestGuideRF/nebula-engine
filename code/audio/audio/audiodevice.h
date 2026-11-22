#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "resources/resourceid.h"
#include "ids/idallocator.h"
#include "audioclip.h"
#include "debug/debugtimer.h"
#include "debug/debugcounter.h"
#include "math/point.h"
#include "math/vector.h"
#include "math/mat4.h"
namespace SoLoud
{
class Wav;
}
namespace Audio
{
ID_32_TYPE(AudioEmitterId);
class AudioDevice : public Core::RefCounted
{
    __DeclareClass(AudioDevice);
    __DeclareSingleton(AudioDevice);
public:
    AudioDevice();
    ~AudioDevice();
    bool Open();
    bool Close();
    void OnFrame();
    ClipId LoadClip(Resources::ResourceName const& name);
    void UnloadClip(ClipId const id);
    ClipInstanceId Play(ClipId clip, float volume, float pan, bool loop = false, float clock = 0.0f);
    ClipInstanceId PlaySpatial(
        ClipId clip,
        float volume,
        Math::vec3 const& position,
        Math::vec3 const& velocity,
        float minDistance,
        float maxDistance,
        bool loop = false,
        float clock = 0.0f
    );
    void StopInstance(ClipInstanceId id);
    void Stop(ClipId id);
    bool IsValid(ClipInstanceId id);
    void UpdatePosition(ClipInstanceId id, Math::point const& pos);
    void UpdateVelocity(ClipInstanceId id, Math::vector const& vel);
    void SetListenerTransform(Math::mat4 const& transform);
    void SetListenerVelocity(Math::vector const& velocity);
    void ResetListener();
    void SetSpeedOfSound(float value = 343.0f);
    AudioEmitterId CreateAudioEmitter(Resources::ResourceName const& name);
    void DestroyAudioEmitter(AudioEmitterId const id);
    ClipInstanceId Play(AudioEmitterId id, bool loop = false);
    void SetSpatialize(AudioEmitterId id, bool value);
    void SetPosition(AudioEmitterId id, Math::point const& pos);
    void SetVelocity(AudioEmitterId id, Math::vector const& vel);
    void SetMaxDistance(AudioEmitterId id, float value);
    void SetMinDistance(AudioEmitterId id, float value);
    void SetClock(AudioEmitterId id, float value);
    void SetPan(AudioEmitterId id, float value);
private:
    enum EmitterSlot
    {
        CLIPID,
        POSITION,
        VELOCITY,
        VOLUME,
        MINDISTANCE,
        MAXDISTANCE,
        PAN,
        SPATIALIZE,
        CLOCK,
    };
    Ids::IdAllocator<ClipId, Math::point, Math::vector, float, float, float, float, bool, float> emitterAllocator;
    enum WavAllocator
    {
        WAV,
        REFCOUNT
    };
    Ids::IdAllocator<SoLoud::Wav, uint> wavs;
    Util::Dictionary<Resources::ResourceName, ClipId> clipMap;
    struct Listener
    {
        float forward[3] = {0, 0, 1};
        float up[3] = {0, 1, 0};
        float position[3] = {0, 0, 0};
        float velocity[3] = {0, 0, 0};
    } listener;
    _declare_timer(AudioOnFrameTime) _declare_counter(AudioNumberOfSoundsPlaying)
};
}