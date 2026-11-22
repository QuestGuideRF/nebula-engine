//------------------------------------------------------------------------------
//  envelopesamplebuffer.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "particles/envelopesamplebuffer.h"

namespace Particles
{

//------------------------------------------------------------------------------
/**
*/
EnvelopeSampleBuffer::EnvelopeSampleBuffer() :
    numSamples(0),
    buffer(0)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
EnvelopeSampleBuffer::~EnvelopeSampleBuffer()
{
    if (this->IsValid())
    {
        this->Discard();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
EnvelopeSampleBuffer::Setup(const EmitterAttrs& emitterAttrs, SizeT numSamp)
{
    n_assert(!this->IsValid());
    this->numSamples = numSamp;

    // выделить буфер выборки
    SizeT size = this->numSamples * EmitterAttrs::NumEnvelopeAttrs * sizeof(float);   
    this->buffer = (float*) Memory::Alloc(Memory::ResourceHeap, size);

    // огибающие кривые перед выборкой в ​​буфер выборки
    IndexT i;
    for (i = 0; i < EmitterAttrs::NumEnvelopeAttrs; i++)
    {
        float* samplePtr = this->buffer + i;
        emitterAttrs.GetEnvelope((EmitterAttrs::EnvelopeAttr)i).PreSample(samplePtr, this->numSamples, EmitterAttrs::NumEnvelopeAttrs);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
EnvelopeSampleBuffer::Discard()
{
    n_assert(this->IsValid());
    n_assert(0 != this->buffer);

    Memory::Free(Memory::ResourceHeap, this->buffer);
    this->buffer = 0;
    this->numSamples = 0;
}

} // Частицы пространства имен