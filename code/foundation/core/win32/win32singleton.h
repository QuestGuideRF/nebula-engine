#pragma once
#include "core/types.h"
#define __DeclareSingleton(type) \
public: \
    thread_local static type * Singleton; \
    static type * Instance() { n_assert(nullptr != Singleton); return Singleton; }; \
    static bool HasInstance() { return nullptr != Singleton; }; \
private:
#define __DeclareInterfaceSingleton(type) \
public: \
    static type * Singleton; \
    static type * Instance() { n_assert(nullptr != Singleton); return Singleton; }; \
    static bool HasInstance() { return nullptr != Singleton; }; \
private:
#define __ImplementSingleton(type) \
    thread_local type * type::Singleton = nullptr;
#define __ImplementInterfaceSingleton(type) \
    type * type::Singleton = nullptr;
#define __ConstructSingleton \
    n_assert(nullptr == Singleton); Singleton = this;
#define __ConstructInterfaceSingleton \
    n_assert(nullptr == Singleton); Singleton = this;
#define __DestructSingleton \
    n_assert(Singleton); Singleton = nullptr;
#define __DestructInterfaceSingleton \
    n_assert(Singleton); Singleton = nullptr;