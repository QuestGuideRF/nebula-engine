#pragma once
#ifndef WIN32_WIN32CALENDARTIME_H
#define WIN32_WIN32CALENDARTIME_H
#include "timing/base/calendartimebase.h"
namespace Win32
{
class Win32CalendarTime : public Base::CalendarTimeBase
{
public:
    static Timing::CalendarTime GetSystemTime();
    static Timing::CalendarTime GetLocalTime();
    static IO::FileTime SystemTimeToFileTime(const Timing::CalendarTime& systemTime);
    static Timing::CalendarTime FileTimeToSystemTime(const IO::FileTime& fileTime);
    static IO::FileTime LocalTimeToFileTime(const Timing::CalendarTime& localTime);
    static Timing::CalendarTime FileTimeToLocalTime(const IO::FileTime& fileTime);
private:
    static Timing::CalendarTime FromWin32SystemTime(const SYSTEMTIME& t);
    static SYSTEMTIME ToWin32SystemTime(const Timing::CalendarTime& calTime);
};
}
#endif