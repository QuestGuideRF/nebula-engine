#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "profiling/profiling.h"
#include "coregraphics/commandbuffer.h"
namespace Dynui
{
class ImguiProfiler : public Core::RefCounted
{
    __DeclareClass(ImguiProfiler);
    __DeclareInterfaceSingleton(ImguiProfiler);
public:
    ImguiProfiler();
    ~ImguiProfiler();
    void Capture();
    void Render(Timing::Time frameTime, IndexT frameIndex);
    void TogglePause();
private:
    bool pauseProfiling;
    bool showFrameProfiler;
    bool profileFixedFps;
    float averageFrameTime;
    float prevAverageFrameTime;
    float currentFrameTime;
    int fixedFps;
    Util::Array<float> frametimeHistory;
    Util::Array<Profiling::ProfilingContext> profilingContexts;
    Util::Array<CoreGraphics::FrameProfilingMarker> frameProfilingMarkers;
};
}