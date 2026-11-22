#pragma once
#include "core/types.h"
#include "util/string.h"
namespace Win32
{
class Win32FileTime
{
public:
    Win32FileTime();
    Win32FileTime(const Util::String& str);
    friend bool operator==(const Win32FileTime& a, const Win32FileTime& b);
    friend bool operator!=(const Win32FileTime& a, const Win32FileTime& b);
    friend bool operator>(const Win32FileTime& a, const Win32FileTime& b);
    friend bool operator<(const Win32FileTime& a, const Win32FileTime& b);
    Util::String AsString() const;
    uint GetHighBits() const;
    uint GetLowBits() const;
    void SetBits(uint lowBits, uint highBits);
    FILETIME time;
};
inline
Win32FileTime::Win32FileTime()
{
    time.dwLowDateTime = 0;
    time.dwHighDateTime = 0;
}
inline bool
operator==(const Win32FileTime& a, const Win32FileTime& b)
{
    return (0 == CompareFileTime(&(a.time), &(b.time)));
}
inline bool
operator!=(const Win32FileTime& a, const Win32FileTime& b)
{
    return (0 != CompareFileTime(&(a.time), &(b.time)));
}
inline bool
operator>(const Win32FileTime& a, const Win32FileTime& b)
{
    return (1 == CompareFileTime(&(a.time), &(b.time)));
}
inline bool
operator <(const Win32FileTime& a, const Win32FileTime& b)
{
    return (-1 == CompareFileTime(&(a.time), &(b.time)));
}
};