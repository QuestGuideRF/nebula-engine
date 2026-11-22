#pragma once
#include "graphics/graphicscontext.h"
#include "resources/resource.h"
#include "ids/idallocator.h"
namespace Vegetation
{
ID_32_TYPE(VegetationBiomeId);
struct VegetationSetupSettings
{
    float minHeight, maxHeight;
    Math::vec2 worldSize;
};
struct VegetationGrassSetup
{
    Resources::ResourceName mask;
    Resources::ResourceName albedo;
    Resources::ResourceName normals;
    Resources::ResourceName material;
    float slopeThreshold;
    float heightThreshold;
};
struct VegetationMeshSetup
{
    Resources::ResourceName mask;
    Resources::ResourceName albedo;
    Resources::ResourceName normals;
    Resources::ResourceName material;
    Resources::ResourceName mesh;
    float slopeThreshold;
    float heightThreshold;
};
class VegetationContext : public Graphics::GraphicsContext
{
    __DeclareContext();
public:
    VegetationContext();
    ~VegetationContext();
    static void Create(const VegetationSetupSettings& settings);
    static void Discard();
    static void Setup(Resources::ResourceName heightMap, SizeT numGrassPlanesPerTuft, float grassPatchRadius);
    static void SetupGrass(const Graphics::GraphicsEntityId id, const VegetationGrassSetup& setup);
    static void SetupMesh(const Graphics::GraphicsEntityId id, const VegetationMeshSetup& setup);
    static void UpdateViewResources(const Ptr<Graphics::View>& view, const Graphics::FrameContext& ctx);
private:
    static Graphics::ContextEntityId Alloc();
    static void Dealloc(Graphics::ContextEntityId id);
    enum class VegetationType : uint8_t
    {
        GrassType,
        MeshType
    };
    enum
    {
        Vegetation_Mask,
        Vegetation_Albedos,
        Vegetation_Normals,
        Vegetation_Materials,
        Vegetation_Meshes,
        Vegetation_GPUInfo,
        Vegetation_TextureIndex,
        Vegetation_SlopeThreshold,
        Vegetation_HeightThreshold,
        Vegetation_Type
    };
    typedef Ids::IdAllocator<
        CoreGraphics::TextureId,
        CoreGraphics::TextureId,
        CoreGraphics::TextureId,
        CoreGraphics::TextureId,
        CoreGraphics::MeshId,
        uint,
        IndexT,
        float, float,
        VegetationType
    > VegetationAllocator;
    static VegetationAllocator vegetationAllocator;
};
}