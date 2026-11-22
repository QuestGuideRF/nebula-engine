#pragma once
#include "messaging/asyncport.h"
#include "util/stringatom.h"
namespace Interface
{
class InterfaceBase : public Messaging::AsyncPort
{
    __DeclareClass(InterfaceBase);
public:
    InterfaceBase();
    virtual ~InterfaceBase();
    virtual void AttachHandler(const Ptr<Messaging::Handler>& h);
    virtual void Open();
    virtual void Close();
    const Util::StringAtom& GetCompanyName() const;
    const Util::StringAtom& GetAppName() const;
    const Util::StringAtom& GetRootDirectory() const;
private:
    void SetCompanyName(const Util::StringAtom& companyName);
    void SetAppName(const Util::StringAtom& appName);
    void SetRootDirectory(const Util::StringAtom& dir);
    Util::StringAtom rootDirectory;
    Util::StringAtom companyName;
    Util::StringAtom appName;
};
inline void
InterfaceBase::SetCompanyName(const Util::StringAtom& n)
{
    this->companyName = n;
}
inline const Util::StringAtom&
InterfaceBase::GetCompanyName() const
{
    return this->companyName;
}
inline void
InterfaceBase::SetAppName(const Util::StringAtom& n)
{
    this->appName = n;
}
inline const Util::StringAtom&
InterfaceBase::GetAppName() const
{
    return this->appName;
}
inline void
InterfaceBase::SetRootDirectory(const Util::StringAtom& d)
{
    this->rootDirectory = d;
}
inline const Util::StringAtom&
InterfaceBase::GetRootDirectory() const
{
    return this->rootDirectory;
}
}