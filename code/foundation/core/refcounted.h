#pragma once
#include "core/types.h"
#include "core/rtti.h"
#include "core/factory.h"
#include "threading/interlocked.h"
#include "core/refcountedlist.h"
#if NEBULA_DEBUG
#include "threading/criticalsection.h"
#include "util/dictionary.h"
#endif
namespace Core
{
class RefCounted
{
    __DeclareClass(RefCounted);
public:
    RefCounted();
    int GetRefCount() const;
    void AddRef();
    void Release();
    bool IsInstanceOf(const Rtti& rtti) const;
    bool IsInstanceOf(const Util::String& className) const;
    bool IsInstanceOf(const Util::FourCC& classFourCC) const;
    bool IsA(const Rtti& rtti) const;
    bool IsA(const Util::String& rttiName) const;
    bool IsA(const Util::FourCC& rttiFourCC) const;
    const Util::String& GetClassName() const;
    Util::FourCC GetClassFourCC() const;
    static void DumpRefCountingLeaks();
    #if NEBULA_DEBUG
    struct Stats
    {
        Util::String className;
        Util::FourCC classFourCC;
        SizeT numObjects;
        SizeT overallRefCount;
        SizeT instanceSize;
    };
    static Util::Dictionary<Util::String,Stats> GetOverallStats();
    #endif
protected:
    virtual ~RefCounted();
private:
    volatile int refCount;
    #if NEBULA_DEBUG
protected:
        static ThreadLocal bool isInCreate;
        static Threading::CriticalSection criticalSection;
private:
        bool destroyed;
        static RefCountedList list;
        RefCountedList::Iterator listIterator;
public:
        void SetDebugName(const Util::String& name);
    #endif
};
inline
RefCounted::RefCounted() :
    refCount(0)
{
    #if NEBULA_DEBUG
    n_assert2(this->isInCreate, "RefCounted objects must be created with Create()!");
    this->listIterator = list.AddBack(this);
    this->destroyed = false;
    #endif
}
inline void
RefCounted::AddRef()
{
    Threading::Interlocked::Increment(&this->refCount);
}
inline void
RefCounted::Release()
{
    if (0 == Threading::Interlocked::Decrement(&this->refCount))
    {
        delete this;
    }
}
inline int
RefCounted::GetRefCount() const
{
    return this->refCount;
}
inline bool
RefCounted::IsInstanceOf(const Rtti& other) const
{
    return this->GetRtti() == &other;
}
inline bool
RefCounted::IsInstanceOf(const Util::String& other) const
{
    return this->GetRtti()->GetName() == other;
}
inline bool
RefCounted::IsInstanceOf(const Util::FourCC& other) const
{
    return this->GetRtti()->GetFourCC() == other;
}
inline bool
RefCounted::IsA(const Rtti& other) const
{
    return this->GetRtti()->IsDerivedFrom(other);
}
inline bool
RefCounted::IsA(const Util::String& other) const
{
    return this->GetRtti()->IsDerivedFrom(other);
}
inline bool
RefCounted::IsA(const Util::FourCC& other) const
{
    return this->GetRtti()->IsDerivedFrom(other);
}
inline const Util::String&
RefCounted::GetClassName() const
{
    return this->GetRtti()->GetName();
}
inline Util::FourCC
RefCounted::GetClassFourCC() const
{
    return this->GetRtti()->GetFourCC();
}
#if NEBULA_DEBUG
inline void
RefCounted::SetDebugName(const Util::String& name)
{
    list.SetDebugName(this, name);
}
#endif
}