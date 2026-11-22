#pragma once
#include "timing/time.h"
namespace Math
{
template<class TYPE> class Extrapolator
{
public:
    Extrapolator();
    ~Extrapolator() = default;
    bool AddSample(Timing::Time packetTime, Timing::Time curTime, const TYPE& pos);
    bool AddSample(Timing::Time packetTime, Timing::Time curTime, const TYPE& pos, const TYPE& vel);
    void Reset(Timing::Time packetTime, Timing::Time curTime, const TYPE& pos);
    void Reset(Timing::Time packetTime, Timing::Time curTime, const TYPE& pos, const TYPE& vel);
    bool ReadValue(Timing::Time forTime, TYPE& oPos) const;
    bool ReadValue(Timing::Time forTime, TYPE& oPos, TYPE& oVel) const;
    Timing::Time EstimateLatency() const;
    Timing::Time EstimateUpdateTime() const;
    bool Estimates(Timing::Time packetTime, Timing::Time curTime);
    TYPE snapPos;
    TYPE snapVel;
    TYPE aimPos;
    TYPE lastPacketPos;
    Timing::Time snapTime;
    Timing::Time aimTime;
    Timing::Time lastPacketTime;
    Timing::Time latency;
    Timing::Time updateTime;
};
template<class TYPE>
Extrapolator<TYPE>::Extrapolator()
{
    this->Reset(0, 0, TYPE());
}
template<class TYPE> bool
Extrapolator<TYPE>::AddSample(Timing::Time packetTime, Timing::Time curTime, const TYPE& pos)
{
    TYPE vel(0);
    if (Math::abs(packetTime - this->lastPacketTime) > N_TINY)
    {
        float dt = (float)(1.0 / (packetTime - this->lastPacketTime));
        vel = (pos - this->lastPacketPos) * dt;
    }
    return this->AddSample(packetTime, curTime, pos, vel);
}
template<class TYPE> bool
Extrapolator<TYPE>::AddSample(Timing::Time packetTime, Timing::Time curTime, const TYPE& pos, const TYPE& vel)
{
    if (!Estimates(packetTime, curTime)) {
        return false;
    }
    this->lastPacketPos = pos;
    this->lastPacketTime = packetTime;
    this->ReadValue(curTime, this->snapPos);
    this->aimTime = curTime + this->updateTime;
    float dt = (float)(this->aimTime - packetTime);
    this->snapTime = curTime;
    this->aimPos = pos + vel * dt;
    if (Math::abs(this->aimTime - this->snapTime) < N_TINY)
    {
        this->snapVel = vel;
    }
    else
    {
        float dt = (float)(1.0 / (this->aimTime - this->snapTime));
        this->snapVel = (this->aimPos - this->snapPos) * dt;
    }
    return true;
}
template<class TYPE> void
Extrapolator<TYPE>::Reset(Timing::Time packetTime, Timing::Time curTime, const TYPE& pos)
{
    TYPE vel(0);
    this->Reset(packetTime, curTime, pos, vel);
}
template<class TYPE> void
Extrapolator<TYPE>::Reset(Timing::Time packetTime, Timing::Time curTime, const TYPE& pos, const TYPE& vel)
{
    n_assert(packetTime <= curTime);
    this->lastPacketTime = packetTime;
    this->lastPacketPos = pos;
    this->snapTime = curTime;
    this->snapPos = pos;
    this->updateTime = curTime - packetTime;
    this->latency = this->updateTime;
    this->aimTime = curTime + this->updateTime;
    this->snapVel = vel;
    this->aimPos = this->snapPos + this->snapVel * (float)this->updateTime;
}
template<class TYPE> bool
Extrapolator<TYPE>::ReadValue(Timing::Time forTime, TYPE& oPos) const
{
    TYPE vel;
    return this->ReadValue(forTime, oPos, vel);
}
template<class TYPE> bool
Extrapolator<TYPE>::ReadValue(Timing::Time forTime, TYPE& oPos, TYPE& oVel) const
{
    bool ok = true;
    if (forTime < this->snapTime)
    {
        forTime = this->snapTime;
        ok = false;
    }
    Timing::Time maxRange = this->aimTime + this->updateTime;
    if (forTime > maxRange)
    {
        forTime = maxRange;
        ok = false;
    }
    oVel = this->snapVel;
    oPos = this->snapPos + oVel * (float)(forTime - this->snapTime);
    return ok;
}
template<class TYPE> Timing::Time
Extrapolator<TYPE>::EstimateLatency() const
{
    return this->latency;
}
template<class TYPE> Timing::Time
Extrapolator<TYPE>::EstimateUpdateTime() const
{
    return this->updateTime;
}
template<class TYPE> bool
Extrapolator<TYPE>::Estimates(Timing::Time packet, Timing::Time cur)
{
    if (packet <= this->lastPacketTime)
    {
        return false;
    }
    Timing::Time lat = cur - packet;
    if (lat < 0) lat = 0;
    if (lat > this->latency)
    {
        this->latency = (this->latency + lat) * 0.5;
    }
    else
    {
        this->latency = (this->latency * 7 + lat) * 0.125;
    }
    Timing::Time tick = packet - this->lastPacketTime;
    if (tick > this->updateTime)
    {
        this->updateTime = (this->updateTime + tick) * 0.5;
    }
    else
    {
        this->updateTime = (this->updateTime * 7.0 + tick) * 0.125;
    }
    return true;
}
}