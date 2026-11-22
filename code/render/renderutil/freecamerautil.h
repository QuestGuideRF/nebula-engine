#pragma once
#include "math/mat4.h"
#include "math/vec2.h"
#include "math/polar.h"
namespace RenderUtil
{
class FreeCameraUtil
{
public:
    FreeCameraUtil();
    void Setup(const Math::point& defaultEyePos, const Math::vector& defaultEyeVec);
    void Reset();
    void Update(float deltaTime);
    const Math::mat4& GetTransform() const;
    void SetTargetPosition(Math::vec3 const& point);
    void SetRotateButton(bool state);
    void SetAccelerateButton(bool state);
    void SetMouseMovement(Math::vec2 movement);
    void SetRotationSpeed(float speed);
    void SetMovementSpeed(float speed);
    void SetForwardsKey(bool state);
    void SetBackwardsKey(bool state);
    void SetLeftStrafeKey(bool state);
    void SetRightStrafeKey(bool state);
    void SetUpKey(bool state);
    void SetDownKey(bool state);
private:
    Math::point defaultEyePos;
    Math::vector defaultEyeVec;
    Math::vec2 mouseMovement;
    Math::polar viewAngles;
    Math::point position;
    Math::point targetPosition;
    Math::mat4 cameraTransform;
    float rotationSpeed;
    float moveSpeed;
    bool rotateButton;
    bool accelerateButton;
    bool forwardsKey;
    bool backwardsKey;
    bool leftStrafeKey;
    bool rightStrafeKey;
    bool upKey;
    bool downKey;
};
inline const Math::mat4&
FreeCameraUtil::GetTransform() const
{
    return this->cameraTransform;
}
inline void
FreeCameraUtil::SetTargetPosition(Math::vec3 const& point)
{
    this->targetPosition = point;
}
inline void
FreeCameraUtil::SetRotateButton( bool state )
{
    this->rotateButton = state;
}
inline void
FreeCameraUtil::SetAccelerateButton( bool state )
{
    this->accelerateButton = state;
}
inline void
FreeCameraUtil::SetMouseMovement( Math::vec2 movement )
{
    this->mouseMovement = movement;
}
inline void
FreeCameraUtil::SetRotationSpeed( float speed )
{
    this->rotationSpeed = speed;
}
inline void
FreeCameraUtil::SetMovementSpeed( float speed )
{
    this->moveSpeed = speed;
}
inline void
FreeCameraUtil::SetForwardsKey( bool state )
{
    this->forwardsKey = state;
}
inline void
FreeCameraUtil::SetBackwardsKey( bool state )
{
    this->backwardsKey = state;
}
inline void
FreeCameraUtil::SetLeftStrafeKey( bool state )
{
    this->leftStrafeKey = state;
}
inline void
FreeCameraUtil::SetRightStrafeKey( bool state )
{
    this->rightStrafeKey = state;
}
inline void
FreeCameraUtil::SetUpKey( bool state )
{
    this->upKey = state;
}
inline void
FreeCameraUtil::SetDownKey( bool state )
{
    this->downKey = state;
}
}