#pragma once
#include "math/mat4.h"
#include "math/vec2.h"
#include "math/frustum.h"
namespace Graphics
{
class CameraSettings
{
public:
    CameraSettings();
    void SetupPerspectiveFov(float fov, float aspect, float zNear, float zFar);
    void SetupOrthogonal(float w, float h, float zNear, float zFar);
    const Math::mat4& GetProjTransform() const;
    const Math::mat4& GetInvProjTransform() const;
    const Math::vec2& GetFocalLength() const;
    bool IsPerspective() const;
    bool IsOrthogonal() const;
    float GetZNear() const;
    float GetZFar() const;
    float GetFov() const;
    float GetAspect() const;
    float GetNearWidth() const;
    float GetNearHeight() const;
    float GetFarWidth() const;
    float GetFarHeight() const;
private:
    Math::mat4 projMatrix;
    Math::mat4 invProjMatrix;
    bool isPersp;
    float zNear;
    float zFar;
    float fov;
    float aspect;
    float nearWidth;
    float nearHeight;
    float farWidth;
    float farHeight;
    Math::vec2 focalLength;
};
inline const Math::mat4&
CameraSettings::GetProjTransform() const
{
    return this->projMatrix;
}
inline const Math::mat4&
CameraSettings::GetInvProjTransform() const
{
    return this->invProjMatrix;
}
inline bool
CameraSettings::IsPerspective() const
{
    return this->isPersp;
}
inline bool
CameraSettings::IsOrthogonal() const
{
    return !this->isPersp;
}
inline float
CameraSettings::GetZNear() const
{
    return this->zNear;
}
inline float
CameraSettings::GetZFar() const
{
    return this->zFar;
}
inline float
CameraSettings::GetFov() const
{
    return this->fov;
}
inline float
CameraSettings::GetAspect() const
{
    return this->aspect;
}
inline float
CameraSettings::GetNearWidth() const
{
    return this->nearWidth;
}
inline float
CameraSettings::GetNearHeight() const
{
    return this->nearHeight;
}
inline float
CameraSettings::GetFarWidth() const
{
    return this->farWidth;
}
inline float
CameraSettings::GetFarHeight() const
{
    return this->farHeight;
}
inline const Math::vec2&
CameraSettings::GetFocalLength() const
{
    return this->focalLength;
}
}