#ifndef Math_PFEEDBACKLOOP_H
#define Math_PFEEDBACKLOOP_H
#include "core/types.h"
#include "timing/time.h"
namespace Math
{
template<class TYPE> class PFeedbackLoop
{
public:
    PFeedbackLoop();
    void Reset(Timing::Time time, float stepSize, float gain, const TYPE& curState);
    void SetGain(float g);
    float GetGain() const;
    void SetGoal(const TYPE& c);
    const TYPE& GetGoal() const;
    void SetState(const TYPE& s);
    const TYPE& GetState() const;
    void Update(Timing::Time time);
private:
    Timing::Time time;
    float stepSize;
    float gain;
    TYPE goal;
    TYPE state;
};
template<class TYPE>
PFeedbackLoop<TYPE>::PFeedbackLoop() :
    time(0.0),
    stepSize(0.001f),
    gain(-1.0f)
{
}
template<class TYPE>
void
PFeedbackLoop<TYPE>::Reset(Timing::Time t, float s, float g, const TYPE& curState)
{
    this->time = t;
    this->stepSize = s;
    this->gain = g;
    this->state = curState;
    this->goal  = curState;
}
template<class TYPE>
void
PFeedbackLoop<TYPE>::SetGain(float g)
{
    this->gain = g;
}
template<class TYPE>
float
PFeedbackLoop<TYPE>::GetGain() const
{
    return this->gain;
}
template<class TYPE>
void
PFeedbackLoop<TYPE>::SetGoal(const TYPE& g)
{
    this->goal = g;
}
template<class TYPE>
const TYPE&
PFeedbackLoop<TYPE>::GetGoal() const
{
    return this->goal;
}
template<class TYPE>
void
PFeedbackLoop<TYPE>::SetState(const TYPE& s)
{
    this->state = s;
}
template<class TYPE>
const TYPE&
PFeedbackLoop<TYPE>::GetState() const
{
    return this->state;
}
template<class TYPE>
void
PFeedbackLoop<TYPE>::Update(Timing::Time curTime)
{
    Timing::Time dt = curTime - this->time;
    if (dt < 0.0)
    {
        this->time = curTime;
    }
    else if (dt > 0.5)
    {
        this->time = curTime - 0.5;
    }
    while (this->time < curTime)
    {
        this->state = this->state + (this->state - this->goal) * this->gain * this->stepSize;
        this->time += this->stepSize;
    }
}
}
#endif