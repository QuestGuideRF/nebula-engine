#pragma once
#include "math/scalar.h"
#include "core/types.h"
namespace Particles
{
class EnvelopeCurve
{
public:
    enum ModFunc
    {
        Sine = 0,
        Cosine,
    };
    EnvelopeCurve();
    void Setup(float val0, float val1, float val2, float val3, float keyPos0, float keyPos1, float freq, float amp, ModFunc mod);
    float Sample(float t) const;
    void PreSample(float* sampleBuffer, SizeT numSamples, SizeT sampleStride) const;
    float GetMaxValue() const;
    float GetMinValue() const;
    const float* GetValues() const;
    void SetValues(float v0, float v1, float v2, float v3);
    const float* GetLimits() const;
    void SetLimits(float min, float max);
    const float GetKeyPos0() const;
    void SetKeyPos0(float f);
    const float GetKeyPos1() const;
    void SetKeyPos1(float f);
    const float GetFrequency() const;
    void SetFrequency(float f);
    const float GetAmplitude() const;
    void SetAmplitude(float f);
    const int GetModFunc() const;
    void SetModFunc(int func);
private:
    float values[4];
    float limits[2];
    float keyPos0;
    float keyPos1;
    float frequency;
    float amplitude;
    ModFunc modFunction : 1;
};
inline const float*
EnvelopeCurve::GetValues() const
{
    return this->values;
}
inline void
EnvelopeCurve::SetValues( float v0, float v1, float v2, float v3 )
{
    this->values[0] = v0;
    this->values[1] = v1;
    this->values[2] = v2;
    this->values[3] = v3;
}
inline const float
EnvelopeCurve::GetKeyPos0() const
{
    return this->keyPos0;
}
inline void
EnvelopeCurve::SetKeyPos0( float f )
{
    this->keyPos0 = f;
}
inline const float
EnvelopeCurve::GetKeyPos1() const
{
    return this->keyPos1;
}
inline void
EnvelopeCurve::SetKeyPos1( float f )
{
    this->keyPos1 = f;
}
inline const float
EnvelopeCurve::GetFrequency() const
{
    return this->frequency;
}
inline void
EnvelopeCurve::SetFrequency( float f )
{
    this->frequency = f;
}
inline const float
EnvelopeCurve::GetAmplitude() const
{
    return this->amplitude;
}
inline void
EnvelopeCurve::SetAmplitude( float f )
{
    this->amplitude = f;
}
inline const int
EnvelopeCurve::GetModFunc() const
{
    return this->modFunction;
}
inline void
EnvelopeCurve::SetModFunc( int i )
{
    this->modFunction = (ModFunc)i;
}
inline const float*
EnvelopeCurve::GetLimits() const
{
    return this->limits;
}
inline void
EnvelopeCurve::SetLimits( float min, float max )
{
    this->limits[0] = min;
    this->limits[1] = max;
}
}