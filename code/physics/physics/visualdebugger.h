#pragma once
#include "core/refcounted.h"
#include "core/ptr.h"
#include "core/singleton.h"
#include "math/bbox.h"
namespace Physics
{
class Scene;
class VisualDebugger
{
    __DeclareInterfaceSingleton(VisualDebugger);
public:
    VisualDebugger();
    virtual ~VisualDebugger();
    virtual void OnStep();
    virtual void Initialize(Scene* s);
    void SetSimulationFrameTime(float time);
    float SetSimulationFrameTime() const;
    void DrawTimedArrow(const Math::point& p, const Math::vector& dir, uint numSteps, const Math::vec4& color = Math::vec4(1, 1, 1, 1));
    void DrawTimedLine(const Math::point& p, const Math::vector& dir, uint numSteps, const Math::vec4& color = Math::vec4(1, 1, 1, 1));
    void DrawTimedPlane(const Math::point& p, const Math::vector& dir, float scale, uint numSteps, const Math::vec4& color = Math::vec4(1, 1, 1, 1));
    void DrawTimedText(const Math::point& p, const Util::String& text, uint numSteps, const Math::vec4& color = Math::vec4(1, 1, 1, 1));
    void DrawTimedAABB(const Math::bbox& bbox, uint numSteps, const Math::vec4& color = Math::vec4(1, 1, 1, 1));
    virtual void SetCameraView(const Math::mat4& viewMatrix);
    virtual void DrawArrow(const Math::point& p, const Math::vector& dir, const Math::vec4& color = Math::vec4(1, 1, 1, 1));
    virtual void DrawLine(const Math::point& p, const Math::vector& dir, const Math::vec4& color = Math::vec4(1, 1, 1, 1));
    virtual void DrawPlane(const Math::point& p, const Math::vector& dir, float scale, const Math::vec4& color = Math::vec4(1, 1, 1, 1));
    virtual void DrawText(const Math::point& p, const Util::String& text, const Math::vec4& color = Math::vec4(1, 1, 1, 1));
    virtual void DrawAABB(const Math::bbox& bbox, const Math::vec4& color = Math::vec4(1, 1, 1, 1));
protected:
    class TimedDrawData;
    void Draw(const TimedDrawData& data);
    class TimedDrawData
    {
    public:
        enum DrawType
        {
            Unitialized = -1,
            Arrow = 0,
            Text3D,
            AABB,
            Line,
            Plane
        };
        TimedDrawData():
          type(Unitialized)
        { }
        TimedDrawData(uint numSteps, DrawType t, const Math::vec4& c, const Math::point& vec1, const Math::vector& vec2, const Util::String& txt, float _f):
          stepsLeft(numSteps),
          type(t),
          color(c),
          v1(vec1),
          v2(vec2),
          text(txt),
          f(_f)
        { }
        uint stepsLeft;
        DrawType type;
        Math::vec4 color;
        Math::point v1;
        Math::vector v2;
        float f;
        Util::String text;
    };
    Scene* scene;
    float simulationFrameTime;
    Util::Array<TimedDrawData> timedDrawData;
};
inline void
VisualDebugger::SetSimulationFrameTime(float time)
{
    this->simulationFrameTime = time;
}
inline float
VisualDebugger::SetSimulationFrameTime() const
{
    return this->simulationFrameTime;
}
}