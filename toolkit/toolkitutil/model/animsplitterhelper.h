#pragma once
#include "core/refcounted.h"
#include "coreanimation/infinitytype.h"
#include "io/xmlreader.h"
namespace ToolkitUtil
{
class AnimSplitterHelper : public Core::RefCounted
{
    __DeclareClass(AnimSplitterHelper);
public:
    struct Split
    {
        Util::String name;
        int startOffset;
        int endOffset;
        CoreAnimation::InfinityType::Code postInfinity;
        CoreAnimation::InfinityType::Code preInfinity;
        Split()
        {
            postInfinity = CoreAnimation::InfinityType::Constant;
            preInfinity = CoreAnimation::InfinityType::Constant;
        }
    };
    AnimSplitterHelper();
    virtual ~AnimSplitterHelper();
    void Setup(Ptr<IO::XmlReader> reader);
    const Util::Array<Split>& GetTake(const Util::String& take);
    bool HasTake(const Util::String& take);
private:
    Util::String batch;
    Util::Dictionary<Util::String, Util::Array<Split> > takes;
};
inline const Util::Array<AnimSplitterHelper::Split>&
AnimSplitterHelper::GetTake(const Util::String& take)
{
    return this->takes[take];
}
inline bool
AnimSplitterHelper::HasTake(const Util::String& take)
{
    return this->takes.FindIndex(take) >= 0;
}
}