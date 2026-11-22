#pragma once
#include "core/types.h"
#include "threading/osx/osxthreadlocalptr.h"
#define __DeclareSingleton(type) \
public: \
static OSX::OSXThreadLocalPtr<type> Singleton; \
static type * Instance() { n_assert(!Singleton.isvalid()); return Singleton.get(); }; \
static bool HasInstance() { return Singleton.isvalid(); }; \
private:
#define __DeclareInterfaceSingleton(type) \
public: \
static type * Singleton; \
static type * Instance() { n_assert(0 != Singleton); return Singleton; }; \
static bool HasInstance() { return 0 != Singleton; }; \
private:
#define __ImplementSingleton(type) \
OSX::OSXThreadLocalPtr<type> type::Singleton;
#define __ImplementInterfaceSingleton(type) \
type * type::Singleton = 0;
#define __ConstructSingleton \
n_assert(!Singleton.isvalid()); Singleton.set(this);
#define __ConstructInterfaceSingleton \
n_assert(0 == Singleton); Singleton = this;
#define __DestructSingleton \
n_assert(Singleton.isvalid()); Singleton.set(0);
#define __DestructInterfaceSingleton \
n_assert(Singleton); Singleton = 0;