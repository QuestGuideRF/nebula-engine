#pragma once
#ifndef POSIX_POSIXFILETIME_H
#define POSIX_POSIXFILETIME_H
#include "core/types.h"
namespace Posix
{
class PosixFileTime
{
public:
    PosixFileTime();
    PosixFileTime(const Util::String& str);
    friend bool operator==(const PosixFileTime& a, const PosixFileTime& b);
    friend bool operator!=(const PosixFileTime& a, const PosixFileTime& b);
    friend bool operator>(const PosixFileTime& a, const PosixFileTime& b);
    friend bool operator<(const PosixFileTime& a, const PosixFileTime& b);
    uint GetHighBits() const;
    uint GetLowBits() const;
    void SetBits(uint lowbits, uint highbits);
    Util::String AsString() const;
private:
    friend class PosixFSWrapper;
    friend class PosixCalendarTime;
    timespec time;
};
inline
PosixFileTime::PosixFileTime()
{
    time.tv_sec = 0;
    time.tv_nsec = 0;
}
inline uint
PosixFileTime::GetHighBits() const
{
    return this->time.tv_sec;
}
inline uint
PosixFileTime::GetLowBits() const
{
    return this->time.tv_nsec;
}
inline void
PosixFileTime::SetBits(uint lowbits, uint highbits)
{
    this->time.tv_nsec = lowbits;
    this->time.tv_sec = highbits;
}
inline bool
operator==(const PosixFileTime& a, const PosixFileTime& b)
{
    return (a.time.tv_sec == b.time.tv_sec) && (a.time.tv_nsec == b.time.tv_nsec);
}
inline bool
operator!=(const PosixFileTime& a, const PosixFileTime& b)
{
    return (a.time.tv_sec != b.time.tv_sec) || (a.time.tv_nsec != b.time.tv_nsec);
}
inline bool
operator>(const PosixFileTime& a, const PosixFileTime& b)
{
    if (a.time.tv_sec < b.time.tv_sec) return false;
    if (a.time.tv_sec > b.time.tv_sec) return true;
    if (a.time.tv_nsec > b.time.tv_nsec) return true;
    return false;
}
inline bool
operator <(const PosixFileTime& a, const PosixFileTime& b)
{
    if (a.time.tv_sec > b.time.tv_sec) return false;
    if (a.time.tv_sec < b.time.tv_sec) return true;
    if (a.time.tv_nsec < b.time.tv_nsec) return true;
    return false;
}
};
#endif