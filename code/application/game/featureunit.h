#pragma once
#include "core/refcounted.h"
#include "game/manager.h"
#include "core/singleton.h"
#include "util/commandlineargs.h"
#include "io/jsonwriter.h"
#include "ids/idgenerationpool.h"
#include "util/arrayallocator.h"
#include "game/component.h"
namespace Game
{
class FeatureUnit : public Core::RefCounted
{
    __DeclareClass(FeatureUnit)
public:
    FeatureUnit();
    virtual ~FeatureUnit();
    template <typename COMPONENT_TYPE>
    ComponentId RegisterComponentType(ComponentRegisterInfo<COMPONENT_TYPE> info = {});
    virtual void OnAttach();
    virtual void OnRemove();
    virtual void OnActivate();
    virtual void OnDeactivate();
    bool IsActive() const;
    virtual void OnLoad(World* world);
    virtual void OnStart(World* world);
    virtual void OnSave(World* world);
    virtual void OnBeforeLoad(World* world);
    virtual void OnBeforeCleanup(World* world);
    virtual void OnStop(World* world);
    virtual void OnBeginFrame();
    virtual void OnBeforeViews();
    virtual void OnFrame();
    virtual void OnEndFrame();
    virtual void OnDecay();
    virtual void OnRenderDebug();
    virtual void AttachManager(Ptr<Manager> manager);
    virtual void RemoveManager(Ptr<Manager> manager);
    void SetCmdLineArgs(const Util::CommandLineArgs& a);
    const Util::CommandLineArgs& GetCmdLineArgs() const;
protected:
    Util::Array<Ptr<Manager>> managers;
    bool active;
    Util::CommandLineArgs args;
};
template <typename COMPONENT_TYPE>
ComponentId
FeatureUnit::RegisterComponentType(ComponentRegisterInfo<COMPONENT_TYPE> info)
{
    uint32_t componentFlags = 0;
    componentFlags |= (uint32_t)COMPONENTFLAG_DECAY * (uint32_t)info.decay;
    ComponentInterface* cInterface = new ComponentInterface(COMPONENT_TYPE::Traits::name, COMPONENT_TYPE(), componentFlags);
    cInterface->Init = reinterpret_cast<ComponentInterface::ComponentInitFunc>(info.OnInit);
    Game::ComponentId const cid = MemDb::AttributeRegistry::Register<COMPONENT_TYPE>(cInterface);
    Game::ComponentSerialization::Register<COMPONENT_TYPE>(cid);
    Game::ComponentInspection::Register(cid, &Game::ComponentDrawFuncT<COMPONENT_TYPE>);
    return cid;
}
inline bool
FeatureUnit::IsActive() const
{
    return this->active;
}
inline void
FeatureUnit::SetCmdLineArgs(const Util::CommandLineArgs& a)
{
    this->args = a;
}
inline const Util::CommandLineArgs&
FeatureUnit::GetCmdLineArgs() const
{
    return this->args;
}
}