#ifndef Math_PIDFEEDBACKLOOP_H
#define Math_PIDFEEDBACKLOOP_H
#include "core/types.h"
#include "timing/time.h"
namespace Math
{
class PIDFeedbackLoop
{
public:
    PIDFeedbackLoop();
    void SetState(double value);
    void SetGoal(double wantedValue);
    void SetConstants(double pConst, double iConst, double dConst, double acceleration = 0.0);
    const double& GetState() const;
    const double& GetGoal() const;
    double GetLastError() const;
    double GetLastDelta() const;
    void Update(Timing::Time time);
    void ResetError();
    void SetIsAngularValue(bool val);
private:
    double value;
    double goal;
    double pConst;
    double iConst;
    double dConst;
    double maxAcceleration;
    double lastError;
    double lastDelta;
    double runningError;
    bool validError;
    bool isAngularValue;
    Timing::Time lastTime;
    Timing::Time lastDeltaTime;
    Timing::Time maxAllowableDeltaTime;
};
inline
PIDFeedbackLoop::PIDFeedbackLoop() :
    value(0.0),
    goal(0.0),
    pConst(1.0),
    iConst(0.0),
    dConst(0.0),
    maxAcceleration(0.0),
    lastError(0.0),
    lastDelta(0.0),
    runningError(0.0),
    validError(false),
    isAngularValue(false),
    lastTime(0.0),
    lastDeltaTime(0.0),
    maxAllowableDeltaTime(0.03)
{
}
inline
void
PIDFeedbackLoop::SetState(double value)
{
    this->value = value;
    lastError = 0.0;
    lastDelta = 0.0;
}
inline
void
PIDFeedbackLoop::SetGoal(double wantedValue)
{
    this->goal = wantedValue;
}
inline
void
PIDFeedbackLoop::SetConstants(double pConst, double iConst, double dConst, double acceleration)
{
    this->pConst = pConst;
    this->iConst = iConst;
    this->dConst = dConst;
    maxAcceleration = acceleration;
}
inline
const double&
PIDFeedbackLoop::GetState() const
{
    return this->value;
}
inline
double
PIDFeedbackLoop::GetLastError() const
{
    return this->lastError;
}
inline
const double&
PIDFeedbackLoop::GetGoal() const
{
    return this->goal;
}
inline
double
PIDFeedbackLoop::GetLastDelta() const
{
    return this->lastDelta;
}
inline
void
PIDFeedbackLoop::SetIsAngularValue(bool val)
{
    this->isAngularValue = val;
}
inline
void
PIDFeedbackLoop::Update(Timing::Time time)
{
    Timing::Time frameTime = time - this->lastTime;
    if (frameTime != 0.0f)
    {
        if (frameTime > maxAllowableDeltaTime)
            frameTime = maxAllowableDeltaTime;
        double difference;
        if (this->isAngularValue)
        {
            difference = (double)n_angulardistance((float)value, (float)goal);
        }
        else
        {
            difference = goal - value;
        }
        double error = difference * frameTime;
        runningError += error;
        double dP = pConst * error;
        double dI = iConst * runningError * frameTime;
        double dD(0.0f);
        if (validError)
            dD = dConst * (lastError - error) * frameTime;
        else
            validError = true;
        lastError = error;
        double thisDelta = dP + dI + dD;
        if (maxAcceleration != 0.0f)
        {
            double timeRatio(1.0);
            if (lastDeltaTime != 0.0)
                timeRatio = frameTime / lastDeltaTime;
            lastDeltaTime = frameTime;
            lastDelta *= timeRatio;
            double difference = (thisDelta - lastDelta);
            double accl = maxAcceleration * frameTime * frameTime;
            if (difference < -accl)
                thisDelta = lastDelta - accl;
            else if (difference > accl)
                thisDelta = lastDelta + accl;
        }
        value += thisDelta;
        if (this->isAngularValue)
        {
            value = (double)n_modangle((float)value);
        }
        lastDelta = thisDelta;
    }
    this->lastTime = time;
}
inline
void
PIDFeedbackLoop::ResetError()
{
    runningError = 0.0f;
    validError = false;
}
}
#endif