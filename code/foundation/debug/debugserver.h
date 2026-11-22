#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "threading/criticalsection.h"
#include "util/stringatom.h"
namespace Debug
{
class DebugCounter;
class DebugTimer;
class DebugServer : public Core::RefCounted
{
    __DeclareClass(DebugServer);
    __DeclareInterfaceSingleton(DebugServer);
public:
    DebugServer();
    virtual ~DebugServer();
    bool IsOpen() const;
    void RegisterDebugTimer(const Ptr<DebugTimer>& timer);
    void UnregisterDebugTimer(const Ptr<DebugTimer>& timer);
    void RegisterDebugCounter(const Ptr<DebugCounter>& counter);
    void UnregisterDebugCounter(const Ptr<DebugCounter>& counter);
    Util::Array<Ptr<DebugTimer>> GetDebugTimers() const;
    Util::Array<Ptr<DebugCounter>> GetDebugCounters() const;
    Ptr<DebugTimer> GetDebugTimerByName(const Util::StringAtom& name) const;
    Ptr<DebugCounter> GetDebugCounterByName(const Util::StringAtom& name) const;
private:
    friend class DebugHandler;
    friend class DebugPageHandler;
    void Open();
    void Close();
    bool isOpen;
    Threading::CriticalSection critSect;
    Util::Dictionary<Util::StringAtom, Ptr<DebugTimer>> debugTimers;
    Util::Dictionary<Util::StringAtom, Ptr<DebugCounter>> debugCounters;
};
inline bool
DebugServer::IsOpen() const
{
    bool retval;
    this->critSect.Enter();
    retval = this->isOpen;
    this->critSect.Leave();
    return retval;
}
}