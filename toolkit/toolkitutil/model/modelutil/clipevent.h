#pragma once
#include "core/refcounted.h"
#include "timing/time.h"
namespace ToolkitUtil
{
class ClipEvent : public Core::RefCounted
{
    __DeclareClass(ClipEvent);
public:
    enum MarkerType
    {
        Ticks,
        Frames
    };
    ClipEvent();
    virtual ~ClipEvent();
    void SetName(const Util::String& name);
    const Util::String& GetName() const;
    void SetCategory(const Util::String& str);
    const Util::String& GetCategory() const;
    bool HasCategory() const;
    void SetMarkerAsMilliseconds(Timing::Tick time);
    void SetMarkerAsFrames(int frame);
    const int GetMarker() const;
    const MarkerType GetMarkerType() const;
private:
    MarkerType markerType;
    Util::String name;
    Util::String category;
    Timing::Tick time;
};
inline void
ClipEvent::SetName(const Util::String& n)
{
    this->name = n;
}
inline const Util::String&
ClipEvent::GetName() const
{
    return this->name;
}
inline void
ClipEvent::SetCategory(const Util::String& str)
{
    this->category = str;
}
inline const Util::String&
ClipEvent::GetCategory() const
{
    return this->category;
}
inline bool
ClipEvent::HasCategory() const
{
    return this->category.IsValid();
}
inline const int
ClipEvent::GetMarker() const
{
    return this->time;
}
inline const ClipEvent::MarkerType
ClipEvent::GetMarkerType() const
{
    return this->markerType;
}
}