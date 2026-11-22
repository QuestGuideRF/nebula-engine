#pragma once
#include "core/refcounted.h"
#include "math/frustum.h"
#include "math/mat4.h"
#include "graphics/graphicsentity.h"
#ifndef FLT_MAX
#define FLT_MAX 3.402823466e+38F
#endif
#ifndef FLT_MIN
#define FLT_MIN 1.175494351e-38F
#endif
namespace Lighting
{
class CSMUtil
{
public:
    CSMUtil(const uint numCascades);
    virtual ~CSMUtil();
    void SetCameraEntity(const Graphics::GraphicsEntityId camera);
    const Graphics::GraphicsEntityId GetCameraEntity() const;
    void SetShadowBox(const Math::bbox& sceneBox);
    void SetGlobalLight(const Graphics::GraphicsEntityId globalLight);
    const Graphics::GraphicsEntityId GetGlobalLight() const;
    void SetTextureWidth(int width);
    void SetBlurSize(int size);
    void SetFitTexels(bool state);
    const Math::mat4& GetCascadeViewProjection(IndexT cascadeIndex) const;
    const Math::mat4& GetShadowView() const;
    const Util::FixedArray<Math::mat4>& GetCascadeProjectionTransforms() const;
    const Util::FixedArray<float>& GetCascadeDistances() const;
    const Math::mat4& GetCascadeCamera(IndexT index)  const;
    void Compute(const Graphics::GraphicsEntityId camera, const Graphics::GraphicsEntityId light);
private:
    struct CascadeFrustum
    {
        float rightSlope;
        float leftSlope;
        float topSlope;
        float bottomSlope;
        float nearPlane, farPlane;
    };
    Math::bbox shadowBox;
    Math::vec4 frustumCenter;
    Graphics::GraphicsEntityId globalLight;
    Graphics::GraphicsEntityId cameraEntity;
    uint numCascades;
    Util::FixedArray<Math::mat4> cascadeProjectionTransform, cascadeViewProjectionTransform;
    Util::FixedArray<float> cascadeDistances, intervalDistances;
    Math::mat4 shadowView;
    float cascadeMaxDistance;
};
inline void
CSMUtil::SetShadowBox( const Math::bbox& shadowRange )
{
    this->shadowBox = shadowRange;
}
inline void
CSMUtil::SetCameraEntity( const Graphics::GraphicsEntityId camera )
{
    this->cameraEntity = camera;
}
inline const Graphics::GraphicsEntityId
CSMUtil::GetCameraEntity() const
{
    return this->cameraEntity;
}
inline void
CSMUtil::SetGlobalLight( const Graphics::GraphicsEntityId globalLight )
{
    this->globalLight = globalLight;
}
inline const Graphics::GraphicsEntityId
CSMUtil::GetGlobalLight() const
{
    return this->globalLight;
}
inline const Math::mat4&
CSMUtil::GetCascadeViewProjection( IndexT cascadeIndex ) const
{
    return this->cascadeViewProjectionTransform[cascadeIndex];
}
inline const Math::mat4&
CSMUtil::GetShadowView() const
{
    return this->shadowView;
}
inline const Util::FixedArray<Math::mat4>&
CSMUtil::GetCascadeProjectionTransforms() const
{
    return this->cascadeProjectionTransform;
}
inline const Util::FixedArray<float>&
CSMUtil::GetCascadeDistances() const
{
    return this->intervalDistances;
}
}