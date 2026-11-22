#pragma once
#include "core/types.h"
namespace OSX
{
class OSXFileTime
{
public:
    OSXFileTime();
    friend bool operator==(const OSXFileTime& a, const OSXFileTime& b);
    friend bool operator!=(const OSXFileTime& a, const OSXFileTime& b);
    friend bool operator>(const OSXFileTime& a, const OSXFileTime& b);
    friend bool operator<(const OSXFileTime& a, const OSXFileTime& b);
    time_t fileTime;
};
inline
OSXFileTime::OSXFileTime() :
    fileTime(0)
{
}
inline bool
operator==(const OSXFileTime& a, const OSXFileTime& b)
{
    return (a.fileTime == b.fileTime);
}
inline bool
operator!=(const OSXFileTime& a, const OSXFileTime& b)
{
    return (a.fileTime != b.fileTime);
}
inline bool
operator>(const OSXFileTime& a, const OSXFileTime& b)
{
    return (a.fileTime > b.fileTime);
}
inline bool
operator <(const OSXFileTime& a, const OSXFileTime& b)
{
    return (a.fileTime < b.fileTime);
}
}