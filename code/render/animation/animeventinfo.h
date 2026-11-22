#pragma once
#include "coreanimation/animevent.h"
#include "timing/time.h"
#include "util/stringatom.h"
#include "graphics/graphicsentity.h"
namespace Animation
{
class AnimEventInfo
{
public:
    AnimEventInfo();
    friend bool operator==(const AnimEventInfo& a, const AnimEventInfo& b);
    friend bool operator!=(const AnimEventInfo& a, const AnimEventInfo& b);
    friend bool operator<(const AnimEventInfo& a, const AnimEventInfo& b);
    friend bool operator>(const AnimEventInfo& a, const AnimEventInfo& b);
    friend bool operator<=(const AnimEventInfo& a, const AnimEventInfo& b);
    friend bool operator>=(const AnimEventInfo& a, const AnimEventInfo& b);
    const CoreAnimation::AnimEvent& GetAnimEvent() const;
    void SetAnimEvent(const CoreAnimation::AnimEvent& val);
    const Util::StringAtom& GetAnimJobName() const;
    void SetAnimJobName(const Util::StringAtom& val);
    float GetWeight() const;
    void SetWeight(float val);
    void SetEntityId(const Graphics::GraphicsEntity::Id& id);
    const Graphics::GraphicsEntity::Id& GetEntityId() const;
private:
    CoreAnimation::AnimEvent animEvent;
    Util::StringAtom animJobName;
    float weight;
    Graphics::GraphicsEntity::Id id;
};
inline
AnimEventInfo::AnimEventInfo(): weight(0.0f),
                                id(InvalidIndex)
{
}
inline bool
operator==(const AnimEventInfo& a, const AnimEventInfo& b)
{
    return (a.animEvent == b.animEvent);
}
inline bool
operator!=(const AnimEventInfo& a, const AnimEventInfo& b)
{
    return (a.animEvent != b.animEvent);
}
inline bool
operator<(const AnimEventInfo& a, const AnimEventInfo& b)
{
    return (a.animEvent < b.animEvent);
}
inline bool
operator>(const AnimEventInfo& a, const AnimEventInfo& b)
{
    return (a.animEvent > b.animEvent);
}
inline bool
operator<=(const AnimEventInfo& a, const AnimEventInfo& b)
{
    return (a.animEvent <= b.animEvent);
}
inline bool
operator>=(const AnimEventInfo& a, const AnimEventInfo& b)
{
    return (a.animEvent >= b.animEvent);
}
inline const CoreAnimation::AnimEvent&
AnimEventInfo::GetAnimEvent() const
{
    return this->animEvent;
}
inline void
AnimEventInfo::SetAnimEvent(const CoreAnimation::AnimEvent& val)
{
    this->animEvent = val;
}
inline const Util::StringAtom&
AnimEventInfo::GetAnimJobName() const
{
    return this->animJobName;
}
inline void
AnimEventInfo::SetAnimJobName(const Util::StringAtom& val)
{
    this->animJobName = val;
}
inline float
AnimEventInfo::GetWeight() const
{
    return this->weight;
}
inline void
AnimEventInfo::SetWeight(float val)
{
    this->weight = val;
}
inline
void
AnimEventInfo::SetEntityId(const Graphics::GraphicsEntity::Id& id)
{
    this->id = id;
}
inline
const Graphics::GraphicsEntity::Id&
AnimEventInfo::GetEntityId() const
{
    return this->id;
}
}