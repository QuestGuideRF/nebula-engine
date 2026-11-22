#pragma once
#include "core/singleton.h"
#include "core/refcounted.h"
#include "animsplitterhelper.h"
#include "skinhelper.h"
namespace ToolkitUtil
{
class BatchAttributes : public Core::RefCounted
{
    __DeclareClass(BatchAttributes);
    __DeclareSingleton(BatchAttributes);
public:
    BatchAttributes();
    virtual ~BatchAttributes();
    void Open();
    void Close();
    bool IsOpen() const;
    const Ptr<AnimSplitterHelper>& GetSplitter(const Util::String& resource);
    bool HasSplitter(const Util::String& resource);
    const Ptr<SkinHelper>& GetSkin(const Util::String& resource);
    bool HasSkin(const Util::String& resource);
private:
    void Load();
    Util::Dictionary<Util::String, Ptr<AnimSplitterHelper> > animSplitters;
    Util::Dictionary<Util::String, Ptr<SkinHelper> > skins;
    bool isOpen;
};
inline const Ptr<AnimSplitterHelper>&
BatchAttributes::GetSplitter(const Util::String& resource)
{
    return this->animSplitters[resource];
}
inline bool
BatchAttributes::HasSplitter(const Util::String& resource)
{
    return this->animSplitters.FindIndex(resource) >= 0;
}
inline const Ptr<SkinHelper>&
BatchAttributes::GetSkin(const Util::String& resource)
{
    return this->skins[resource];
}
inline bool
BatchAttributes::HasSkin(const Util::String& resource)
{
    return this->skins.Contains(resource);
}
}