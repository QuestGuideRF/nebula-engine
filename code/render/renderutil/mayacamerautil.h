#pragma once
#ifndef RENDERUTIL_MAYACAMERAUTIL_H
#define RENDERUTIL_MAYACAMERAUTIL_H
#include "math/mat4.h"
#include "math/vec2.h"
#include "math/polar.h"
#include "math/scalar.h"
namespace RenderUtil
{
class MayaCameraUtil
{
public:
    MayaCameraUtil();
    void Setup(const Math::point& defaultCenterOfInterest, const Math::point& defaultEyePos, const Math::vector& defaultUpVec);
    void Reset();
    void Update(float deltaTime);
    const Math::mat4& GetCameraTransform() const;
    const Math::scalar& GetViewDistance() const;
    const Math::point& GetCenterOfInterest() const;
    void SetOrbitButton(bool b);
    void SetPanButton(bool b);
    void SetZoomButton(bool b);
    void SetZoomInButton(bool b);
    void SetZoomOutButton(bool b);
    void SetMouseMovement(const Math::vec2& v);
    void SetZoomIn(float v);
    void SetZoomOut(float v);
    void SetPanning(const Math::vec2& v);
    void SetOrbiting(const Math::vec2& v);
    void SetCenterOfInterest(const Math::point& point);
    void SetViewDistance(const Math::scalar& distance);
private:
    Math::point defaultCenterOfInterest;
    Math::point defaultEyePos;
    Math::vector defaultUpVec;
    Math::polar viewAngles;
    Math::scalar viewDistance;
    Math::point centerOfInterest;
    Math::mat4 cameraTransform;
    bool orbitButton;
    bool panButton;
    bool zoomButton;
    bool zoomInButton;
    bool zoomOutButton;
    Math::vec2 mouseMovement;
    float zoomIn;
    float zoomOut;
    Math::vec2 panning;
    Math::vec2 orbiting;
};
inline void
MayaCameraUtil::SetOrbitButton(bool b)
{
    this->orbitButton = b;
}
inline void
MayaCameraUtil::SetPanButton(bool b)
{
    this->panButton = b;
}
inline void
MayaCameraUtil::SetZoomButton(bool b)
{
    this->zoomButton = b;
}
inline void
MayaCameraUtil::SetZoomInButton(bool b)
{
    this->zoomInButton = b;
}
inline void
MayaCameraUtil::SetZoomOutButton(bool b)
{
    this->zoomOutButton = b;
}
inline void
MayaCameraUtil::SetMouseMovement(const Math::vec2& v)
{
    this->mouseMovement = v;
}
inline const Math::mat4&
MayaCameraUtil::GetCameraTransform() const
{
    return this->cameraTransform;
}
inline const Math::scalar&
MayaCameraUtil::GetViewDistance() const
{
    return this->viewDistance;
}
inline const Math::point&
MayaCameraUtil::GetCenterOfInterest() const
{
    return this->centerOfInterest;
}
inline void
MayaCameraUtil::SetZoomIn(float v)
{
    this->zoomIn = v;
}
inline void
MayaCameraUtil::SetZoomOut(float v)
{
    this->zoomOut = v;
}
inline void
MayaCameraUtil::SetPanning(const Math::vec2& v)
{
    this->panning = v;
}
inline void
MayaCameraUtil::SetOrbiting(const Math::vec2& v)
{
    this->orbiting = v;
}
inline void
MayaCameraUtil::SetCenterOfInterest(const Math::point& point)
{
    this->centerOfInterest = point;
}
inline void
MayaCameraUtil::SetViewDistance(const Math::scalar& distance)
{
    this->viewDistance = distance;
}
}
#endif