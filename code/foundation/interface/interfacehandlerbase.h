#pragma once
#include "messaging/handler.h"
#include "util/stringatom.h"
namespace Interface
{
class InterfaceHandlerBase : public Messaging::Handler
{
    __DeclareClass(InterfaceHandlerBase);
public:
    InterfaceHandlerBase();
    void SetCompanyName(const Util::StringAtom& companyName);
    const Util::StringAtom& GetCompanyName() const;
    void SetAppName(const Util::StringAtom& appName);
    const Util::StringAtom& GetAppName() const;
    virtual void DoWork();
protected:
    Util::StringAtom companyName;
    Util::StringAtom appName;
};
inline void
InterfaceHandlerBase::SetCompanyName(const Util::StringAtom& n)
{
    this->companyName = n;
}
inline const Util::StringAtom&
InterfaceHandlerBase::GetCompanyName() const
{
    return this->companyName;
}
inline void
InterfaceHandlerBase::SetAppName(const Util::StringAtom& n)
{
    this->appName = n;
}
inline const Util::StringAtom&
InterfaceHandlerBase::GetAppName() const
{
    return this->appName;
}
}