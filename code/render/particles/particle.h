#pragma once
#include "math/vec4.h"
#include "math/mat4.h"
#include "math/bbox.h"
#include "particles/emitterattrs.h"
#include "threading/interlocked.h"
#include "threading/event.h"
namespace Particles
{
    static const SizeT ParticleSystemNumEnvelopeSamples = 192;
    static const SizeT MaxNumRenderedParticles = 65535;
    struct Particle
    {
        Math::vec4 position;
        Math::vec4 startPosition;
        Math::vec4 stretchPosition;
        Math::vec4 velocity;
        Math::vec4 uvMinMax;
        Math::vec4 color;
        float rotation;
        float rotationVariation;
        float size;
        float sizeVariation;
        float oneDivLifeTime;
        float relAge;
        float age;
        float particleId;
    };
    typedef unsigned int JOB_ID;
    struct ParticleJobUniformData
    {
        ParticleJobUniformData()
            : gravity(0.0f, 0.0f, 0.0f)
            , windVector(0.0f, 0.0f, 0.0f)
            , stretchToStart(false)
            , stretchTime(0)
            , sampleBuffer(nullptr)
        {};
        Math::vector gravity;
        Math::vector windVector;
        bool stretchToStart;
        float stretchTime;
        const float* sampleBuffer;
    };
    struct ParticleJobSliceOutputData
    {
        Math::bbox bbox;
        unsigned int numLivingParticles;
    };
    struct ParticleJobContext
    {
        const Particle* inputParticles;
        Particle* outputParticles;
        const ParticleJobUniformData* uniformData;
        float stepTime;
        uint numParticles;
        ParticleJobSliceOutputData* output;
    };
    static const SizeT ParticleJobInputElementSize = sizeof(Particle);
    static const SizeT ParticleJobInputMaxElementsPerSlice = JobMaxSliceSize / ParticleJobInputElementSize;
    static const SizeT ParticleJobInputSliceSize = ParticleJobInputMaxElementsPerSlice * ParticleJobInputElementSize;
}