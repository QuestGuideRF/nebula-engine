#pragma once
#include "core/refcounted.h"
#include "clipevent.h"
namespace ToolkitUtil
{
class Take;
class Clip : public Core::RefCounted
{
    __DeclareClass(Clip);
public:
    enum InfinityType
    {
        Constant,
        Cycle
    };
    Clip();
    virtual ~Clip();
    void Cleanup();
    void SetName(const Util::String& name);
    const Util::String& GetName() const;
    void SetTake(const Ptr<Take>& take);
    const Ptr<Take>& GetTake() const;
    void SetStart(int start);
    const int GetStart() const;
    void SetEnd(int end);
    const int GetEnd() const;
    void SetPreInfinity(InfinityType infinityType);
    const InfinityType GetPreInfinity() const;
    void SetPostInfinity(InfinityType infinityType);
    const InfinityType GetPostInfinity() const;
    const Util::Array<Ptr<ClipEvent>>& GetEvents() const;
    const SizeT GetNumEvents() const;
    void AddEvent(const Ptr<ClipEvent>& event);
    void RemoveEvent(const Ptr<ClipEvent>& event);
private:
    Util::String name;
    int start;
    int end;
    InfinityType preInfinity;
    InfinityType postInfinity;
    Ptr<Take> take;
    Util::Array<Ptr<ClipEvent>> events;
};
inline void
Clip::SetName( const Util::String& name )
{
    this->name = name;
}
inline const Util::String&
Clip::GetName() const
{
    return this->name;
}
inline void
Clip::SetTake( const Ptr<Take>& take )
{
    n_assert(take.isvalid());
    this->take = take;
}
inline const Ptr<Take>&
Clip::GetTake() const
{
    return this->take;
}
inline void
Clip::SetPreInfinity( Clip::InfinityType infinityType )
{
    this->preInfinity = infinityType;
}
inline const Clip::InfinityType
Clip::GetPreInfinity() const
{
    return this->preInfinity;
}
inline void
Clip::SetPostInfinity(  Clip::InfinityType infinityType )
{
    this->postInfinity = infinityType;
}
inline const Clip::InfinityType
Clip::GetPostInfinity() const
{
    return this->postInfinity;
}
inline const Util::Array<Ptr<ClipEvent>>&
Clip::GetEvents() const
{
    return this->events;
}
inline const SizeT
Clip::GetNumEvents() const
{
    return this->events.Size();
}
}