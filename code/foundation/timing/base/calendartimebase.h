#pragma once
#include "core/types.h"
#include "io/filetime.h"
#include "util/string.h"
namespace Timing
{
    class CalendarTime;
}
namespace Base
{
class CalendarTimeBase
{
public:
    typedef unsigned int Year;
    typedef unsigned int Day;
    typedef unsigned int Hour;
    typedef unsigned int Minute;
    typedef unsigned int Second;
    typedef unsigned int MilliSecond;
    enum Month
    {
        January = 1,
        February,
        March,
        April,
        May,
        June,
        July,
        August,
        September,
        October,
        November,
        December,
    };
    enum Weekday
    {
        Sunday = 0,
        Monday,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday,
    };
    CalendarTimeBase();
    void SetYear(Year y);
    Year GetYear() const;
    void SetMonth(Month m);
    Month GetMonth() const;
    void SetWeekday(Weekday wd);
    Weekday GetWeekday() const;
    void SetDay(Day d);
    Day GetDay() const;
    void SetHour(Hour h);
    Hour GetHour() const;
    void SetMinute(Minute m);
    Minute GetMinute() const;
    void SetSecond(Second s);
    Second GetSecond() const;
    void SetMilliSecond(MilliSecond ms);
    MilliSecond GetMilliSecond() const;
    static Timing::CalendarTime GetSystemTime();
    static Timing::CalendarTime GetLocalTime();
    static IO::FileTime SystemTimeToFileTime(const Timing::CalendarTime& systemTime);
    static Timing::CalendarTime FileTimeToSystemTime(const IO::FileTime& fileTime);
    static IO::FileTime LocalTimeToFileTime(const Timing::CalendarTime& localTime);
    static Timing::CalendarTime FileTimeToLocalTime(const IO::FileTime& fileTime);
    static Util::String Format(const Util::String& fmtString, const Timing::CalendarTime& calTime);
    static Util::String MonthToString(Month m);
    static Month StringToMonth(const Util::String& str);
    static Util::String WeekdayToString(Weekday d);
    static Weekday StringToWeekday(const Util::String& str);
protected:
    Year year;
    Month month;
    Weekday weekday;
    Day day;
    Hour hour;
    Minute minute;
    Second second;
    MilliSecond milliSecond;
};
}