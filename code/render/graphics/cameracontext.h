#pragma once
#include "graphics/graphicscontext.h"
#include "camerasettings.h"
#include "math/scalar.h"
namespace Graphics
{
class CameraContext : public Graphics::GraphicsContext
{
    __DeclareContext();
public:
    CameraContext();
    virtual ~CameraContext();
    static void Create();
    static void UpdateCameras(const Graphics::FrameContext& ctx);
    static void SetupProjectionFov(const Graphics::GraphicsEntityId id, float aspect, float fov, float znear, float zfar);
    static void SetupOrthographic(const Graphics::GraphicsEntityId id, float width, float height, float znear, float zfar);
    static void SetView(const Graphics::GraphicsEntityId id, const Math::mat4& mat);
    static const Math::mat4& GetView(const Graphics::GraphicsEntityId id);
    static const Math::mat4 GetTransform(const Graphics::GraphicsEntityId id);
    static const Math::mat4& GetProjection(const Graphics::GraphicsEntityId id);
    static const Math::mat4& GetViewProjection(const Graphics::GraphicsEntityId id);
    static const CameraSettings& GetSettings(const Graphics::GraphicsEntityId id);
    static void OnWindowResized(const CoreGraphics::WindowId windowId, SizeT width, SizeT height);
    static Graphics::GraphicsEntityId GetLODCamera();
    static void SetLODCamera(const Graphics::GraphicsEntityId id);
private:
    enum
    {
        Camera_Settings,
        Camera_Projection,
        Camera_View,
        Camera_ViewProjection
    };
    typedef Ids::IdAllocator<
        Graphics::CameraSettings,
        Math::mat4,
        Math::mat4,
        Math::mat4
    > CameraAllocator;
    static CameraAllocator cameraAllocator;
    static Graphics::GraphicsEntityId lodCamera;
    static Graphics::ContextEntityId Alloc();
    static void Dealloc(const Graphics::ContextEntityId id);
};
inline Graphics::ContextEntityId
CameraContext::Alloc()
{
    return cameraAllocator.Alloc();
}
inline void
CameraContext::Dealloc(const Graphics::ContextEntityId id)
{
    cameraAllocator.Dealloc(id.id);
}
}