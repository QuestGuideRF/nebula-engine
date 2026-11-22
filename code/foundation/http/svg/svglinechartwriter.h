#pragma once
#include "http/svg/svgpagewriter.h"
#include "timing/time.h"
namespace Http
{
class SvgLineChartWriter : public SvgPageWriter
{
    __DeclareClass(SvgLineChartWriter);
public:
    SvgLineChartWriter();
    virtual ~SvgLineChartWriter();
    virtual bool Open();
    void SetupXAxis(const Util::String& axisName, const Util::String& unitName, int minVal, int maxVal);
    void SetupYAxis(const Util::String& axisName, const Util::String& unitName, float minVal, float maxVal);
    void AddTrack(const Util::String& name, const Util::String& color, const Util::Array<float>& values);
    void AddTrack(const Util::String& name, const Util::String& color, const Util::Array<float>& values, const Util::Array<Timing::Tick>& timeStamps);
    void Draw();
private:
    struct Track
    {
        Util::String name;
        Util::String color;
        Util::Array<float> values;
        Util::Array<Timing::Tick> times;
    };
    Util::String xAxisName;
    Util::String xAxisUnitName;
    int xAxisMinVal;
    int xAxisMaxVal;
    Util::String yAxisName;
    Util::String yAxisUnitName;
    float yAxisMinVal;
    float yAxisMaxVal;
    Util::Array<Track> tracks;
};
}