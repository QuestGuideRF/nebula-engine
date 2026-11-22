#ifndef POSIX_POSIXCALENDARTIME_H
#define POSIX_POSIXCALENDARTIME_H
#include "timing/base/calendartimebase.h"
namespace Posix
{
class PosixCalendarTime : public Base::CalendarTimeBase
{
public:
    static Timing::CalendarTime GetSystemTime();
    static Timing::CalendarTime GetLocalTime();
    static IO::FileTime SystemTimeToFileTime(const Timing::CalendarTime& systemTime);
    static Timing::CalendarTime FileTimeToSystemTime(const IO::FileTime& fileTime);
    static IO::FileTime LocalTimeToFileTime(const Timing::CalendarTime& localTime);
    static Timing::CalendarTime FileTimeToLocalTime(const IO::FileTime& fileTime);
private:
    static Timing::CalendarTime FromPosixSystemTime(const timeval & t);
    static timeval ToPosixSystemTime(const Timing::CalendarTime& calTime);
};
}
#endif