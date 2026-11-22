#pragma once
#include "core/refcounted.h"
#include "core/ptr.h"
#include "core/singleton.h"
#include "game/featureunit.h"
#include "debug/debugtimer.h"
#include "ids/idgenerationpool.h"
#include "api.h"
#include "entitypool.h"
#include "memdb/database.h"
#include "world.h"
#include "processor.h"
namespace Game
{
class GameServer : public Core::RefCounted
{
    __DeclareClass(GameServer)
    __DeclareSingleton(GameServer)
public:
    GameServer();
    virtual ~GameServer();
    virtual bool Open();
    virtual bool Start();
    virtual void Stop();
    virtual void Close();
    bool HasStarted() const;
    virtual void OnBeginFrame();
    virtual void OnFrame();
    virtual void OnEndFrame();
    virtual void NotifyBeforeLoad();
    virtual void NotifyBeforeCleanup();
    virtual void NotifyGameLoad();
    virtual void NotifyGameSave();
    void RenderDebug();
    void AttachGameFeature(const Ptr<FeatureUnit>& feature);
    void RemoveGameFeature(const Ptr<FeatureUnit>& feature);
    bool IsFeatureAttached(const Util::String& stringName) const;
    Util::Array<Ptr<FeatureUnit>> const& GetGameFeatures() const;
    void SetCmdLineArgs(const Util::CommandLineArgs& a);
    const Util::CommandLineArgs& GetCmdLineArgs() const;
    virtual void SetupEmptyWorld(World*);
    virtual void CleanupWorld(World*);
    World* CreateWorld(WorldHash hash);
    World* GetWorld(WorldHash worldHash);
    World* GetWorld(WorldId id);
    void DestroyWorld(WorldHash worldHash);
    struct State
    {
        World* worlds[32];
        uint numWorlds = 0;
        Util::HashTable<WorldHash, WorldId, 32, 1> worldTable;
        Ptr<MemDb::Database> templateDatabase;
    } state;
protected:
    bool isOpen;
    bool isStarted;
    Util::CommandLineArgs args;
    Util::Array<Ptr<FeatureUnit> > gameFeatures;
#if NEBULA_ENABLE_PROFILING
    _declare_timer(GameServerOnBeginFrame);
    _declare_timer(GameServerOnFrame);
    _declare_timer(GameServerOnEndFrame);
    _declare_timer(GameServerManageEntities);
    Util::Array<Ptr<Debug::DebugTimer>> onBeginFrameTimers;
    Util::Array<Ptr<Debug::DebugTimer>> onFrameTimers;
    Util::Array<Ptr<Debug::DebugTimer>> onEndFrameTimers;
#endif
};
inline void
GameServer::SetCmdLineArgs(const Util::CommandLineArgs& a)
{
    this->args = a;
}
inline const Util::CommandLineArgs&
GameServer::GetCmdLineArgs() const
{
    return this->args;
}
};