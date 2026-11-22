#pragma once
#include "http/httprequesthandler.h"
#include "timing/time.h"
namespace Debug
{
class DebugPageHandler : public Http::HttpRequestHandler
{
    __DeclareClass(DebugPageHandler);
public:
    DebugPageHandler();
    virtual void HandleRequest(const Ptr<Http::HttpRequest>& request);
protected:
    void ComputeMinMaxAvgTimes(const Util::Array<Timing::Time>& times, Timing::Time& outMin, Timing::Time& outMax, Timing::Time& outAvg, Timing::Time& outCur) const;
    void ComputeMinMaxAvgCounts(const Util::Array<int>& counterValues, int& outMin, int& outMax, int& outAvg, int& outCur) const;
private:
    void HandleTimerRequest(const Util::String& timerName, const Ptr<Http::HttpRequest>& request);
    void HandleCounterRequest(const Util::String& counterName, const Ptr<Http::HttpRequest>& request);
    void HandleTimerChartRequest(const Util::String& timerName, const Ptr<Http::HttpRequest>& request);
    void HandleCounterChartRequest(const Util::String& counterName, const Ptr<Http::HttpRequest>& request);
    void HandleTimerTableSortRequest(const Util::String& columnName, const Util::String& tableName, const Ptr<Http::HttpRequest>& request);
    void HandleCounterTableSortRequest(const Util::String& columnName, const Util::String& tableName, const Ptr<Http::HttpRequest>& request);
    Util::Dictionary<Util::String, Util::String> timerSortColumns;
    Util::Dictionary<Util::String, Util::String> counterSortColumns;
};
}