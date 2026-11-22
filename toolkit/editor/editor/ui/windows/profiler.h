#pragma once
#include "editor/ui/window.h"
namespace Presentation
{
class Profiler : public BaseWindow
{
    __DeclareClass(Profiler)
public:
    Profiler();
    ~Profiler();
    void Run(SaveMode save) override;
private:
    bool pauseProfiling;
    bool showFrameProfiler;
    bool profileFixedFps;
    float averageFrameTime;
    float prevAverageFrameTime;
    float currentFrameTime;
    int fixedFps;
    Util::Array<float> frametimeHistory;
    Util::Array<Profiling::ProfilingContext> ProfilingContexts;
    Util::Array<CoreGraphics::FrameProfilingMarker> frameProfilingMarkers;
};
__RegisterClass(Profiler)
}