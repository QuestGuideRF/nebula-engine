#pragma once
#include "util/string.h"
#include "util/array.h"
#include "core/refcounted.h"
namespace ToolkitUtil
{
class Clip;
class Take : public Core::RefCounted
{
    __DeclareClass(Take);
public:
    Take();
    ~Take();
    void Cleanup();
    void AddClip(const Ptr<Clip>& clip);
    void RemoveClip(const Ptr<Clip>& clip);
    void RemoveClip(const uint index);
    const Ptr<Clip>& GetClip(const uint index);
    const Util::Array<Ptr<Clip> >& GetClips() const;
    const SizeT GetNumClips();
    const IndexT FindClip(const Ptr<Clip>& clip);
    void SetName(const Util::String& name);
    const Util::String& GetName() const;
private:
    Util::String name;
    Util::Array<Ptr<Clip> > clips;
};
}