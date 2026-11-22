//------------------------------------------------------------------------------
//  backend/tbuisystem.cc
//  (C) 2024 г. Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "render/stdneb.h"
#include "core/sysfunc.h"
#include "timing/calendartime.h"
#include "tbuicontext.h"
#include "tb_system.h"

#ifdef TB_RUNTIME_DEBUG_INFO

//------------------------------------------------------------------------------
/**
*/
void
TBDebugOut(const char* str)
{
    Core::SysFunc::DebugOut(str);
}

#endif // TB_RUNTIME_DEBUG_INFO

namespace tb
{
namespace
{

// надеюсь, нам это не понадобится, оставим пока
#if 0
//------------------------------------------------------------------------------
/**
*/
static double
GetMillisecondsSinceEpoch(Timing::CalendarTime calendarTime)
{
    int year = calendarTime.GetYear();
    int month = calendarTime.GetMonth();
    int day = calendarTime.GetDay();
    int hour = calendarTime.GetHour();
    int minute = calendarTime.GetMinute();
    int second = calendarTime.GetSecond();
    int millisecond = calendarTime.GetMilliSecond();

    // Проверка ввода (базовые проверки)
    if (month < 1 || month > 12 || day < 1 || day > 31 || hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 ||
        second > 59 || millisecond < 0 || millisecond > 999)
    {
        return -1.0; // Failure
    }

    // Построение структуры tm
    std::tm timeinfo = {};
    timeinfo.tm_year = year - 1900; // tm_year — это годы, прошедшие с 1900 года.
    timeinfo.tm_mon = month - 1;    // tm_mon отсчитывается от 0
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;

    // Преобразовать в time_t (секунды с начала эпохи)
    std::time_t time_since_epoch = std::mktime(&timeinfo);

    // Проверьте недействительную конвертацию времени
    if (time_since_epoch == -1)
    {
        return -1.0; // Failure
    }

    // Конвертировать в миллисекунды как двойной
    return static_cast<double>(time_since_epoch) * 1000.0 + static_cast<double>(millisecond);
}
#endif
}

//------------------------------------------------------------------------------
/** 
    Получите системное время в миллисекундах с некоторой неопределенной эпохи.
*/
double
TBSystem::GetTimeMS()
{
    n_assert(TBUI::TBUIContext::state.timer.isvalid());
    return TBUI::TBUIContext::state.timer->GetTime();
}

//------------------------------------------------------------------------------
/** 
    Called when the need to call TBMessageHandler::ProcessMessages has changed 
    due to changes in the message queue. fire_time is the new time is needs to be 
    called.
    It may be 0 which means that ProcessMessages should be called asap (but NOT 
    from this call!) It may also be TB_NOT_SOON which means that ProcessMessages 
    doesn't need to be called. 
*/
void
TBSystem::RescheduleTimer(double fire_time)
{
}

//------------------------------------------------------------------------------
/** 
    Узнайте, сколько миллисекунд должно пройти после события приземления. 
    генерировать событие длинного клика. 
*/
int
TBSystem::GetLongClickDelayMS()
{
    return 500;
}

//------------------------------------------------------------------------------
/** 
    Узнайте, на сколько пикселей при перетаскивании должно начаться панорамирование прокручиваемых виджетов. 
*/
int
TBSystem::GetPanThreshold()
{
    return 40;
}

//------------------------------------------------------------------------------
/** 
    Узнайте, сколько пикселей составляет типичная строка: длина, которую следует прокручивать. 
    при повороте колеса мыши на одну ступень. 
*/
int
TBSystem::GetPixelsPerLine()
{
    return 40;
}

//------------------------------------------------------------------------------
/** 
    Получите количество точек на дюйм для главного экрана. 
*/
int
TBSystem::GetDPI()
{
    return 96;
}
} // пространство имен ТБ
