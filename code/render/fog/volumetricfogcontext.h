#pragma once
#include "graphics/graphicscontext.h"
namespace Frame
{
class FrameScript;
};
namespace Fog
{
class VolumetricFogContext : public Graphics::GraphicsContext
{
    __DeclareContext()
public:
    VolumetricFogContext();
    virtual ~VolumetricFogContext();
    static void Create();
    static void Discard();
    static void SetupBoxVolume(
        const Graphics::GraphicsEntityId id,
        const Math::mat4& transform,
        const float density,
        const Math::vec3& absorption);
    static void SetBoxTransform(const Graphics::GraphicsEntityId id, const Math::mat4& transform);
    static void SetupSphereVolume(
        const Graphics::GraphicsEntityId id,
        const Math::vec3& position,
        float radius,
        const float density,
        const Math::vec3& absorption);
    static void SetSpherePosition(const Graphics::GraphicsEntityId id, const Math::vec3& position);
    static void SetSphereRadius(const Graphics::GraphicsEntityId id, const float radius);
    static void SetTurbidity(const Graphics::GraphicsEntityId id, const float turbidity);
    static void SetAbsorption(const Graphics::GraphicsEntityId id, const Math::vec3& absorption);
    static void UpdateViewDependentResources(const Ptr<Graphics::View>& view, const Graphics::FrameContext& ctx);
    static void RenderUI(const Graphics::FrameContext& ctx);
    void SetGlobalTurbidity(float f);
    void SetGlobalAbsorption(const Math::vec3& color);
    static Math::mat4 GetTransform(const Graphics::GraphicsEntityId id);
    static void SetTransform(const Graphics::GraphicsEntityId id, const Math::mat4& mat);
private:
    enum FogVolumeType
    {
        BoxVolume,
        SphereVolume
    };
    enum
    {
        FogVolume_Type,
        FogVolume_TypedId,
        FogVolume_Turbidity,
        FogVolume_Absorption,
    };
    typedef Ids::IdAllocator<
        FogVolumeType,
        Ids::Id32,
        float,
        Math::vec3
    > FogGenericVolumeAllocator;
    static FogGenericVolumeAllocator fogGenericVolumeAllocator;
    enum
    {
        FogBoxVolume_Transform,
    };
    typedef Ids::IdAllocator<
        Math::mat4
    > FogBoxVolumeAllocator;
    static FogBoxVolumeAllocator fogBoxVolumeAllocator;
    enum
    {
        FogSphereVolume_Position,
        FogSphereVolume_Radius,
    };
    typedef Ids::IdAllocator<
        Math::vec3,
        float
    > FogSphereVolumeAllocator;
    static FogSphereVolumeAllocator fogSphereVolumeAllocator;
    static Graphics::ContextEntityId Alloc();
    static void Dealloc(Graphics::ContextEntityId id);
};
}