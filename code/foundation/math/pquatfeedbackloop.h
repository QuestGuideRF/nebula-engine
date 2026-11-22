#ifndef Math_PQUATFEEDBACKLOOP_H
#define Math_PQUATFEEDBACKLOOP_H
#include "core/types.h"
#include "math/quaternion.h"
#include "math/vector.h"
#include "timing/time.h"
namespace Math
{
using namespace Timing;
class PQuatFeedbackLoop
{
public:
    PQuatFeedbackLoop();
    void Reset(Time time, float stepSize, float gain, const quaternion& curState);
    void SetGain(float g);
    float GetGain() const;
    void SetGoal(const quaternion& c);
    const quaternion& GetGoal() const;
    void SetState(const quaternion& s);
    const quaternion& GetState() const;
    void Update(Time time);
private:
    Time time;
    float stepSize;
    float gain;
    quaternion goal;
    quaternion state;
};
inline
PQuatFeedbackLoop::PQuatFeedbackLoop() :
    time(0.0),
    stepSize(0.001f),
    gain(-1.0f)
{
}
inline
void
PQuatFeedbackLoop::Reset(Time t, float s, float g, const quaternion& curState)
{
    this->time = t;
    this->stepSize = s;
    this->gain = g;
    this->state = curState;
}
inline
void
PQuatFeedbackLoop::SetGain(float g)
{
    n_assert(g > 0.0f);
    this->gain = g;
}
inline
float
PQuatFeedbackLoop::GetGain() const
{
    return this->gain;
}
inline
void
PQuatFeedbackLoop::SetGoal(const quaternion& g)
{
    this->goal = g;
}
inline
const quaternion&
PQuatFeedbackLoop::GetGoal() const
{
    return this->goal;
}
inline
void
PQuatFeedbackLoop::SetState(const quaternion& s)
{
    this->state = s;
}
inline
const quaternion&
PQuatFeedbackLoop::GetState() const
{
    return this->state;
}
inline
void
PQuatFeedbackLoop::Update(Time curTime)
{
    Time dt = curTime - this->time;
    if (dt < 0.0)
    {
        this->time = curTime;
    }
    else if (dt > 0.5)
    {
        this->time = curTime;
    }
    while (this->time < curTime)
    {
        vector stateVec = this->state.rotate(vector(0.0f, 0.0f, 1.0f));
        vector goalVec  = this->goal.rotate(vector(0.0f, 0.0f, 1.0f));
        float dot = stateVec % goalVec;
        float error = (-dot + 1.0f) * 0.5f;
        float lerp = error * this->gain * this->stepSize;
        quaternion res;
        res.slerp(this->state, this->goal, lerp);
        this->state = res;
        this->time += this->stepSize;
    }
}
#endif