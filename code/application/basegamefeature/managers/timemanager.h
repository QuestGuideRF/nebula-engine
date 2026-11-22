#pragma once
#include "core/singleton.h"
#include "game/manager.h"
#include "timing/time.h"
#define TIMESOURCE_GAMEPLAY uint32_t('GPTS')
#define TIMESOURCE_INPUT uint32_t('IPTS')
#define TIMESOURCE_SYSTEM uint32_t('SYTS')
namespace Game
{
struct TimeSourceCreateInfo
{
    uint32_t hash = 0;
};
struct TimeSource
{
    Timing::Time const frameTime;
    Timing::Time const time;
    Timing::Tick const ticks;
    int pauseCounter;
    float timeFactor;
};
namespace Time
{
    TimeSource* const CreateTimeSource(TimeSourceCreateInfo const& info);
    TimeSource* const GetTimeSource(uint32_t TIMESOURCE_HASH);
    void SetGlobalTimeFactor(float factor);
    float GetGlobalTimeFactor();
};
class TimeManager : public Game::Manager
{
    __DeclareClass(TimeManager)
    __DeclareSingleton(TimeManager)
public:
    TimeManager();
    virtual ~TimeManager();
    void OnActivate() override;
    void OnDeactivate() override;
    void OnBeginFrame() override;
};
}