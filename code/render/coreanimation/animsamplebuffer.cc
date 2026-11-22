//------------------------------------------------------------------------------
//  animsamplebuffer.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coreanimation/animsamplebuffer.h"
namespace CoreAnimation
{
using namespace Math;

//------------------------------------------------------------------------------
/**
*/
AnimSampleBuffer::AnimSampleBuffer() :
    numSamples(0),
    samples(nullptr),
    sampleCounts(nullptr)
{
    // empty
}    

//------------------------------------------------------------------------------
/**
*/
AnimSampleBuffer::~AnimSampleBuffer()
{
    if (this->IsValid())
    {
        this->Discard();
    }
}

//------------------------------------------------------------------------------
/**
*/
AnimSampleBuffer::AnimSampleBuffer(AnimSampleBuffer&& rhs) :
    animResource(rhs.animResource),
    numSamples(rhs.numSamples)
{
    // стереть все текущие массивы
    if (this->samples)
        Memory::Free(Memory::ResourceHeap, this->samples);
    if (this->sampleCounts)
        Memory::Free(Memory::ResourceHeap, this->sampleCounts);

    this->samples = rhs.samples;
    this->sampleCounts = rhs.sampleCounts;

    rhs.numSamples = 0;
    rhs.samples = nullptr;
    rhs.sampleCounts = nullptr;
    rhs.animResource = InvalidAnimationId;
}

//------------------------------------------------------------------------------
/**
*/
AnimSampleBuffer::AnimSampleBuffer(const AnimSampleBuffer& rhs) :
    animResource(rhs.animResource),
    numSamples(rhs.numSamples)
{
    if (this->samples)
        Memory::Free(Memory::ResourceHeap, this->samples);
    if (this->sampleCounts)
        Memory::Free(Memory::ResourceHeap, this->sampleCounts);

    if (this->numSamples > 0)
    {
        this->samples = (float*)Memory::Alloc(Memory::ResourceHeap, this->numSamples * sizeof(float));
        memcpy(this->samples, rhs.samples, this->numSamples * sizeof(float));

        this->sampleCounts = (uchar*)Memory::Alloc(Memory::ResourceHeap, (this->numSamples * sizeof(uchar)) + 16);
        memcpy(this->sampleCounts, rhs.sampleCounts, (this->numSamples * sizeof(uchar)) + 16);
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
AnimSampleBuffer::operator=(AnimSampleBuffer&& rhs)
{
    this->numSamples = rhs.numSamples;
    this->animResource = rhs.animResource;

    // стереть все текущие массивы
    if (this->samples)
        Memory::Free(Memory::ResourceHeap, this->samples);
    if (this->sampleCounts)
        Memory::Free(Memory::ResourceHeap, this->sampleCounts);

    this->samples = rhs.samples;
    this->sampleCounts = rhs.sampleCounts;

    rhs.numSamples = 0;
    rhs.samples = nullptr;
    rhs.sampleCounts = nullptr;
    rhs.animResource = InvalidAnimationId;
}

//------------------------------------------------------------------------------
/**
*/
void
AnimSampleBuffer::operator=(const AnimSampleBuffer& rhs)
{
    this->numSamples = rhs.numSamples;
    this->animResource = rhs.animResource;
    if (this->samples)
        Memory::Free(Memory::ResourceHeap, this->samples);
    if (this->sampleCounts)
        Memory::Free(Memory::ResourceHeap, this->sampleCounts);

    if (this->numSamples > 0)
    {
        this->samples = (float*)Memory::Alloc(Memory::ResourceHeap, this->numSamples * sizeof(float));
        memcpy(this->samples, rhs.samples, this->numSamples * sizeof(float));

        this->sampleCounts = (uchar*)Memory::Alloc(Memory::ResourceHeap, (this->numSamples * sizeof(uchar)) + 16);
        memcpy(this->sampleCounts, rhs.sampleCounts, (this->numSamples * sizeof(uchar)) + 16);
    }   
}

//------------------------------------------------------------------------------
/**
*/
void
AnimSampleBuffer::Setup(const AnimationId& animRes)
{
    n_assert(!this->IsValid());
    n_assert(0 == this->samples);
    n_assert(0 == this->sampleCounts);

    this->animResource = animRes;
    const Util::FixedArray<AnimClip>& clips = AnimGetClips(this->animResource);
    SizeT maxCurveCount = 0;
    for (const auto& clip : clips)
        maxCurveCount = Math::max(maxCurveCount, clip.numCurves);

    // Количество образцов, которое нам нужно, будет равно количеству кривых, то есть pos/rot/scale/3 * 10.
    this->numSamples = (maxCurveCount / 3) * 10;
    this->samples      = (float*) Memory::Alloc(Memory::ResourceHeap, this->numSamples * sizeof(float));

    // ПРИМЕЧАНИЕ. Размер счетчика выборок должен быть равен 16 байтам, при этом в буфере будет выделено еще несколько байтов.
    this->sampleCounts = (uchar*)  Memory::Alloc(Memory::ResourceHeap, (maxCurveCount * sizeof(uchar)) + 16);
}

//------------------------------------------------------------------------------
/**
*/
void
AnimSampleBuffer::Discard()
{
    n_assert(this->IsValid());
    n_assert(0 != this->samples);
    n_assert(0 != this->sampleCounts);

    this->animResource = InvalidAnimationId;
    Memory::Free(Memory::ResourceHeap, this->samples);
    Memory::Free(Memory::ResourceHeap, this->sampleCounts);
    this->samples = nullptr;
    this->sampleCounts = nullptr;
}

} // пространство имен CoreAnimation
