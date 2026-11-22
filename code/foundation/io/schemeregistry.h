#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "threading/criticalsection.h"
namespace IO
{
class SchemeRegistry : public Core::RefCounted
{
    __DeclareClass(SchemeRegistry);
    __DeclareInterfaceSingleton(SchemeRegistry);
public:
    SchemeRegistry();
    virtual ~SchemeRegistry();
    void Setup();
    void Discard();
    bool IsValid() const;
    void RegisterUriScheme(const Util::String& uriScheme, const Core::Rtti& classRtti);
    void UnregisterUriScheme(const Util::String& uriScheme);
    bool IsUriSchemeRegistered(const Util::String& uriScheme) const;
    const Core::Rtti& GetStreamClassByUriScheme(const Util::String& uriScheme) const;
    Util::Array<Util::String> GetAllRegisteredUriSchemes() const;
private:
    void SetupStandardSchemes();
    Threading::CriticalSection critSect;
    Util::Dictionary<Util::String, const Core::Rtti*> schemeRegistry;
    bool isValid;
};
}