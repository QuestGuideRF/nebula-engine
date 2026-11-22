#pragma once
#include "core/ptr.h"
#include "core/refcounted.h"
#include "core/singleton.h"
#include "io/console.h"
#include "util/stringatom.h"
namespace Core
{
class CoreServer : public RefCounted
{
    __DeclareClass(CoreServer);
    __DeclareSingleton(CoreServer);
public:
    CoreServer();
    virtual ~CoreServer();
    void SetCompanyName(const Util::StringAtom& s);
    const Util::StringAtom& GetCompanyName() const;
    void SetAppName(const Util::StringAtom& s);
    const Util::StringAtom& GetAppName() const;
    void SetRootDirectory(const Util::StringAtom& s);
    const Util::StringAtom& GetRootDirectory() const;
    void SetToolDirectory(const Util::StringAtom& s);
    const Util::StringAtom& GetToolDirectory() const;
    void Open();
    void Close();
    bool IsOpen() const;
    void Trigger();
private:
    Ptr<IO::Console> con;
    Util::StringAtom companyName;
    Util::StringAtom appName;
    Util::StringAtom rootDirectory;
    Util::StringAtom toolDirectory;
    bool isOpen;
};
inline bool
CoreServer::IsOpen() const
{
    return this->isOpen;
}
inline void
CoreServer::SetCompanyName(const Util::StringAtom& s)
{
    this->companyName = s;
}
inline const Util::StringAtom&
CoreServer::GetCompanyName() const
{
    return this->companyName;
}
inline void
CoreServer::SetAppName(const Util::StringAtom& s)
{
    this->appName = s;
}
inline const Util::StringAtom&
CoreServer::GetAppName() const
{
    return this->appName;
}
inline void
CoreServer::SetRootDirectory(const Util::StringAtom& s)
{
    this->rootDirectory = s;
}
inline const Util::StringAtom&
CoreServer::GetRootDirectory() const
{
    return this->rootDirectory;
}
inline void
CoreServer::SetToolDirectory(const Util::StringAtom& s)
{
    this->toolDirectory = s;
}
inline const Util::StringAtom&
CoreServer::GetToolDirectory() const
{
    return this->toolDirectory;
}
}