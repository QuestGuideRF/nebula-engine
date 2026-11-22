#pragma once
#include "graphics/graphicscontext.h"
namespace Decals
{
class DecalContext : public Graphics::GraphicsContext
{
    __DeclareContext();
public:
    DecalContext();
    virtual ~DecalContext();
    static void Create();
    static void Discard();
    static void SetupDecalPBR(
        const Graphics::GraphicsEntityId id,
        const Math::mat4 transform,
        const Resources::ResourceId albedo,
        const Resources::ResourceId normal,
        const Resources::ResourceId material);
    static void SetupDecalEmissive(
        const Graphics::GraphicsEntityId id,
        const Math::mat4 transform,
        const Resources::ResourceId emissive);
    static void SetAlbedoTexture(const Graphics::GraphicsEntityId id, const Resources::ResourceId albedo);
    static void SetNormalTexture(const Graphics::GraphicsEntityId id, const Resources::ResourceId normal);
    static void SetMaterialTexture(const Graphics::GraphicsEntityId id, const Resources::ResourceId material);
    static void SetEmissiveTexture(const Graphics::GraphicsEntityId id, const Resources::ResourceId emissive);
    static void SetTransform(const Graphics::GraphicsEntityId id, const Math::mat4 transform);
    static Math::mat4 GetTransform(const Graphics::GraphicsEntityId id);
    static void UpdateViewDependentResources(const Ptr<Graphics::View>& view, const Graphics::FrameContext& ctx);
#ifndef PUBLIC_BUILD
    static void OnRenderDebug(uint32_t flags);
#endif
private:
    enum DecalType
    {
        PBRDecal,
        EmissiveDecal
    };
    enum
    {
        Decal_Transform,
        Decal_Type,
        Decal_TypedId
    };
    typedef Ids::IdAllocator<
        Math::mat4,
        DecalType,
        Ids::Id32
    > GenericDecalAllocator;
    static GenericDecalAllocator genericDecalAllocator;
    enum
    {
        DecalPBR_Albedo,
        DecalPBR_Normal,
        DecalPBR_Material,
    };
    typedef Ids::IdAllocator<
        Resources::ResourceId,
        Resources::ResourceId,
        Resources::ResourceId
    > PBRDecalAllocator;
    static PBRDecalAllocator pbrDecalAllocator;
    enum
    {
        DecalEmissive_Emissive,
    };
    typedef Ids::IdAllocator<
        Resources::ResourceId
    > EmissiveDecalAllocator;
    static EmissiveDecalAllocator emissiveDecalAllocator;
    static Graphics::ContextEntityId Alloc();
    static void Dealloc(Graphics::ContextEntityId id);
};
}