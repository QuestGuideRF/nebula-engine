#pragma once
#include "core/refcounted.h"
#include "timing/time.h"
namespace CoreAnimation
{
class AnimKeyBuffer : public Core::RefCounted
{
    __DeclareClass(AnimKeyBuffer);
public:
    struct Interval
    {
        Timing::Tick start, end;
        uint key0, key1;
        float duration;
    };
    AnimKeyBuffer();
    virtual ~AnimKeyBuffer();
    void Setup(SizeT numIntervals, SizeT numKeys, void* intervalPtr, void* keyPtr);
    void Discard();
    bool IsValid() const;
    SizeT GetNumKeys() const;
    SizeT GetByteSize() const;
    const float* GetKeyBufferPointer() const;
    const AnimKeyBuffer::Interval* GetIntervalBufferPointer() const;
private:
    SizeT numKeys;
    SizeT numIntervals;
    uint mapCount;
    float* keyBuffer;
    AnimKeyBuffer::Interval* intervalBuffer;
};
inline bool
AnimKeyBuffer::IsValid() const
{
    return (0 != this->intervalBuffer);
}
inline SizeT
AnimKeyBuffer::GetNumKeys() const
{
    return this->numKeys;
}
inline SizeT
AnimKeyBuffer::GetByteSize() const
{
    return this->numKeys * sizeof(float);
}
inline const float*
AnimKeyBuffer::GetKeyBufferPointer() const
{
    return this->keyBuffer;
}
inline const AnimKeyBuffer::Interval*
AnimKeyBuffer::GetIntervalBufferPointer() const
{
    return this->intervalBuffer;
}
}