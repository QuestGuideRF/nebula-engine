#pragma once
#include "messaging/handler.h"
#include "util/stringatom.h"
#include "animation/animeventinfo.h"
namespace Animation
{
class AnimEventHandlerBase : public Messaging::Handler
{
    __DeclareClass(AnimEventHandlerBase);
public:
    AnimEventHandlerBase();
    virtual ~AnimEventHandlerBase();
    virtual bool HandleEvent(const Animation::AnimEventInfo& event);
    virtual void OnFrame(Timing::Time time);
    void SetCategoryName(const Util::StringAtom& catName);
    const Util::StringAtom& GetCategoryName() const;
 private:
    bool isOpen;
    Util::StringAtom categoryName;
};
inline void
AnimEventHandlerBase::SetCategoryName(const Util::StringAtom& catName)
{
    this->categoryName = catName;
}
inline const Util::StringAtom&
AnimEventHandlerBase::GetCategoryName() const
{
    return this->categoryName;
}
}