#pragma once
#include "core/refcounted.h"
#include "ids/id.h"
namespace Game
{
class World;
class Manager : public Core::RefCounted
{
    __DeclareClass(Manager)
public:
    Manager();
    virtual ~Manager();
    bool IsActive() const;
    virtual void OnActivate();
    virtual void OnDeactivate();
    virtual void OnBeginFrame() {}
    virtual void OnBeforeViews() {}
    virtual void OnFrame() {}
    virtual void OnEndFrame() {}
    virtual void OnDecay() {}
    virtual void OnLoad(World*) {}
    virtual void OnSave(World*) {}
    virtual void OnCleanup(World*) {}
    virtual void OnStart(World*) {}
    virtual void OnRenderDebug() {}
private:
    bool isActive = false;
};
inline bool
Manager::IsActive() const
{
    return this->isActive;
}
};