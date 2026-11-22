#pragma once
#include "graphics/graphicscontext.h"
#include "coregraphics/shader.h"
#include "coregraphics/buffer.h"
#include "coregraphics/texture.h"
#include <array>
#include <system_shaders/lights_cluster.h>
namespace Frame
{
class FrameScript;
};
namespace Lighting
{
class LightContext : public Graphics::GraphicsContext
{
    __DeclareContext();
public:
    enum class LightType
    {
        DirectionalLightType,
        PointLightType,
        SpotLightType,
        AreaLightType
    };
    enum class AreaLightShape
    {
        Disk,
        Rectangle,
        Tube
    };
    LightContext();
    virtual ~LightContext();
    static void Create();
    static void Discard();
    static void SetupGlobalLight(const Graphics::GraphicsEntityId id, const Math::vec3& color, const float intensity, const Math::vec3& ambient, const float zenith, const float azimuth, bool castShadows = false);
    static void SetupPointLight(
        const Graphics::GraphicsEntityId id
        , const Math::vec3& color
        , const float intensity
        , const float range
        , bool castShadows = false
        , const CoreGraphics::TextureId projection = CoreGraphics::InvalidTextureId
    );
    static void SetupSpotLight(
        const Graphics::GraphicsEntityId id
        , const Math::vec3& color
        , const float intensity
        , const float innerConeAngle
        , const float outerConeAngle
        , const float range
        , bool castShadows = false
        , const CoreGraphics::TextureId projection = CoreGraphics::InvalidTextureId
    );
    static void SetupAreaLight(
        const Graphics::GraphicsEntityId id
        , const AreaLightShape shape
        , const Math::vec3& color
        , const float intensity
        , const float range
        , bool twoSided = false
        , bool castShadows = false
        , bool renderMesh = false
    );
    static void SetColor(const Graphics::GraphicsEntityId id, const Math::vec3& color);
    static Math::vec3 GetColor(const Graphics::GraphicsEntityId id);
    static void SetRange(const Graphics::GraphicsEntityId id, const float range);
    static void SetIntensity(const Graphics::GraphicsEntityId id, const float intensity);
    static float GetIntensity(const Graphics::GraphicsEntityId id);
    static void SetTransform(const Graphics::GraphicsEntityId id, const float azimuth, const float zenith);
    static const Math::mat4 GetTransform(const Graphics::GraphicsEntityId id);
    static const Math::mat4 GetObserverTransform(const Graphics::GraphicsEntityId id);
    static const void SetPosition(const Graphics::GraphicsEntityId id, const Math::point& position);
    static const Math::point GetPosition(const Graphics::GraphicsEntityId id);
    static const void SetRotation(const Graphics::GraphicsEntityId id, const Math::quat& rotation);
    static const Math::quat GetRotation(const Graphics::GraphicsEntityId id);
    static const void SetScale(const Graphics::GraphicsEntityId id, const Math::vec3& scale);
    static const Math::vec3 GetScale(const Graphics::GraphicsEntityId id);
    static Math::vec3 GetAmbient(const Graphics::GraphicsEntityId id);
    static void SetAmbient(const Graphics::GraphicsEntityId id, Math::vec3& ambient);
    static LightType GetType(const Graphics::GraphicsEntityId id);
    static void GetInnerOuterAngle(const Graphics::GraphicsEntityId id, float& inner, float& outer);
    static void SetInnerOuterAngle(const Graphics::GraphicsEntityId id, float inner, float outer);
    static void OnPrepareView(const Ptr<Graphics::View>& view, const Graphics::FrameContext& ctx);
    static void UpdateViewDependentResources(const Ptr<Graphics::View>& view, const Graphics::FrameContext& ctx);
    static void RunFrameScriptJobs(const Graphics::FrameContext& ctx);
    static void WindowResized(const CoreGraphics::WindowId windowId, SizeT width, SizeT height);
#ifndef PUBLIC_BUILD
    static void OnRenderDebug(uint32_t flags);
#endif
    static void SetupTerrainShadows(const CoreGraphics::TextureId terrainShadowMap, const uint worldSize);
    static const CoreGraphics::BufferId GetLightIndexBuffer();
    static const CoreGraphics::BufferId GetLightsBuffer();
    static const LightsCluster::LightUniforms& GetLightUniforms();
private:
    static void SetGlobalLightTransform(const Graphics::ContextEntityId id, const Math::mat4& transform, const Math::vector& direction);
    static void SetGlobalLightViewProjTransform(const Graphics::ContextEntityId id, const Math::mat4& transform);
    enum
    {
        Type,
        Color,
        Intensity,
        ShadowCaster,
        Range,
        TypedLightId
    };
    typedef Ids::IdAllocator<
        LightType,
        Math::vec3,
        float,
        bool,
        float,
        Ids::Id32
    > GenericLightAllocator;
    static GenericLightAllocator genericLightAllocator;
    struct ConstantBufferSet
    {
        uint offset, slice;
    };
    enum
    {
        PointLight_Transform,
        PointLight_ConstantBufferSet,
        PointLight_ShadowConstantBufferSet,
        PointLight_DynamicOffsets,
        PointLight_ProjectionTexture,
        PointLight_Observer
    };
    typedef Ids::IdAllocator<
        Math::transform44,
        ConstantBufferSet,
        ConstantBufferSet,
        Util::FixedArray<uint>,
        CoreGraphics::TextureId,
        Graphics::GraphicsEntityId
    > PointLightAllocator;
    static PointLightAllocator pointLightAllocator;
    enum
    {
        SpotLight_Transform,
        SpotLight_ConstantBufferSet,
        SpotLight_ShadowConstantBufferSet,
        SpotLight_DynamicOffsets,
        SpotLight_ConeAngles,
        SpotLight_ProjectionTexture,
        SpotLight_ProjectionTransform,
        SpotLight_Observer
    };
    typedef Ids::IdAllocator<
        Math::transform44,
        ConstantBufferSet,
        ConstantBufferSet,
        Util::FixedArray<uint>,
        std::array<float, 2>,
        CoreGraphics::TextureId,
        Math::mat4,
        Graphics::GraphicsEntityId
    > SpotLightAllocator;
    static SpotLightAllocator spotLightAllocator;
    enum
    {
        AreaLight_Transform,
        AreaLight_Shape,
        AreaLight_ConstantBufferSet,
        AreaLight_ShadowConstantBufferSet,
        AreaLight_DynamicOffsets,
        AreaLight_TwoSided,
        AreaLight_Observer,
        AreaLight_RenderMesh,
    };
    typedef Ids::IdAllocator<
        Math::transform44,
        AreaLightShape,
        ConstantBufferSet,
        ConstantBufferSet,
        Util::FixedArray<uint>,
        bool,
        Graphics::GraphicsEntityId,
        bool
    > AreaLightAllocator;
    static AreaLightAllocator areaLightAllocator;
    enum
    {
        DirectionalLight_Direction,
        DirectionalLight_Ambient,
        DirectionalLight_Transform,
        DirectionalLight_ViewProjTransform,
        DirectionalLight_CascadeObservers
    };
    typedef Ids::IdAllocator<
        Math::vector,
        Math::vec3,
        Math::mat4,
        Math::mat4,
        Util::Array<Graphics::GraphicsEntityId>
    > DirectionalLightAllocator;
    static DirectionalLightAllocator directionalLightAllocator;
    enum
    {
        ShadowCaster_Transform
    };
    typedef Ids::IdAllocator<
        Math::mat4
    > ShadowCasterAllocator;
    static ShadowCasterAllocator shadowCasterAllocator;
    static Util::HashTable<Graphics::GraphicsEntityId, uint, 16, 1> shadowCasterSliceMap;
    static Graphics::ContextEntityId Alloc();
    static void Dealloc(Graphics::ContextEntityId id);
};
}