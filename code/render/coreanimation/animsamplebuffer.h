#pragma once
#include "core/refcounted.h"
#include "animation.h"
namespace CoreAnimation
{
class AnimSampleBuffer
{
public:
    AnimSampleBuffer();
    virtual ~AnimSampleBuffer();
    AnimSampleBuffer(AnimSampleBuffer&& rhs);
    AnimSampleBuffer(const AnimSampleBuffer& rhs);
    void operator=(AnimSampleBuffer&& rhs);
    void operator=(const AnimSampleBuffer& rhs);
    void Setup(const AnimationId& animResource);
    void Discard();
    bool IsValid() const;
    void Reset();
    SizeT GetNumSamples() const;
    float* GetSamplesPointer() const;
    uchar* GetSampleCountsPointer() const;
private:
    AnimationId animResource;
    SizeT numSamples;
    float* samples;
    uchar* sampleCounts;
};
inline bool
AnimSampleBuffer::IsValid() const
{
    return this->animResource != InvalidAnimationId;
}
inline SizeT
AnimSampleBuffer::GetNumSamples() const
{
    return this->numSamples;
}
inline float*
AnimSampleBuffer::GetSamplesPointer() const
{
    return this->samples;
}
inline uchar*
AnimSampleBuffer::GetSampleCountsPointer() const
{
    return this->sampleCounts;
}
}