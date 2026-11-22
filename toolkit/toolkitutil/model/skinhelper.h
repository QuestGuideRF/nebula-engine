#pragma once
#include "core/refcounted.h"
#include "io/xmlreader.h"
namespace ToolkitUtil
{
class SkinHelper : public Core::RefCounted
{
    __DeclareClass(SkinHelper);
public:
    SkinHelper();
    virtual ~SkinHelper();
    void Setup(const Ptr<IO::XmlReader>& reader);
    const Util::Array<Util::String>& GetSkins() const;
    bool HasSkin(const Util::String& skin);
private:
    Util::Array<Util::String> skins;
};
inline bool
SkinHelper::HasSkin(const Util::String& skin)
{
    return this->skins.FindIndex(skin) != InvalidIndex;
}
inline const Util::Array<Util::String>&
SkinHelper::GetSkins() const
{
    return this->skins;
}
}