#pragma once
#include "util/array.h"
#include "util/string.h"
#include "util/fourcc.h"
#include "util/dictionary.h"
#include "util/hashtable.h"
#include "core/ptr.h"
namespace Core
{
class RefCounted;
class Rtti;
class Factory
{
public:
    static Factory* Instance();
    static void Destroy();
    void Register(const Rtti* rtti, const Util::String& className, const Util::FourCC& classFourCC);
    void Register(const Rtti* rtti, const Util::String& className);
    bool ClassExists(const Util::String& className) const;
    bool ClassExists(const Util::FourCC classFourCC) const;
    const Rtti* GetClassRtti(const Util::String& className) const;
    const Rtti* GetClassRtti(const Util::FourCC& classFourCC) const;
    void* Create(const Util::String& className) const;
    void* Create(const Util::FourCC classFourCC) const;
    void* CreateArray(const Util::String& className, SizeT num) const;
    void* CreateArray(const Util::FourCC classFourCC, SizeT num) const;
private:
    Factory();
    ~Factory();
    static Factory* Singleton;
    Util::HashTable<Util::String, const Rtti*, 1024> nameTable;
    Util::Dictionary<Util::FourCC, const Rtti*> fourccTable;
};
}