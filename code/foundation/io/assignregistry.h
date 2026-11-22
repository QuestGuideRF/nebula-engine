#pragma once
#include "core/refcounted.h"
#include "io/assign.h"
#include "io/uri.h"
#include "util/hashtable.h"
#include "threading/criticalsection.h"
#include "core/singleton.h"
namespace IO
{
class AssignRegistry : public Core::RefCounted
{
    __DeclareClass(AssignRegistry);
    __DeclareInterfaceSingleton(AssignRegistry);
public:
    AssignRegistry();
    virtual ~AssignRegistry();
    void Setup();
    void Discard();
    bool IsValid() const;
    void SetAssign(const Assign& assign);
    bool HasAssign(const Util::String& assignName) const;
    Util::String GetAssign(const Util::String& assignName) const;
    void ClearAssign(const Util::String& assignName);
    Util::Array<Assign> GetAllAssigns() const;
    URI ResolveAssigns(const URI& uri) const;
    Util::String ResolveAssignsInString(const Util::String& uriString) const;
    void PrintAll() const;
private:
    void SetupSystemAssigns();
    void SetupProjectAssigns();
    bool isValid;
    Threading::CriticalSection critSect;
    Util::HashTable<Util::String, Util::String> assignTable;
};
}