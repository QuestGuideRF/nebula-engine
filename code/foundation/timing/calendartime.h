#pragma once
#include "core/config.h"
#if __WIN32__
#include "timing/win32/win32calendartime.h"
namespace Timing
{
class CalendarTime : public Win32::Win32CalendarTime
{ };
}
#elif __linux__
#include "timing/posix/posixcalendartime.h"
namespace Timing
{
class CalendarTime : public Posix::PosixCalendarTime
{ };
}
#else
#error "Timing::CalendarTime не реализован на этой платформе!"
#endif