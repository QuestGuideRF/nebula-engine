#pragma once
#include "core/refcounted.h"
#include "util/stringatom.h"
#include "util/ringbuffer.h"
#include "threading/criticalsection.h"
#if NEBULA_ENABLE_PROFILING
#define _declare_counter(counter) Ptr<Debug::DebugCounter> counter;
#define _setup_counter(counter) counter = Debug::DebugCounter::Create(); counter->Setup(Util::StringAtom(#counter));
#define _setup_grouped_counter(counter, group) counter = Debug::DebugCounter::Create(); counter->Setup(Util::StringAtom(#counter), group);
#define _discard_counter(counter) counter->Discard(); counter = nullptr;
#define _begin_counter(counter) counter->Begin();
#define _begin_counter_noreset(counter) counter->Begin(false);
#define _reset_counter(counter) counter->Reset();
#define _incr_counter(counter, val) counter->Incr(val);
#define _decr_counter(counter, val) counter->Decr(val);
#define _set_counter(counter, val) counter->Set(val);
#define _end_counter(counter) counter->End();
#else
#define _declare_counter(counter)
#define _setup_counter(counter)
#define _setup_grouped_counter(counter, group)
#define _discard_counter(counter)
#define _begin_counter(counter)
#define _begin_counter_noreset()
#define _reset_counter(counter)
#define _incr_counter(counter, val)
#define _decr_counter(counter, val)
#define _set_counter(counter, val)
#define _end_counter(counter)
#endif
namespace Debug
{
class DebugCounter : public Core::RefCounted
{
    __DeclareClass(DebugCounter);
public:
    DebugCounter();
    virtual ~DebugCounter();
    void Setup(const Util::StringAtom& name, const Util::StringAtom& group = Util::StringAtom("Ungrouped"));
    void Discard();
    bool IsValid() const;
    void Begin(bool reset=true);
    void Reset();
    void Incr(int amount);
    void Decr(int amount);
    void Set(int val);
    void End();
    const Util::StringAtom& GetName() const;
    const Util::StringAtom& GetGroup() const;
    int GetSample() const;
    Util::Array<int> GetHistory() const;
private:
    Threading::CriticalSection critSect;
    Util::StringAtom name;
    Util::StringAtom group;
    int value;
    Util::RingBuffer<int> history;
};
inline bool
DebugCounter::IsValid() const
{
    return this->name.IsValid();
}
inline void
DebugCounter::Begin(bool reset)
{
    if (reset)
    {
        this->value = 0;
    }
}
inline void
DebugCounter::Reset()
{
    this->value = 0;
}
inline void
DebugCounter::Incr(int amount)
{
    this->value += amount;
}
inline void
DebugCounter::Decr(int amount)
{
    this->value -= amount;
}
inline void
DebugCounter::Set(int val)
{
    this->value = val;
}
inline void
DebugCounter::End()
{
    this->critSect.Enter();
    this->history.Add(this->value);
    this->critSect.Leave();
}
inline const Util::StringAtom&
DebugCounter::GetName() const
{
    return this->name;
}
inline const Util::StringAtom&
DebugCounter::GetGroup() const
{
    return this->group;
}
}