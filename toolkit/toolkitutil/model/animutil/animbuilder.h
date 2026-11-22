#pragma once
#include "model/animutil/animbuilderclip.h"
namespace ToolkitUtil
{
class AnimBuilder
{
public:
    AnimBuilder();
    void Clear();
    void Reserve(SizeT numClips);
    void AddClip(const AnimBuilderClip& clip);
    SizeT GetNumClips() const;
    AnimBuilderClip& GetClipAtIndex(IndexT i) const;
    SizeT CountCurves() const;
    SizeT CountEvents() const;
    SizeT CountKeys() const;
    void BuildVelocityCurves(float keysPerMS);
    Util::Array<float> keys;
    Util::Array<Timing::Tick> keyTimes;
    Util::Array<AnimBuilderCurve> curves;
    Util::Array<CoreAnimation::AnimEvent> events;
    Util::Array<AnimBuilderClip> clips;
private:
};
inline SizeT
AnimBuilder::GetNumClips() const
{
    return this->clips.Size();
}
inline AnimBuilderClip&
AnimBuilder::GetClipAtIndex(IndexT i) const
{
    return this->clips[i];
}
}