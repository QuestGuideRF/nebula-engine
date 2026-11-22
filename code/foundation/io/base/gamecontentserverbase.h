#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
namespace Base
{
class GameContentServerBase : public Core::RefCounted
{
    __DeclareClass(GameContentServerBase);
    __DeclareInterfaceSingleton(GameContentServerBase);
public:
    GameContentServerBase();
    virtual ~GameContentServerBase();
    void SetTitle(const Util::String& title);
    const Util::String& GetTitle() const;
    void SetTitleId(const Util::String& titleId);
    const Util::String& GetTitleId() const;
    void SetVersion(const Util::String& version);
    const Util::String& GetVersion() const;
    void Setup();
    void Discard();
    bool IsValid() const;
protected:
    Util::String title;
    Util::String titleId;
    Util::String version;
    bool isValid;
};
inline bool
GameContentServerBase::IsValid() const
{
    return this->isValid;
}
inline void
GameContentServerBase::SetTitle(const Util::String& t)
{
    this->title = t;
}
inline const Util::String&
GameContentServerBase::GetTitle() const
{
    return this->title;
}
inline void
GameContentServerBase::SetTitleId(const Util::String& tid)
{
    this->titleId = tid;
}
inline const Util::String&
GameContentServerBase::GetTitleId() const
{
    return this->titleId;
}
inline void
GameContentServerBase::SetVersion(const Util::String& v)
{
    this->version = v;
}
inline const Util::String&
GameContentServerBase::GetVersion() const
{
    return this->version;
}
}