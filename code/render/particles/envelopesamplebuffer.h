#pragma once
#include "particles/envelopecurve.h"
#include "particles/emitterattrs.h"
namespace Particles
{
class EnvelopeSampleBuffer
{
public:
    EnvelopeSampleBuffer();
    ~EnvelopeSampleBuffer();
    void Setup(const EmitterAttrs& emitterAttrs, SizeT numSamples);
    void Discard();
    bool IsValid() const;
    IndexT AsSampleIndex(float t) const;
    float* LookupSamples(IndexT sampleIndex) const;
    SizeT GetNumSamples() const;
    const float* GetSampleBuffer() const;
private:
    SizeT numSamples;
    float* buffer;
};
inline bool
EnvelopeSampleBuffer::IsValid() const
{
    return (0 != this->buffer);
}
inline IndexT
EnvelopeSampleBuffer::AsSampleIndex(float t) const
{
    IndexT index = IndexT(t / this->numSamples);
    index = Math::clamp(index, 0, this->numSamples - 1);
    return index;
}
inline float*
EnvelopeSampleBuffer::LookupSamples(IndexT sampleIndex) const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert((sampleIndex >= 0) && (sampleIndex < this->numSamples));
    n_assert(0 != buffer);
    #endif
    return this->buffer + (sampleIndex * EmitterAttrs::NumEnvelopeAttrs);
}
inline SizeT
EnvelopeSampleBuffer::GetNumSamples() const
{
    return this->numSamples;
}
inline const float*
EnvelopeSampleBuffer::GetSampleBuffer() const
{
    return this->buffer;
}
}