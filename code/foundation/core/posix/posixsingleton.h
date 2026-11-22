#pragma once
#ifndef CORE_POSIXSINGLETON_H
#define CORE_POSIXSINGLETON_H
#include "core/types.h"
#define __DeclareSingleton(type) \
public: \
    static ThreadLocal type * Singleton; \
    static type * Instance() { n_assert(0 != Singleton); return Singleton; }; \
    static bool HasInstance() { return 0 != Singleton; }; \
private:
#define __DeclareInterfaceSingleton(type) \
public: \
    static type * Singleton; \
    static type * Instance() { n_assert(0 != Singleton); return Singleton; }; \
    static bool HasInstance() { return 0 != Singleton; }; \
private:
#define __ImplementSingleton(type) \
    ThreadLocal type * type::Singleton = 0;
#define __ImplementInterfaceSingleton(type) \
    type * type::Singleton = 0;
#define __ConstructSingleton \
    n_assert(0 == Singleton); Singleton = this;
#define __ConstructInterfaceSingleton \
    n_assert(0 == Singleton); Singleton = this;
#define __DestructSingleton \
    n_assert(Singleton); Singleton = 0;
#define __DestructInterfaceSingleton \
    n_assert(Singleton); Singleton = 0;
#endif