//------------------------------------------------------------------------------
//  terraincontext.cc
//  (C) 2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "terraincontext.h"
#include "graphics/graphicsserver.h"
#include "frame/framesubgraph.h"
#include "graphics/cameracontext.h"
#include "graphics/view.h"
#include "dynui/imguicontext.h"
#include "jobs2/jobs2.h"
#include "imgui.h"
#include "renderutil/drawfullscreenquad.h"
#include "core/cvar.h"

#include "lighting/lightcontext.h"
#include "raytracing/raytracingcontext.h"

#include "occupancyquadtree.h"
#include "texturetilecache.h"

#include "resources/resourceserver.h"
#include "materials/materialloader.h"

#include "terrain/shaders/terrain.h"
#include "terrain/shaders/terrain_mesh_generate.h"
#include "terrain/shaders/terrain_tile_write.h"

#include "frame/default.h"

N_DECLARE_COUNTER(N_TERRAIN_TOTAL_AVAILABLE_DATA, Terrain Total Data Size);

namespace Terrain
{
TerrainContext::TerrainAllocator TerrainContext::terrainAllocator;
TerrainContext::TerrainBiomeAllocator TerrainContext::terrainBiomeAllocator;

Threading::Event subtexturesFinishedEvent, sectionCullFinishedEvent;
Threading::AtomicCounter subtexturesDoneCounter = 0, sectionCullDoneCounter = 0;

const uint IndirectionTextureSize = 2048;
const uint SubTextureWorldSize = 64;
const uint SubTextureMaxTiles = 256;
const uint IndirectionNumMips = Math::log2(SubTextureMaxTiles) + 1;
const float SubTextureRange = 300.0f; // Дальность 300 метров
const float SubTextureSwapDistance = 10.0f;
const float SubTextureFadeStart = 200.0f;
const uint SubTextureMaxPixels = 65536;
const uint SubTextureMaxUpdates = 1024;

const uint LowresFallbackSize = 4096;
const uint LowresFallbackMips = Math::log2(LowresFallbackSize) + 1;

const uint PhysicalTextureTileSize = 256;
const uint PhysicalTextureTileHalfPadding = 4;
const uint PhysicalTextureTilePadding = PhysicalTextureTileHalfPadding * 2;
const uint PhysicalTextureNumTiles = 32;
const uint PhysicalTextureTilePaddedSize = PhysicalTextureTileSize + PhysicalTextureTilePadding;
const uint PhysicalTextureSize = (PhysicalTextureTileSize) * PhysicalTextureNumTiles;
const uint PhysicalTexturePaddedSize = (PhysicalTextureTilePaddedSize) * PhysicalTextureNumTiles;

const uint TerrainShadowMapSize = 2048;


__ImplementContext(TerrainContext, TerrainContext::terrainAllocator);

struct
{
    TerrainSetupSettings settings;
    CoreGraphics::ShaderId terrainShader;
    CoreGraphics::ShaderId tileShader;
    CoreGraphics::ShaderProgramId terrainShadowProgram;
    CoreGraphics::ResourceTableId resourceTable;
    CoreGraphics::BufferId systemConstants;
    CoreGraphics::VertexLayoutId vlo;

    CoreGraphics::WindowId wnd;

    CoreGraphics::BufferId biomeBuffer;
    Terrain::MaterialLayers biomeMaterials;
    Util::Array<CoreGraphics::TextureId> biomeTextures;
    CoreGraphics::TextureId biomeMasks[Terrain::MAX_BIOMES];
    CoreGraphics::TextureId biomeWeights[Terrain::MAX_BIOMES];
    Threading::AtomicCounter biomeLoaded[Terrain::MAX_BIOMES][4];
    uint biomeLowresGenerated[Terrain::MAX_BIOMES];
    IndexT biomeCounter;

    Graphics::GraphicsEntityId sun;

    CoreGraphics::TextureId terrainShadowMap;
    Math::vec4 cachedSunDirection;
    bool shadowMapInvalid;
    bool updateShadowMap;

    Threading::CriticalSection syncPoint;
    CoreGraphics::ResourceTableId terrainShadowResourceTable;

    CoreGraphics::TextureId terrainPosBuffer;
    
    float mipLoadDistance = 1500.0f;
    float mipRenderPadding = 150.0f;
    SizeT layers;

    bool debugRender;
    bool renderToggle;

} terrainState;

struct PhysicalTileUpdate
{
    uint constantBufferOffsets[2];
    uint tileOffset[2];
};

struct SubTexture
{
    Math::float2 worldCoordinate;
    Math::uint2 indirectionOffset;
    uint maxMip;
    uint mipBias;
    uint numTiles;
};

struct SubTextureCompressed
{
    Math::float2 worldCoordinate;
    uint32_t : 32;
    uint32_t indirectionX : 12;
    uint32_t indirectionY : 12;
    uint mip : 8;
};

struct
{
    CoreGraphics::ShaderProgramId                                   terrainPrepassProgram;
    CoreGraphics::PipelineId                                        terrainPrepassPipeline;
    CoreGraphics::ShaderProgramId                                   terrainPageClearUpdateBufferProgram;
    CoreGraphics::ShaderProgramId                                   terrainResolveProgram;
    CoreGraphics::PipelineId                                        terrainResolvePipeline;
    CoreGraphics::ShaderProgramId                                   terrainTileWriteProgram;
    CoreGraphics::ShaderProgramId                                   terrainWriteLowresProgram;

    bool                                                            virtualSubtextureBufferUpdate;
    CoreGraphics::BufferSet                                         subtextureStagingBuffers;
    CoreGraphics::BufferId                                          subTextureBuffer;

    CoreGraphics::BufferSet                                         pageUpdateReadbackBuffers;
    CoreGraphics::TextureId                                         indirectionTexture;
    CoreGraphics::TextureId                                         indirectionTextureCopy;

    CoreGraphics::BufferId                                          pageUpdateListBuffer;
    CoreGraphics::BufferId                                          pageStatusBuffer;

    Threading::AtomicCounter                                        numPatchesThisFrame;
    CoreGraphics::BufferSet                                         patchConstants;
    CoreGraphics::BufferId                                          runtimeConstants;
    CoreGraphics::BufferWithStaging                                 tileWriteBufferSet;

    CoreGraphics::TextureId                                         physicalAlbedoCacheBC;
    CoreGraphics::TextureViewId                                     physicalAlbedoCacheBCWrite;
    CoreGraphics::TextureId                                         physicalNormalCacheBC;
    CoreGraphics::TextureViewId                                     physicalNormalCacheBCWrite;
    CoreGraphics::TextureId                                         physicalMaterialCacheBC;
    CoreGraphics::TextureViewId                                     physicalMaterialCacheBCWrite;
    CoreGraphics::TextureId                                         lowresAlbedo;
    CoreGraphics::TextureId                                         lowresNormal;
    CoreGraphics::TextureId                                         lowresMaterial;
    bool                                                            updateLowres = false;

    CoreGraphics::ResourceTableSet                                  systemTable;
    CoreGraphics::ResourceTableId                                   runtimeTable;
    SizeT                                                           numPageBufferUpdateEntries;

    Util::FixedArray<SubTexture>                                    subTextures;
    Util::FixedArray<TerrainSubTexture>                             gpuSubTextures;
    Threading::AtomicCounter                                        subTextureNumOutputs;
    Terrain::SubTextureUpdateJobOutput                              subTextureJobOutputs[SubTextureMaxUpdates];
    OccupancyQuadTree                                               indirectionOccupancy;
    OccupancyQuadTree                                               physicalTextureTileOccupancy;
    TextureTileCache                                                physicalTextureTileCache;

    Util::FixedArray<uint>                                          indirectionMipOffsets;
    Util::FixedArray<uint>                                          indirectionMipSizes;
    Util::Array<CoreGraphics::TextureCopy>                          indirectionTextureCopies;
    Util::Array<IndirectionEntry>                                   indirectionEntryUpdates;
    Util::FixedArray<IndirectionEntry>                              indirectionBuffer;
    CoreGraphics::BufferSet                                         indirectionUploadBuffers;
    Util::FixedArray<uint>                                          indirectionUploadOffsets;

    Util::Array<TerrainTileWrite::TileWrite>                                 tileWrites;
    Util::Array<TerrainTileWrite::TileWrite>                                 tileWritesThisFrame;

    Util::Array<CoreGraphics::BufferCopy, 4>                        indirectionBufferUpdatesThisFrame;
    Util::Array<CoreGraphics::TextureCopy, 4>                       indirectionTextureUpdatesThisFrame;
    Util::Array<CoreGraphics::TextureCopy, 4>                       indirectionTextureFromCopiesThisFrame;
    Util::Array<CoreGraphics::TextureCopy, 4>                       indirectionTextureToCopiesThisFrame;
    Util::Array<CoreGraphics::BufferCopy, 4>                        indirectionBufferClearsThisFrame;
    Util::Array<CoreGraphics::TextureCopy, 4>                       indirectionTextureClearsThisFrame;
    uint numPixels;

} terrainVirtualTileState;

struct
{
    CoreGraphics::ShaderProgramId meshProgram;
    CoreGraphics::VertexAlloc vertexBuffer, indexBuffer;
    CoreGraphics::ResourceTableId meshGenTable;
    CoreGraphics::BufferId patchBuffer, constantsBuffer;
    SizeT numTris, numPatches;
    bool updateMesh = false;
    std::function<void()> terrainSetupCallback;
    IndexT setupBlasFrame;
} raytracingState;

struct TerrainVert
{
    Math::float3 position;
    short uv[2];
};

struct TerrainTri
{
    IndexT a, b, c;
};

struct TerrainQuad
{
    IndexT a, b, c, d;
};

enum BiomeLoadBits
{
    AlbedoLoaded = 0x1
    , NormalLoaded = 0x2
    , MaterialLoaded = 0x4
    , MaskLoaded = 0x8
    , WeightsLoaded = 0x10
};

//------------------------------------------------------------------------------
/**
*/
void
PackSubTexture(const SubTexture& subTex, TerrainSubTexture& compressed)
{
    // Первые 12 бит смещены по x,
    // следующие 12 бит смещены по y,
    // затем 4 бита для mips
    // наконец, 4 бита для смещения MIP
    compressed.packed0 = (subTex.indirectionOffset.x & 0xFFF)
        | ((subTex.indirectionOffset.y & 0xFFF) << 12)
        | ((subTex.maxMip & 0xF) << 24)
        | ((subTex.mipBias & 0xF) << 28)
        ;
}

//------------------------------------------------------------------------------
/**
*/
TerrainContext::TerrainContext()
{
}

//------------------------------------------------------------------------------
/**
*/
TerrainContext::~TerrainContext()
{
}

//------------------------------------------------------------------------------
/**
*/
void 
TerrainContext::Create(const TerrainSetupSettings& settings)
{
    __CreateContext();
    using namespace CoreGraphics;

    Core::CVarCreate(Core::CVarType::CVar_Int, "r_terrain_debug", "0", "Показать интерфейс отладки системы ландшафта [0,1]");

#ifndef PUBLIC_BUILD
    __bundle.OnRenderDebug = TerrainContext::OnRenderDebug;
#endif
    Graphics::GraphicsServer::Instance()->RegisterGraphicsContext(&__bundle, &__state);

    terrainState.settings = settings;
    terrainState.updateShadowMap = true;
    terrainState.cachedSunDirection = Math::vec4(0);

    // создать буфер вершин
    Util::Array<VertexComponent> vertexComponents = 
    {
        VertexComponent{ 0, VertexComponent::Format::Float3 },
        VertexComponent{ 1, VertexComponent::Format::Short2 },
    };
    terrainState.vlo = CreateVertexLayout({ .name = "Terrain"_atm, .comps = vertexComponents });

    terrainState.terrainShader = ShaderGet("shd:terrain/shaders/terrain.fxb");
    terrainState.tileShader = ShaderGet("shd:terrain/shaders/terrain_tile_write.fxb");

    terrainState.resourceTable = ShaderCreateResourceTable(terrainState.terrainShader, NEBULA_SYSTEM_GROUP);
    terrainState.terrainShadowProgram = ShaderGetProgram(terrainState.terrainShader, ShaderFeatureMask("TerrainShadows"));

    CoreGraphics::ShaderId meshGenShader = ShaderGet("shd:terrain/shaders/terrain_mesh_generate.fxb");
    raytracingState.meshProgram = ShaderGetProgram(meshGenShader, ShaderFeatureMask("Main"));
    raytracingState.meshGenTable = ShaderCreateResourceTable(meshGenShader, NEBULA_BATCH_GROUP, 1);

    CoreGraphics::BufferCreateInfo sysBufInfo;
    sysBufInfo.name = "VirtualSystemBuffer"_atm;
    sysBufInfo.size = 1;
    sysBufInfo.elementSize = sizeof(Terrain::TerrainSystemUniforms);
    sysBufInfo.mode = CoreGraphics::DeviceAndHost;
    sysBufInfo.usageFlags = CoreGraphics::ConstantBuffer;
    terrainState.systemConstants = CoreGraphics::CreateBuffer(sysBufInfo);

    CoreGraphics::BufferCreateInfo biomeBufferInfo;
    biomeBufferInfo.name = "BiomeBuffer"_atm;
    biomeBufferInfo.size = 1;
    biomeBufferInfo.elementSize = sizeof(Terrain::MaterialLayers);
    biomeBufferInfo.mode = CoreGraphics::DeviceAndHost;
    biomeBufferInfo.usageFlags = CoreGraphics::ConstantBuffer;
    terrainState.biomeBuffer = CoreGraphics::CreateBuffer(biomeBufferInfo);

    CoreGraphics::TextureCreateInfo shadowMapInfo;
    shadowMapInfo.name = "TerrainShadowMap"_atm;
    shadowMapInfo.width = TerrainShadowMapSize;
    shadowMapInfo.height = TerrainShadowMapSize;
    shadowMapInfo.format = CoreGraphics::PixelFormat::R16G16F;
    shadowMapInfo.usage = CoreGraphics::TextureUsage::ReadWriteTexture;
    terrainState.terrainShadowMap = CoreGraphics::CreateTexture(shadowMapInfo);

    Lighting::LightContext::SetupTerrainShadows(terrainState.terrainShadowMap, settings.worldSizeX);

    ResourceTableSetConstantBuffer(terrainState.resourceTable, { terrainState.biomeBuffer, Terrain::Table_System::MaterialLayers_SLOT, 0, NEBULA_WHOLE_BUFFER_SIZE, 0 });
    ResourceTableSetConstantBuffer(terrainState.resourceTable, { terrainState.systemConstants, Terrain::Table_System::TerrainSystemUniforms_SLOT, 0, NEBULA_WHOLE_BUFFER_SIZE, 0});
    ResourceTableSetRWTexture(terrainState.resourceTable, { terrainState.terrainShadowMap, Terrain::Table_System::TerrainShadowMap_SLOT, 0, CoreGraphics::InvalidSamplerId });
    ResourceTableCommitChanges(terrainState.resourceTable);

    //------------------------------------------------------------------------------
    /**
        Adaptive terrain virtual texturing setup

        Passes:
        1. Clear page update buffer.
        2. Rasterize geometry and update page buffer.
        3. Read back on CPU and generate terrain tile jobs
        4. Splat terrain tiles into to physical texture cache.
        5. Render full-screen pass to put terrain pixels on the screen.

        Why don't we just render the terrain directly and sample our input textures? 
        Well, it would require a lot of work for the terrain splatting shader, 
        especially if we want to have features like roads and procedural decals. 
        This is called an optimization. LOL. 
    */
    //------------------------------------------------------------------------------

    terrainVirtualTileState.terrainPageClearUpdateBufferProgram = ShaderGetProgram(terrainState.terrainShader, ShaderFeatureMask("TerrainPageClearUpdateBuffer"));
    terrainVirtualTileState.terrainPrepassProgram = ShaderGetProgram(terrainState.terrainShader, ShaderFeatureMask("TerrainPrepass"));
    terrainVirtualTileState.terrainResolveProgram = ShaderGetProgram(terrainState.terrainShader, ShaderFeatureMask("TerrainResolve"));
    terrainVirtualTileState.terrainTileWriteProgram = ShaderGetProgram(terrainState.tileShader, ShaderFeatureMask("TerrainTileWrite"));
    terrainVirtualTileState.terrainWriteLowresProgram = ShaderGetProgram(terrainState.tileShader, ShaderFeatureMask("TerrainLowresWrite"));

    terrainVirtualTileState.systemTable = ShaderCreateResourceTableSet(terrainState.terrainShader, NEBULA_SYSTEM_GROUP);
    terrainVirtualTileState.runtimeTable = ShaderCreateResourceTable(terrainState.terrainShader, NEBULA_BATCH_GROUP);

    CoreGraphics::BufferCreateInfo bufInfo;
    bufInfo.name = "VirtualRuntimeBuffer"_atm;
    bufInfo.size = 1;
    bufInfo.elementSize = sizeof(Terrain::TerrainRuntimeUniforms);
    bufInfo.mode = CoreGraphics::DeviceAndHost;
    bufInfo.usageFlags = CoreGraphics::ConstantBuffer;
    terrainVirtualTileState.runtimeConstants = CoreGraphics::CreateBuffer(bufInfo);

    bufInfo.name = "TerrainTileWrites"_atm;
    bufInfo.size = 1;
    bufInfo.elementSize = sizeof(TerrainTileWrite::TilesToWrite);
    bufInfo.usageFlags = CoreGraphics::ConstantBuffer;
    terrainVirtualTileState.tileWriteBufferSet = CoreGraphics::BufferWithStaging(bufInfo);

    uint subTextureCount = (terrainState.settings.worldSizeX / SubTextureWorldSize) * (terrainState.settings.worldSizeZ / SubTextureWorldSize);

    bufInfo.name = "SubTextureBuffer"_atm;
    bufInfo.size = subTextureCount;
    bufInfo.elementSize = sizeof(Terrain::TerrainSubTexture);
    bufInfo.mode = BufferAccessMode::DeviceLocal;
    bufInfo.usageFlags = CoreGraphics::ReadWriteBuffer | CoreGraphics::TransferBufferDestination;
    terrainVirtualTileState.subTextureBuffer = CoreGraphics::CreateBuffer(bufInfo);

    bufInfo.name = "SubTextureStagingBuffer"_atm;
    bufInfo.size = subTextureCount;
    bufInfo.elementSize = sizeof(Terrain::TerrainSubTexture);
    bufInfo.mode = BufferAccessMode::HostLocal;
    bufInfo.usageFlags = CoreGraphics::TransferBufferSource;
    terrainVirtualTileState.subtextureStagingBuffers = BufferSet(bufInfo);

    CoreGraphics::TextureCreateInfo texInfo;
    texInfo.name = "IndirectionTexture"_atm;
    texInfo.width = IndirectionTextureSize;
    texInfo.height = IndirectionTextureSize;
    texInfo.format = CoreGraphics::PixelFormat::R32;
    texInfo.usage = CoreGraphics::SampleTexture | CoreGraphics::TransferDestinationTexture | CoreGraphics::TransferSourceTexture;
    texInfo.mips = IndirectionNumMips;
    texInfo.clear = true;
    texInfo.clearColorU4 = Math::uint4{ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
    terrainVirtualTileState.indirectionTexture = CoreGraphics::CreateTexture(texInfo);

    texInfo.name = "IndirectionTextureCopy"_atm;
    terrainVirtualTileState.indirectionTextureCopy = CoreGraphics::CreateTexture(texInfo);

    uint offset = 0;

    // настройка переменных смещения и размера mip
    terrainVirtualTileState.indirectionMipOffsets.Resize(IndirectionNumMips);
    terrainVirtualTileState.indirectionMipSizes.Resize(IndirectionNumMips);
    for (uint i = 0; i < IndirectionNumMips; i++)
    {
        uint width = IndirectionTextureSize >> i;
        uint height = IndirectionTextureSize >> i;

        terrainVirtualTileState.indirectionMipSizes[i] = width;
        terrainVirtualTileState.indirectionMipOffsets[i] = offset;
        offset += width * height;
    }

    terrainVirtualTileState.indirectionBuffer.Resize(offset);
    terrainVirtualTileState.indirectionBuffer.Fill(IndirectionEntry{ 0xF, 0x3FFF, 0x3FFF });

    bufInfo.name = "TerrainUploadBuffer"_atm;
    bufInfo.elementSize = sizeof(IndirectionEntry);
    bufInfo.size = IndirectionTextureSize * IndirectionTextureSize;
    bufInfo.mode = BufferAccessMode::HostLocal;
    bufInfo.usageFlags = CoreGraphics::TransferBufferSource;
    bufInfo.data = nullptr;
    bufInfo.dataSize = 0;
    terrainVirtualTileState.indirectionUploadBuffers = bufInfo;
    terrainVirtualTileState.indirectionUploadOffsets.Resize(CoreGraphics::GetNumBufferedFrames());
    terrainVirtualTileState.indirectionUploadOffsets.Fill(0x0);


    // очистить буфер, последующие заполнения должны очистить буферы

    bufInfo.name = "PageStatusBuffer"_atm;
    bufInfo.elementSize = sizeof(uint);
    bufInfo.size = offset;
    bufInfo.mode = BufferAccessMode::DeviceLocal;
    bufInfo.usageFlags = CoreGraphics::ReadWriteBuffer | CoreGraphics::TransferBufferDestination;
    bufInfo.data = nullptr;
    bufInfo.dataSize = 0;
    terrainVirtualTileState.pageStatusBuffer = CoreGraphics::CreateBuffer(bufInfo);

    bufInfo.name = "PageUpdateListBuffer"_atm;
    bufInfo.elementSize = sizeof(Terrain::PageUpdateList);
    bufInfo.size = 1;
    bufInfo.mode = BufferAccessMode::DeviceLocal;
    bufInfo.usageFlags = CoreGraphics::ReadWriteBuffer | CoreGraphics::TransferBufferSource;
    bufInfo.data = nullptr;
    bufInfo.dataSize = 0;
    terrainVirtualTileState.pageUpdateListBuffer = CoreGraphics::CreateBuffer(bufInfo);

    bufInfo.name = "PageUpdateReadbackBuffer"_atm;
    bufInfo.elementSize = sizeof(Terrain::PageUpdateList);
    bufInfo.size = 1;
    bufInfo.usageFlags = CoreGraphics::TransferBufferDestination;
    bufInfo.mode = BufferAccessMode::HostCached;
    bufInfo.data = nullptr;
    bufInfo.dataSize = 0;

    CoreGraphics::CmdBufferId cmdBuf = CoreGraphics::LockGraphicsSetupCommandBuffer("Настройка страницы местности");
    terrainVirtualTileState.pageUpdateReadbackBuffers = BufferSet(bufInfo);
    for (IndexT i = 0; i < terrainVirtualTileState.pageUpdateReadbackBuffers.buffers.Size(); i++)
    {
        CoreGraphics::BufferFill(cmdBuf, terrainVirtualTileState.pageUpdateReadbackBuffers.buffers[i], 0x0);
    }

    // мы закончили
    CoreGraphics::UnlockGraphicsSetupCommandBuffer(cmdBuf);

    CoreGraphics::TextureCreateInfo albedoCacheInfo;
    albedoCacheInfo.name = "Тайник местности Альбедо"_atm;
    albedoCacheInfo.width = PhysicalTexturePaddedSize;
    albedoCacheInfo.height = PhysicalTexturePaddedSize;
    albedoCacheInfo.format = CoreGraphics::PixelFormat::BC3;
    albedoCacheInfo.usage = CoreGraphics::TextureUsage::SampleTexture | CoreGraphics::TextureUsage::ReadWriteTexture;
    albedoCacheInfo.allowCast = true;
    terrainVirtualTileState.physicalAlbedoCacheBC = CoreGraphics::CreateTexture(albedoCacheInfo);

    CoreGraphics::TextureViewCreateInfo albedoCacheWriteInfo;
    albedoCacheWriteInfo.name = "Альбедо записи ландшафтного кэша"_atm;
    albedoCacheWriteInfo.tex = terrainVirtualTileState.physicalAlbedoCacheBC;
    albedoCacheWriteInfo.format = CoreGraphics::PixelFormat::R32G32B32A32;
    albedoCacheWriteInfo.usage = CoreGraphics::TextureUsage::ReadWriteTexture;
    terrainVirtualTileState.physicalAlbedoCacheBCWrite = CoreGraphics::CreateTextureView(albedoCacheWriteInfo);

    CoreGraphics::TextureCreateInfo normalCacheInfo;
    normalCacheInfo.name = "Нормали кэша ландшафта"_atm;
    normalCacheInfo.width = PhysicalTexturePaddedSize;
    normalCacheInfo.height = PhysicalTexturePaddedSize;
    normalCacheInfo.format = CoreGraphics::PixelFormat::BC5;
    normalCacheInfo.usage = CoreGraphics::TextureUsage::SampleTexture | CoreGraphics::TextureUsage::ReadWriteTexture;
    normalCacheInfo.allowCast = true;
    terrainVirtualTileState.physicalNormalCacheBC = CoreGraphics::CreateTexture(normalCacheInfo);

    CoreGraphics::TextureViewCreateInfo normalCacheWriteInfo;
    normalCacheWriteInfo.name = "Запись нормалей кэша ландшафта";
    normalCacheWriteInfo.format = CoreGraphics::PixelFormat::R32G32B32A32;
    normalCacheWriteInfo.tex = terrainVirtualTileState.physicalNormalCacheBC;
    normalCacheWriteInfo.usage = CoreGraphics::TextureUsage::ReadWriteTexture;
    terrainVirtualTileState.physicalNormalCacheBCWrite = CoreGraphics::CreateTextureView(normalCacheWriteInfo);

    CoreGraphics::TextureCreateInfo materialCacheInfo;
    materialCacheInfo.name = "Материал тайника местности"_atm;
    materialCacheInfo.width = PhysicalTexturePaddedSize;
    materialCacheInfo.height = PhysicalTexturePaddedSize;
    materialCacheInfo.format = CoreGraphics::PixelFormat::BC3;
    materialCacheInfo.usage = CoreGraphics::TextureUsage::SampleTexture | CoreGraphics::TextureUsage::ReadWriteTexture;
    materialCacheInfo.allowCast = true;
    terrainVirtualTileState.physicalMaterialCacheBC = CoreGraphics::CreateTexture(materialCacheInfo);

    CoreGraphics::TextureViewCreateInfo materialCacheWriteInfo;
    materialCacheWriteInfo.name = "Запись материала в кэш местности"_atm;
    materialCacheWriteInfo.format = CoreGraphics::PixelFormat::R32G32B32A32;
    materialCacheWriteInfo.tex = terrainVirtualTileState.physicalMaterialCacheBC;
    materialCacheWriteInfo.usage = CoreGraphics::TextureUsage::ReadWriteTexture;
    terrainVirtualTileState.physicalMaterialCacheBCWrite = CoreGraphics::CreateTextureView(materialCacheWriteInfo);

    CoreGraphics::TextureCreateInfo lowResAlbedoInfo;
    lowResAlbedoInfo.name = "Местность Лоурс Альбедо"_atm;
    lowResAlbedoInfo.mips = LowresFallbackMips;
    lowResAlbedoInfo.width = LowresFallbackSize;
    lowResAlbedoInfo.height = LowresFallbackSize;
    lowResAlbedoInfo.format = CoreGraphics::PixelFormat::R8G8B8A8;
    lowResAlbedoInfo.usage = CoreGraphics::TextureUsage::ReadWriteTexture | CoreGraphics::TextureUsage::SampleTexture | CoreGraphics::TextureUsage::TransferDestinationTexture | CoreGraphics::TextureUsage::TransferSourceTexture;
    terrainVirtualTileState.lowresAlbedo = CoreGraphics::CreateTexture(lowResAlbedoInfo);

    CoreGraphics::TextureCreateInfo lowResNormalInfo;
    lowResNormalInfo.name = "Ландшафт Лоурес Нормальный"_atm;
    lowResNormalInfo.mips = LowresFallbackMips;
    lowResNormalInfo.width = LowresFallbackSize;
    lowResNormalInfo.height = LowresFallbackSize;
    lowResNormalInfo.format = CoreGraphics::PixelFormat::R8G8B8A8;
    lowResNormalInfo.usage = CoreGraphics::TextureUsage::ReadWriteTexture | CoreGraphics::TextureUsage::SampleTexture | CoreGraphics::TextureUsage::TransferDestinationTexture | CoreGraphics::TextureUsage::TransferSourceTexture;
    terrainVirtualTileState.lowresNormal = CoreGraphics::CreateTexture(lowResNormalInfo);

    CoreGraphics::TextureCreateInfo lowResMaterialInfo;
    lowResMaterialInfo.name = "Материал местности Lowres"_atm;
    lowResMaterialInfo.mips = LowresFallbackMips;
    lowResMaterialInfo.width = LowresFallbackSize;
    lowResMaterialInfo.height = LowresFallbackSize;
    lowResMaterialInfo.format = CoreGraphics::PixelFormat::R8G8B8A8;
    lowResMaterialInfo.usage = CoreGraphics::TextureUsage::ReadWriteTexture | CoreGraphics::TextureUsage::SampleTexture | CoreGraphics::TextureUsage::TransferDestinationTexture | CoreGraphics::TextureUsage::TransferSourceTexture;
    terrainVirtualTileState.lowresMaterial = CoreGraphics::CreateTexture(lowResMaterialInfo);

    // настроить буфер виртуальных подтекстур
    terrainVirtualTileState.subTextures.Resize(subTextureCount);
    terrainVirtualTileState.gpuSubTextures.Resize(subTextureCount);

    uint subTextureCountX = terrainState.settings.worldSizeX / SubTextureWorldSize;
    uint subTextureCountZ = terrainState.settings.worldSizeZ / SubTextureWorldSize;
    for (uint z = 0; z < subTextureCountZ; z++)
    {
        for (uint x = 0; x < subTextureCountX; x++)
        {
            uint index = x + z * subTextureCountX;
            terrainVirtualTileState.subTextures[index].numTiles = 0;
            terrainVirtualTileState.subTextures[index].maxMip = 0;
            terrainVirtualTileState.subTextures[index].indirectionOffset.x = UINT32_MAX;
            terrainVirtualTileState.subTextures[index].indirectionOffset.y = UINT32_MAX;
            terrainVirtualTileState.subTextures[index].mipBias = 0;

            // position is calculated as the center of each cell, offset by half of the world size (so we are oriented around 0)
            float xPos = x * SubTextureWorldSize - terrainState.settings.worldSizeX / 2.0f;
            float zPos = z * SubTextureWorldSize - terrainState.settings.worldSizeZ / 2.0f;
            terrainVirtualTileState.subTextures[index].worldCoordinate.x = xPos;
            terrainVirtualTileState.subTextures[index].worldCoordinate.y = zPos;
            terrainVirtualTileState.gpuSubTextures[index].worldCoordinate[0] = xPos;
            terrainVirtualTileState.gpuSubTextures[index].worldCoordinate[1] = zPos;
            PackSubTexture(terrainVirtualTileState.subTextures[index], terrainVirtualTileState.gpuSubTextures[index]);
        }
    }

    // настроить занятость косвенности, текстура косвенности — 2048, а максимальный размер выделения — 256 пикселей косвенности
    terrainVirtualTileState.indirectionOccupancy.Setup(IndirectionTextureSize, 256, 1);

    // настройте физическую занятость текстуры, размер текстуры — 8192x8192 пикселей, а размер страницы — 256, поэтому эффективно сделайте это четырехъядерным деревом, которое заканчивается на отдельных страницах.
    //TerrainVirtualTileState.physicalTextureTileOccupancy.Setup(PhysicalTexturePaddedSize, PhysicalTexturePaddedSize, PhysicalTextureTilePaddedSize);
    terrainVirtualTileState.physicalTextureTileCache.Setup(PhysicalTextureTilePaddedSize, PhysicalTexturePaddedSize);

    terrainVirtualTileState.systemTable.ForEach([](const ResourceTableId table, const IndexT i)
    {
        ResourceTableSetConstantBuffer(table,
                                    {
                                        terrainState.biomeBuffer,
                                        Terrain::Table_System::MaterialLayers_SLOT,
                                        NEBULA_WHOLE_BUFFER_SIZE,
                                    }
        );

        ResourceTableSetConstantBuffer(table,
            {
                terrainState.systemConstants,
                Terrain::Table_System::TerrainSystemUniforms_SLOT,
                NEBULA_WHOLE_BUFFER_SIZE,
            });

        ResourceTableSetRWBuffer(table,
            {
                terrainVirtualTileState.subTextureBuffer,
                Terrain::Table_System::TerrainSubTexturesBuffer_SLOT,
                NEBULA_WHOLE_BUFFER_SIZE,
            });

        ResourceTableSetRWBuffer(table,
            {
                terrainVirtualTileState.pageUpdateListBuffer,
                Terrain::Table_System::PageUpdateListBuffer_SLOT,
                NEBULA_WHOLE_BUFFER_SIZE,
            });

        ResourceTableSetRWBuffer(table,
            {
                terrainVirtualTileState.pageStatusBuffer,
                Terrain::Table_System::PageStatusBuffer_SLOT,
                NEBULA_WHOLE_BUFFER_SIZE,
            });


        ResourceTableCommitChanges(table);
    });
    

    ResourceTableSetConstantBuffer(terrainVirtualTileState.runtimeTable,
        {
            terrainVirtualTileState.runtimeConstants,
            Terrain::Table_Batch::TerrainRuntimeUniforms_SLOT,
            sizeof(Terrain::TerrainRuntimeUniforms),
        });

    ResourceTableSetRWTexture(terrainVirtualTileState.runtimeTable,
        {
            terrainVirtualTileState.physicalAlbedoCacheBCWrite,
            Terrain::Table_Batch::AlbedoCacheOutputBC_SLOT
        });

    ResourceTableSetRWTexture(terrainVirtualTileState.runtimeTable,
        {
            terrainVirtualTileState.physicalNormalCacheBCWrite,
            Terrain::Table_Batch::NormalCacheOutputBC_SLOT
        });

    ResourceTableSetRWTexture(terrainVirtualTileState.runtimeTable,
        {
            terrainVirtualTileState.physicalMaterialCacheBCWrite,
            Terrain::Table_Batch::MaterialCacheOutputBC_SLOT
        });

    ResourceTableSetRWTexture(terrainVirtualTileState.runtimeTable,
        {
            terrainVirtualTileState.lowresAlbedo,
            Terrain::Table_Batch::AlbedoLowresOutput_SLOT
        });

    ResourceTableSetRWTexture(terrainVirtualTileState.runtimeTable,
        {
            terrainVirtualTileState.lowresNormal,
            Terrain::Table_Batch::NormalLowresOutput_SLOT
        });

    ResourceTableSetRWTexture(terrainVirtualTileState.runtimeTable,
        {
            terrainVirtualTileState.lowresMaterial,
            Terrain::Table_Batch::MaterialLowresOutput_SLOT
        });

    ResourceTableSetConstantBuffer(terrainVirtualTileState.runtimeTable,
        {
            terrainVirtualTileState.tileWriteBufferSet.DeviceBuffer(),
            Terrain::Table_Batch::TilesToWrite_SLOT
        });

    ResourceTableCommitChanges(terrainVirtualTileState.runtimeTable);

    if (CoreGraphics::RayTracingSupported)
    {
        FrameScript_default::RegisterSubgraph_TerrainRaytracingMeshGenerate_Compute([](const CoreGraphics::CmdBufferId cmdBuf, const Math::rectangle<int>& viewport, const IndexT frame, const IndexT bufferIndex)
        {
            if (raytracingState.updateMesh)
            {
                CmdBeginMarker(cmdBuf, NEBULA_MARKER_COMPUTE, "Сетка трассировки лучей при обновлении ландшафта");

                CmdSetShaderProgram(cmdBuf, raytracingState.meshProgram);
                CmdSetResourceTable(cmdBuf, raytracingState.meshGenTable, NEBULA_BATCH_GROUP, CoreGraphics::ComputePipeline, nullptr);
                CmdDispatch(cmdBuf, Math::divandroundup(raytracingState.numTris, 64), raytracingState.numPatches, 1);

                CmdEndMarker(cmdBuf);

                // Избегайте большего количества обновлений сетки
                raytracingState.updateMesh = false;

                raytracingState.setupBlasFrame = bufferIndex + CoreGraphics::GetNumBufferedFrames();
            }
        });
    }

    FrameScript_default::Bind_TerrainUpdateList(terrainVirtualTileState.pageUpdateListBuffer);
    FrameScript_default::Bind_TerrainVirtualPageStatuses(terrainVirtualTileState.pageStatusBuffer);
    FrameScript_default::Bind_TerrainSubTextures(terrainVirtualTileState.subTextureBuffer);
    FrameScript_default::Bind_TerrainIndirection(Frame::TextureImport(terrainVirtualTileState.indirectionTexture));
    FrameScript_default::Bind_TerrainIndirectionCopy(Frame::TextureImport(terrainVirtualTileState.indirectionTextureCopy));

    FrameScript_default::RegisterSubgraph_TerrainPrepare_Compute([](const CoreGraphics::CmdBufferId cmdBuf, const Math::rectangle<int>& viewport, const IndexT frame, const IndexT bufferIndex)
    {
        if (terrainVirtualTileState.indirectionBufferUpdatesThisFrame.Size() > 0
            || terrainVirtualTileState.indirectionBufferClearsThisFrame.Size() > 0
            || terrainVirtualTileState.indirectionTextureFromCopiesThisFrame.Size() > 0)
        {
            CmdBeginMarker(cmdBuf, NEBULA_MARKER_TRANSFER, "Перетасовать области косвенности");

            // Выполнить очистку
            if (terrainVirtualTileState.indirectionBufferClearsThisFrame.Size() > 0)
            {
                CmdBarrier(cmdBuf,
                    PipelineStage::HostWrite,
                    PipelineStage::TransferRead,
                    BarrierDomain::Global,
                    {
                        BufferBarrierInfo
                        {
                            terrainVirtualTileState.indirectionUploadBuffers.buffers[bufferIndex],
                            CoreGraphics::BufferSubresourceInfo()
                        },

                    });

                // выполнять перемещения и очищать пиксели косвенности
                CmdCopy(cmdBuf,
                    terrainVirtualTileState.indirectionUploadBuffers.buffers[bufferIndex], terrainVirtualTileState.indirectionBufferClearsThisFrame,
                    terrainVirtualTileState.indirectionTexture, terrainVirtualTileState.indirectionTextureClearsThisFrame);

                CmdBarrier(cmdBuf,
                    PipelineStage::TransferRead,
                    PipelineStage::HostWrite,
                    BarrierDomain::Global,
                    {
                        BufferBarrierInfo
                        {
                            terrainVirtualTileState.indirectionUploadBuffers.buffers[bufferIndex],
                            CoreGraphics::BufferSubresourceInfo()
                        },
                    });
            }

            // Выполнять мип-смены
            if (terrainVirtualTileState.indirectionTextureFromCopiesThisFrame.Size() > 0)
            {
                CmdCopy(cmdBuf,
                    terrainVirtualTileState.indirectionTextureCopy, terrainVirtualTileState.indirectionTextureFromCopiesThisFrame,
                    terrainVirtualTileState.indirectionTexture, terrainVirtualTileState.indirectionTextureToCopiesThisFrame);
            }

            // Выполнение обновлений на стороне ЦП
            if (terrainVirtualTileState.indirectionBufferUpdatesThisFrame.Size() > 0)
            {
                CmdBarrier(cmdBuf,
                    PipelineStage::HostWrite,
                    PipelineStage::TransferRead,
                    BarrierDomain::Global,
                    {
                        BufferBarrierInfo
                        {
                            terrainVirtualTileState.indirectionUploadBuffers.buffers[bufferIndex],
                            CoreGraphics::BufferSubresourceInfo()
                        },

                    });

                // обновить новые пиксели
                CmdCopy(cmdBuf,
                    terrainVirtualTileState.indirectionUploadBuffers.buffers[bufferIndex], terrainVirtualTileState.indirectionBufferUpdatesThisFrame,
                    terrainVirtualTileState.indirectionTexture, terrainVirtualTileState.indirectionTextureUpdatesThisFrame);

                CmdBarrier(cmdBuf,
                    PipelineStage::TransferRead,
                    PipelineStage::HostWrite,
                    BarrierDomain::Global,
                    {
                        BufferBarrierInfo
                        {
                            terrainVirtualTileState.indirectionUploadBuffers.buffers[bufferIndex],
                            CoreGraphics::BufferSubresourceInfo()
                        },
                    });
            }

            // Очистить ожидающие обновления
            terrainVirtualTileState.indirectionTextureFromCopiesThisFrame.Clear();
            terrainVirtualTileState.indirectionTextureToCopiesThisFrame.Clear();

            terrainVirtualTileState.indirectionBufferUpdatesThisFrame.Clear();
            terrainVirtualTileState.indirectionTextureUpdatesThisFrame.Clear();

            terrainVirtualTileState.indirectionBufferClearsThisFrame.Clear();
            terrainVirtualTileState.indirectionTextureClearsThisFrame.Clear();
            CmdEndMarker(cmdBuf);
        }

        // Если у нас есть обновления подтекстур, обязательно скопируйте и их.
        if (terrainVirtualTileState.virtualSubtextureBufferUpdate)
        {
            CmdBeginMarker(cmdBuf, NEBULA_MARKER_TRANSFER, "Копирование подтекстур из хост-буфера");
            CmdBarrier(cmdBuf,
                PipelineStage::HostWrite,
                PipelineStage::TransferRead,
                BarrierDomain::Global,
                {
                    BufferBarrierInfo
                    {
                        terrainVirtualTileState.subtextureStagingBuffers.buffers[bufferIndex],
                        CoreGraphics::BufferSubresourceInfo()
                    },
                });

            BufferCopy from, to;
            from.offset = 0;
            to.offset = 0;
            CmdCopy(cmdBuf
                , terrainVirtualTileState.subtextureStagingBuffers.buffers[bufferIndex], { from }
                , terrainVirtualTileState.subTextureBuffer, { to }
            , BufferGetByteSize(terrainVirtualTileState.subTextureBuffer));

            CmdBarrier(cmdBuf,
                PipelineStage::TransferRead,
                PipelineStage::HostWrite,
                BarrierDomain::Global,
                {
                    BufferBarrierInfo
                    {
                        terrainVirtualTileState.subtextureStagingBuffers.buffers[bufferIndex],
                        CoreGraphics::BufferSubresourceInfo()
                    },
                });

            terrainVirtualTileState.virtualSubtextureBufferUpdate = false;
            CmdEndMarker(cmdBuf);
        }
    }, {
        { FrameScript_default::BufferIndex::TerrainVirtualPageStatuses, CoreGraphics::PipelineStage::TransferWrite }
        , { FrameScript_default::BufferIndex::TerrainSubTextures, CoreGraphics::PipelineStage::TransferWrite }
    }, {
        { FrameScript_default::TextureIndex::TerrainIndirection, CoreGraphics::PipelineStage::TransferWrite }
        , { FrameScript_default::TextureIndex::TerrainIndirectionCopy, CoreGraphics::PipelineStage::TransferRead }
    });

    FrameScript_default::RegisterSubgraph_TerrainIndirectionCopy_Compute([](const CoreGraphics::CmdBufferId cmdBuf, const Math::rectangle<int>& viewport, const IndexT frame, const IndexT bufferIndex)
    {
        CmdBeginMarker(cmdBuf, NEBULA_MARKER_TRANSFER, "Копировать Mips косвенности");

        // Скопируйте этот кадр косвенности, чтобы скопировать
        TextureDimensions dims = TextureGetDimensions(terrainVirtualTileState.indirectionTexture);
        for (IndexT mip = 0; mip < TextureGetNumMips(terrainVirtualTileState.indirectionTexture); mip++)
        {
            TextureCopy cp;
            cp.mip = mip;
            cp.layer = 0;
            cp.region = Math::rectangle(0, 0, dims.width >> mip, dims.height >> mip);
            CmdCopy(cmdBuf,
                    terrainVirtualTileState.indirectionTexture, { cp }, terrainVirtualTileState.indirectionTextureCopy, { cp });
        }

        CmdEndMarker(cmdBuf);
    }, nullptr, {
        { FrameScript_default::TextureIndex::TerrainIndirection, CoreGraphics::PipelineStage::TransferRead }
        , { FrameScript_default::TextureIndex::TerrainIndirectionCopy, CoreGraphics::PipelineStage::TransferWrite }
    });

    FrameScript_default::RegisterSubgraphPipelines_TerrainPrepass_Pass([](const CoreGraphics::PassId pass, uint subpass)
    {
        if (terrainVirtualTileState.terrainPrepassPipeline != InvalidPipelineId)
            DestroyGraphicsPipeline(terrainVirtualTileState.terrainPrepassPipeline);
        terrainVirtualTileState.terrainPrepassPipeline = CreateGraphicsPipeline(
            {
                .shader = terrainVirtualTileState.terrainPrepassProgram,
                .pass = pass,
                .subpass = subpass,
                .inputAssembly = InputAssemblyKey{ { .topo = CoreGraphics::PrimitiveTopology::PatchList, .primRestart = false } }
            });
    });

    FrameScript_default::RegisterSubgraph_TerrainPrepass_Pass([](const CoreGraphics::CmdBufferId cmdBuf, const Math::rectangle<int>& viewport, const IndexT frame, const IndexT bufferIndex)
    {
        if (terrainState.renderToggle == false)
            return;

        // настройте состояние шейдера, установите шейдер до того, как мы установим расположение вершин
        CmdSetGraphicsPipeline(cmdBuf, terrainVirtualTileState.terrainPrepassPipeline);
        CmdSetVertexLayout(cmdBuf, terrainState.vlo);
        CmdSetPrimitiveTopology(cmdBuf, CoreGraphics::PrimitiveTopology::PatchList);

        // установить общие ресурсы
        CmdSetResourceTable(cmdBuf, terrainVirtualTileState.systemTable.tables[bufferIndex], NEBULA_SYSTEM_GROUP, GraphicsPipeline, nullptr);

        // Рисовать ландшафты
        Util::Array<TerrainRuntimeInfo>& runtimes = terrainAllocator.GetArray<Terrain_RuntimeInfo>();

        SizeT numQuadsX = terrainState.settings.quadsPerTileX;
        SizeT numQuadsY = terrainState.settings.quadsPerTileY;
        SizeT numVertsX = numQuadsX + 1;
        SizeT numVertsY = numQuadsY + 1;
        SizeT numTris = numVertsX * numVertsY;

        CoreGraphics::PrimitiveGroup group;
        group.SetBaseIndex(0);
        group.SetBaseVertex(0);
        group.SetNumIndices(numTris * 4);
        group.SetNumVertices(0);

        // проходить и визуализировать экземпляры ландшафта
        for (IndexT i = 0; i < runtimes.Size(); i++)
        {
            TerrainRuntimeInfo& rt = runtimes[i];
            CmdSetResourceTable(cmdBuf, terrainVirtualTileState.runtimeTable, NEBULA_BATCH_GROUP, GraphicsPipeline, nullptr);

            CmdSetVertexBuffer(cmdBuf, 0, rt.vbo, 0);
            CmdSetIndexBuffer(cmdBuf, IndexType::Index32, rt.ibo, 0);
            CmdDraw(cmdBuf, terrainVirtualTileState.numPatchesThisFrame, group);
        }
    }, {
        { FrameScript_default::BufferIndex::TerrainUpdateList, CoreGraphics::PipelineStage::PixelShaderWrite }
        , { FrameScript_default::BufferIndex::TerrainSubTextures, CoreGraphics::PipelineStage::PixelShaderRead }
    });

    FrameScript_default::RegisterSubgraph_TerrainPageExtract_Compute([](const CoreGraphics::CmdBufferId cmdBuf, const Math::rectangle<int>& viewport, const IndexT frame, const IndexT bufferIndex)
    {
        CmdBeginMarker(cmdBuf, NEBULA_MARKER_TRANSFER, "Копирование страниц для повторного чтения");
        CoreGraphics::BufferCopy from, to;
        from.offset = 0;
        to.offset = 0;
        CmdCopy(
            cmdBuf,
            terrainVirtualTileState.pageUpdateListBuffer, { from }, terrainVirtualTileState.pageUpdateReadbackBuffers.buffers[bufferIndex], { to },
            sizeof(Terrain::PageUpdateList)
        );
        CmdEndMarker(cmdBuf);
    }, {
        { FrameScript_default::BufferIndex::TerrainUpdateList, CoreGraphics::PipelineStage::TransferRead }
    });

    FrameScript_default::RegisterSubgraph_SunTerrainShadows_Compute([](const CoreGraphics::CmdBufferId cmdBuf, const Math::rectangle<int>& viewport, const IndexT frame, const IndexT bufferIndex)
    {
        if (!terrainState.renderToggle)
            return;

        Threading::CriticalScope scope(&terrainState.syncPoint);
        if (!terrainState.updateShadowMap)
            return;

        terrainState.updateShadowMap = false;

        // Настройте состояние шейдера, установите шейдер перед установкой макета вершин.
        CmdSetShaderProgram(cmdBuf, terrainState.terrainShadowProgram);

        // Установить общие ресурсы
        CmdSetResourceTable(cmdBuf, terrainState.resourceTable, NEBULA_SYSTEM_GROUP, ComputePipeline, nullptr);
        CmdSetResourceTable(cmdBuf, terrainVirtualTileState.runtimeTable, NEBULA_BATCH_GROUP, ComputePipeline, nullptr);

        // Dispatch
        uint x = Math::divandroundup(TerrainShadowMapSize, 64);
        CmdDispatch(cmdBuf, x, TerrainShadowMapSize, 1);
    });

    FrameScript_default::RegisterSubgraph_TerrainPagesClear_Compute([](const CoreGraphics::CmdBufferId cmdBuf, const Math::rectangle<int>& viewport, const IndexT frame, const IndexT bufferIndex)
    {
        CmdBeginMarker(cmdBuf, NEBULA_MARKER_COMPUTE, "Очистить буфер статуса страницы");

        CmdSetShaderProgram(cmdBuf, terrainVirtualTileState.terrainPageClearUpdateBufferProgram);
        CmdSetResourceTable(cmdBuf, terrainVirtualTileState.systemTable.tables[bufferIndex], NEBULA_SYSTEM_GROUP, ComputePipeline, nullptr);

        // запустите один вычислительный шейдер, чтобы очистить количество записей на странице
        uint numDispatches = Math::divandroundup(Terrain::MAX_PAGE_UPDATES, 64);
        CmdDispatch(cmdBuf, numDispatches, 1, 1);

        CmdEndMarker(cmdBuf);
    }, {
        { FrameScript_default::BufferIndex::TerrainUpdateList, CoreGraphics::PipelineStage::ComputeShaderWrite }
        , { FrameScript_default::BufferIndex::TerrainVirtualPageStatuses, CoreGraphics::PipelineStage::ComputeShaderWrite }
    });

    FrameScript_default::RegisterSubgraph_TerrainUpdateCaches_Compute([](const CoreGraphics::CmdBufferId cmdBuf, const Math::rectangle<int>& viewport, const IndexT frame, const IndexT bufferIndex)
    {
        Threading::CriticalScope scope(&terrainState.syncPoint);
        if (terrainVirtualTileState.updateLowres)
        {
            terrainVirtualTileState.updateLowres = false;

            CmdBeginMarker(cmdBuf, NEBULA_MARKER_COMPUTE, "Обновить кэши Lowres");

            // Переход рендерится в mips для чтения шейдера
            CoreGraphics::CmdBarrier(
                cmdBuf,
                CoreGraphics::PipelineStage::AllShadersRead,
                CoreGraphics::PipelineStage::ComputeShaderWrite,
                CoreGraphics::BarrierDomain::Global,
                {
                    {
                        terrainVirtualTileState.lowresAlbedo,
                        CoreGraphics::TextureSubresourceInfo(),
                    },
                    {
                        terrainVirtualTileState.lowresMaterial,
                        CoreGraphics::TextureSubresourceInfo(),
                    },
                    {
                        terrainVirtualTileState.lowresNormal,
                        CoreGraphics::TextureSubresourceInfo(),
                    }
                });

            // Состояние настройки для обновления
            CmdSetShaderProgram(cmdBuf, terrainVirtualTileState.terrainWriteLowresProgram);
            CmdSetResourceTable(cmdBuf, terrainVirtualTileState.systemTable.tables[bufferIndex], NEBULA_SYSTEM_GROUP, ComputePipeline, nullptr);
            CmdSetResourceTable(cmdBuf, terrainVirtualTileState.runtimeTable, NEBULA_BATCH_GROUP, ComputePipeline, nullptr);

            CoreGraphics::TextureDimensions dims = CoreGraphics::TextureGetDimensions(terrainVirtualTileState.lowresAlbedo);

            CmdDispatch(cmdBuf, Math::divandroundup(dims.width, 8), Math::divandroundup(dims.height, 8), 1);

            // Переход рендерится в mips для чтения шейдера
            CoreGraphics::CmdBarrier(
                cmdBuf,
                CoreGraphics::PipelineStage::ComputeShaderWrite,
                CoreGraphics::PipelineStage::AllShadersRead,
                CoreGraphics::BarrierDomain::Global,
                {
                    {
                        terrainVirtualTileState.lowresAlbedo,
                        CoreGraphics::TextureSubresourceInfo(),
                    },
                    {
                        terrainVirtualTileState.lowresMaterial,
                        CoreGraphics::TextureSubresourceInfo(),
                    },
                    {
                        terrainVirtualTileState.lowresNormal,
                        CoreGraphics::TextureSubresourceInfo(),
                    }
                });

            // Теперь сгенерируйте MIP-карты
            CoreGraphics::TextureGenerateMipmaps(cmdBuf, terrainVirtualTileState.lowresAlbedo);
            CoreGraphics::TextureGenerateMipmaps(cmdBuf, terrainVirtualTileState.lowresMaterial);
            CoreGraphics::TextureGenerateMipmaps(cmdBuf, terrainVirtualTileState.lowresNormal);

            CmdEndMarker(cmdBuf);
        }

        if (terrainVirtualTileState.tileWritesThisFrame.Size() > 0)
        {
            CmdBeginMarker(cmdBuf, NEBULA_MARKER_COMPUTE, "Обновить кэши текстур");

            CoreGraphics::BarrierPush(
                cmdBuf
                , CoreGraphics::PipelineStage::ComputeShaderRead
                , CoreGraphics::PipelineStage::TransferWrite
                , CoreGraphics::BarrierDomain::Global
                , {
                    CoreGraphics::BufferBarrierInfo
                    {
                        .buf = terrainVirtualTileState.tileWriteBufferSet.DeviceBuffer(),
                        .subres = CoreGraphics::BufferSubresourceInfo{}
                    }
                }
            );
            terrainVirtualTileState.tileWriteBufferSet.Flush(cmdBuf, terrainVirtualTileState.tileWritesThisFrame.ByteSize());
            CoreGraphics::BarrierPop(cmdBuf);
            CmdSetShaderProgram(cmdBuf, terrainVirtualTileState.terrainTileWriteProgram);
            CmdSetResourceTable(cmdBuf, terrainVirtualTileState.systemTable.tables[bufferIndex], NEBULA_SYSTEM_GROUP, ComputePipeline, nullptr);
            Util::Array<TerrainRuntimeInfo>& runtimes = terrainAllocator.GetArray<Terrain_RuntimeInfo>();

            // пройти через ожидающие обновления страницы и выполнить рендеринг в физические кэши текстур.
            for (IndexT i = 0; i < runtimes.Size(); i++)
            {
                CmdSetResourceTable(cmdBuf, terrainVirtualTileState.runtimeTable, NEBULA_BATCH_GROUP, ComputePipeline, nullptr);
                static const uint numDispatches = Math::divandroundup(PhysicalTextureTilePaddedSize, 8);

                CmdDispatch(cmdBuf, numDispatches, numDispatches, terrainVirtualTileState.tileWritesThisFrame.Size());
            }
            terrainVirtualTileState.tileWritesThisFrame.Clear();

            CmdEndMarker(cmdBuf);
        }
    }, nullptr, {
        { FrameScript_default::TextureIndex::TerrainAlbedoCache, CoreGraphics::PipelineStage::ComputeShaderWrite }
        , { FrameScript_default::TextureIndex::TerrainMaterialCache, CoreGraphics::PipelineStage::ComputeShaderWrite }
        , { FrameScript_default::TextureIndex::TerrainNormalCache, CoreGraphics::PipelineStage::ComputeShaderWrite }
    });

    FrameScript_default::RegisterSubgraphPipelines_TerrainResolve_Pass([](const CoreGraphics::PassId pass, uint subpass)
    {
        if (terrainVirtualTileState.terrainResolvePipeline != InvalidPipelineId)
            DestroyGraphicsPipeline(terrainVirtualTileState.terrainResolvePipeline);
        terrainVirtualTileState.terrainResolvePipeline = CoreGraphics::CreateGraphicsPipeline(
            {
                .shader = terrainVirtualTileState.terrainResolveProgram,
                .pass = pass,
                .subpass = subpass,
                .inputAssembly = InputAssemblyKey{ { .topo = CoreGraphics::PrimitiveTopology::PatchList, .primRestart = false } }
            });
    });
    FrameScript_default::RegisterSubgraph_TerrainResolve_Pass([](const CoreGraphics::CmdBufferId cmdBuf, const Math::rectangle<int>& viewport, const IndexT frame, const IndexT bufferIndex)
    {
        CmdSetGraphicsPipeline(cmdBuf, terrainVirtualTileState.terrainResolvePipeline);
        CmdSetVertexLayout(cmdBuf, terrainState.vlo);
        CmdSetPrimitiveTopology(cmdBuf, CoreGraphics::PrimitiveTopology::PatchList);

        // установить общие ресурсы
        CmdSetResourceTable(cmdBuf, terrainVirtualTileState.systemTable.tables[bufferIndex], NEBULA_SYSTEM_GROUP, GraphicsPipeline, nullptr);

        if (terrainState.renderToggle == false)
            return;

        // Рисовать ландшафты
        Util::Array<TerrainRuntimeInfo>& runtimes = terrainAllocator.GetArray<Terrain_RuntimeInfo>();

        SizeT numQuadsX = terrainState.settings.quadsPerTileX;
        SizeT numQuadsY = terrainState.settings.quadsPerTileY;
        SizeT numVertsX = numQuadsX + 1;
        SizeT numVertsY = numQuadsY + 1;
        SizeT numTris = numVertsX * numVertsY;

        CoreGraphics::PrimitiveGroup group;
        group.SetBaseIndex(0);
        group.SetBaseVertex(0);
        group.SetNumIndices(numTris * 4);
        group.SetNumVertices(0);

        // проходить и визуализировать экземпляры ландшафта
        for (IndexT i = 0; i < runtimes.Size(); i++)
        {
            TerrainRuntimeInfo& rt = runtimes[i];
            CmdSetResourceTable(cmdBuf, terrainVirtualTileState.runtimeTable, NEBULA_BATCH_GROUP, GraphicsPipeline, nullptr);

            CmdSetVertexBuffer(cmdBuf, 0, rt.vbo, 0);
            CmdSetIndexBuffer(cmdBuf, IndexType::Index32, rt.ibo, 0);
            CmdDraw(cmdBuf, terrainVirtualTileState.numPatchesThisFrame, group);
        }
    }, nullptr, {
        { FrameScript_default::TextureIndex::TerrainAlbedoCache, CoreGraphics::PipelineStage::PixelShaderRead }
        , { FrameScript_default::TextureIndex::TerrainMaterialCache, CoreGraphics::PipelineStage::PixelShaderRead }
        , { FrameScript_default::TextureIndex::TerrainNormalCache, CoreGraphics::PipelineStage::PixelShaderRead }
        , { FrameScript_default::TextureIndex::TerrainIndirection, CoreGraphics::PipelineStage::PixelShaderRead }
    });

    // создать ВЛО
    terrainState.layers = 1;
    terrainState.debugRender = false;
    terrainState.renderToggle = true;
    terrainState.biomeCounter = 0;
}

//------------------------------------------------------------------------------
/**
*/
void 
TerrainContext::Discard()
{
    DestroyBuffer(terrainVirtualTileState.pageStatusBuffer);
    DestroyBuffer(terrainVirtualTileState.subTextureBuffer);
    terrainVirtualTileState.subtextureStagingBuffers.~BufferSet();
    DestroyBuffer(terrainVirtualTileState.pageUpdateListBuffer);
    terrainVirtualTileState.pageUpdateReadbackBuffers.~BufferSet();
}

//------------------------------------------------------------------------------
/**
*/
void 
TerrainContext::SetupTerrain(
    const Graphics::GraphicsEntityId entity
    , const Resources::ResourceName& heightMap
    , const Resources::ResourceName& decisionMap
    , bool enableRayTracing)
{
    n_assert(terrainState.settings.worldSizeX > 0);
    n_assert(terrainState.settings.worldSizeZ > 0);
    using namespace CoreGraphics;
    const Graphics::ContextEntityId cid = GetContextId(entity);
    TerrainRuntimeInfo& runtimeInfo = terrainAllocator.Get<Terrain_RuntimeInfo>(cid.id);
    runtimeInfo.enableRayTracing = CoreGraphics::RayTracingSupported && enableRayTracing;

    runtimeInfo.worldWidth = terrainState.settings.worldSizeX;
    runtimeInfo.worldHeight = terrainState.settings.worldSizeZ;
    runtimeInfo.maxHeight = terrainState.settings.maxHeight;
    runtimeInfo.minHeight = terrainState.settings.minHeight;

    // разделить измерения мира на 
    runtimeInfo.numTilesX = terrainState.settings.worldSizeX / terrainState.settings.tileWidth;
    runtimeInfo.numTilesY = terrainState.settings.worldSizeZ / terrainState.settings.tileHeight;
    runtimeInfo.tileWidth = terrainState.settings.tileWidth;
    runtimeInfo.tileHeight = terrainState.settings.tileHeight;
    SizeT height = terrainState.settings.maxHeight - terrainState.settings.minHeight;
    SizeT numQuadsX = terrainState.settings.quadsPerTileX;
    SizeT numQuadsY = terrainState.settings.quadsPerTileY;
    SizeT numVertsX = numQuadsX + 1;
    SizeT numVertsY = numQuadsY + 1;
    SizeT vertDistanceX = terrainState.settings.tileWidth  / terrainState.settings.quadsPerTileX;
    SizeT vertDistanceY = terrainState.settings.tileHeight / terrainState.settings.quadsPerTileY;

    CoreGraphics::BufferCreateInfo bufInfo;
    bufInfo.name = "TerrainPerPatchData"_atm;
    bufInfo.size = runtimeInfo.numTilesX * runtimeInfo.numTilesY;
    bufInfo.elementSize = sizeof(Terrain::TerrainPatch);
    bufInfo.mode = CoreGraphics::DeviceAndHost;
    bufInfo.usageFlags = CoreGraphics::ReadWriteBuffer;
    terrainVirtualTileState.patchConstants = CoreGraphics::BufferSet(bufInfo);

    terrainVirtualTileState.systemTable.ForEach([](const ResourceTableId table, IndexT i)
    {
        ResourceTableSetRWBuffer(
            table
            , ResourceTableBuffer(terrainVirtualTileState.patchConstants.buffers[i], Terrain::Table_System::TerrainPatchData_SLOT)
            );

        ResourceTableCommitChanges(table);
    });

    // выделить буфер вершин тайла
    Util::FixedArray<TerrainVert> verts(numVertsX * numVertsY);

    // выделить буфер индекса ландшафта, каждый четвертый пиксель будет генерировать два треугольника 
    SizeT numTris = numVertsX * numVertsY;
    Util::FixedArray<TerrainQuad> quads(numQuadsX * numQuadsY);
    Util::FixedArray<Terrain::TerrainPatch> patchData(runtimeInfo.numTilesY * runtimeInfo.numTilesX);

    // разделы настройки
    uint patchCounter = 0;
    for (uint y = 0; y < runtimeInfo.numTilesY; y++)
    {
        for (uint x = 0; x < runtimeInfo.numTilesX; x++)
        {
            Math::bbox box;
            box.set(
                Math::point(
                    x * terrainState.settings.tileWidth - terrainState.settings.worldSizeX / 2 + terrainState.settings.tileWidth / 2,
                    terrainState.settings.minHeight,
                    y * terrainState.settings.tileHeight - terrainState.settings.worldSizeZ / 2 + terrainState.settings.tileHeight / 2
                ),
                Math::vector(terrainState.settings.tileWidth / 2, height, terrainState.settings.tileHeight / 2));
            runtimeInfo.sectionBoxes.Append(box);
            runtimeInfo.sectorVisible.Append(true);
            runtimeInfo.sectorLod.Append(1000.0f);
            runtimeInfo.sectorUpdateTextureTile.Append(false);
            runtimeInfo.sectorUniformOffsets.Append(Util::FixedArray<uint>(2, 0));
            runtimeInfo.sectorTileOffsets.Append(Util::FixedArray<uint>(2, 0));
            runtimeInfo.sectorAllocatedTile.Append(Math::uint3{ UINT32_MAX, UINT32_MAX, UINT32_MAX });
            runtimeInfo.sectorTextureTileSize.Append(Math::uint2{ 0, 0 });
            runtimeInfo.sectorUv.Append({ x * terrainState.settings.tileWidth / terrainState.settings.worldSizeX, y * terrainState.settings.tileHeight / terrainState.settings.worldSizeZ });

            if (CoreGraphics::RayTracingSupported)
            {
                Terrain::TerrainPatch patch;
                patch.PosOffset[0] = box.pmin.x;
                patch.PosOffset[1] = box.pmin.z;
                patch.UvOffset[0] = runtimeInfo.sectorUv.Back().x;
                patch.UvOffset[1] = runtimeInfo.sectorUv.Back().y;
                patchData[patchCounter++] = patch;
            }
        }
    }

    // пройти и настроить секции, ориентированные вокруг ориго, чтобы половина секций была в отрицательном положении
    uint quad = 0;
    for (IndexT y = 0; y < numQuadsY; y++)
    {
        for (IndexT x = 0; x < numQuadsX; x++)
        {
            struct Vertex
            {
                Math::vec4 position;
                Math::vec2 uv;
            };
            Vertex v1, v2, v3, v4;

            // установить вершины ландшафта, UV должно быть частью размера мира
            v1.position.set(x * vertDistanceX, 0, y * vertDistanceY, 1);
            v1.uv = Math::vec2(x * vertDistanceX / float(terrainState.settings.tileWidth), y * vertDistanceY / float(terrainState.settings.tileHeight));

            v2.position.set((x + 1) * vertDistanceX, 0, y * vertDistanceY, 1);
            v2.uv = Math::vec2((x + 1) * vertDistanceX / float(terrainState.settings.tileWidth), y * vertDistanceY / float(terrainState.settings.tileHeight));

            v3.position.set(x * vertDistanceX, 0, (y + 1) * vertDistanceY, 1);
            v3.uv = Math::vec2(x * vertDistanceX / float(terrainState.settings.tileWidth), (y + 1) * vertDistanceY / float(terrainState.settings.tileHeight));

            v4.position.set((x + 1) * vertDistanceX, 0, (y + 1) * vertDistanceY, 1);
            v4.uv = Math::vec2((x + 1) * vertDistanceX / float(terrainState.settings.tileWidth), (y + 1) * vertDistanceY / float(terrainState.settings.tileHeight));

            // вычислить локальный индекс плитки и смещения
            IndexT locX = x;
            IndexT locY = y;
            IndexT nextX = x + 1;
            IndexT nextY = y + 1;

            // получить данные буфера, чтобы мы могли их обновить
            IndexT vidx1, vidx2, vidx3, vidx4;
            vidx1 = locX + locY * numVertsX;
            vidx2 = nextX + locY * numVertsX;
            vidx3 = locX + nextY * numVertsX;
            vidx4 = nextX + nextY * numVertsX;

            TerrainVert& vt1 = verts[vidx1];
            v1.position.storeu3(vt1.position.v);
            vt1.uv[0] = uint(v1.uv.x * 1000);
            vt1.uv[1] = uint(v1.uv.y * 1000);

            TerrainVert& vt2 = verts[vidx2];
            v2.position.storeu3(vt2.position.v);
            vt2.uv[0] = uint(v2.uv.x * 1000);
            vt2.uv[1] = uint(v2.uv.y * 1000);

            TerrainVert& vt3 = verts[vidx3];
            v3.position.storeu3(vt3.position.v);
            vt3.uv[0] = uint(v3.uv.x * 1000);
            vt3.uv[1] = uint(v3.uv.y * 1000);

            TerrainVert& vt4 = verts[vidx4];
            v4.position.storeu3(vt4.position.v);
            vt4.uv[0] = uint(v4.uv.x * 1000);
            vt4.uv[1] = uint(v4.uv.y * 1000);

            // настроить треугольник трис
            TerrainQuad& q1 = quads[quad++];
            q1.a = vidx1;
            q1.b = vidx2;
            q1.c = vidx3;
            q1.d = vidx4;
        }
    }

    // создать вбо
    BufferCreateInfo vboInfo;
    vboInfo.name = "terrain_vbo"_atm;
    vboInfo.size = numVertsX * numVertsY;
    vboInfo.elementSize = CoreGraphics::VertexLayoutGetSize(terrainState.vlo);
    vboInfo.mode = CoreGraphics::DeviceLocal;
    vboInfo.usageFlags = CoreGraphics::VertexBuffer | (runtimeInfo.enableRayTracing ? CoreGraphics::ReadWriteBuffer : 0x0);
    vboInfo.data = verts.Begin();
    vboInfo.dataSize = verts.ByteSize();
    runtimeInfo.vbo = CreateBuffer(vboInfo);

    // создать ибо
    BufferCreateInfo iboInfo;
    iboInfo.name = "terrain_ibo"_atm;
    iboInfo.size = numTris * 4;
    iboInfo.elementSize = CoreGraphics::IndexType::SizeOf(CoreGraphics::IndexType::Index32);
    iboInfo.mode = CoreGraphics::DeviceLocal;
    iboInfo.usageFlags = CoreGraphics::IndexBuffer | (runtimeInfo.enableRayTracing ? CoreGraphics::ReadWriteBuffer : 0x0);
    iboInfo.data = quads.Begin();
    iboInfo.dataSize = quads.ByteSize();
    runtimeInfo.ibo = CreateBuffer(iboInfo);

    // Настройте дополнительные буферы вершин для создания прокси-сервера трассировки лучей, если трассировка лучей включена.
    if (runtimeInfo.enableRayTracing)
    {
        CoreGraphics::BufferCreateInfo patchBufferInfo;
        patchBufferInfo.byteSize = patchData.ByteSize();
        patchBufferInfo.usageFlags = CoreGraphics::BufferUsageFlag::ReadWriteBuffer;
        patchBufferInfo.mode = CoreGraphics::BufferAccessMode::DeviceLocal;
        patchBufferInfo.data = patchData.Begin();
        patchBufferInfo.dataSize = patchData.ByteSize();
        raytracingState.patchBuffer = CoreGraphics::CreateBuffer(patchBufferInfo);

        CoreGraphics::BufferCreateInfo constantBufferInfo;
        constantBufferInfo.byteSize = patchData.ByteSize();
        constantBufferInfo.usageFlags = CoreGraphics::BufferUsageFlag::ConstantBuffer;
        constantBufferInfo.mode = CoreGraphics::BufferAccessMode::DeviceAndHost;
        raytracingState.constantsBuffer = CoreGraphics::CreateBuffer(constantBufferInfo);


        // Create index buffer for triangles used by raytracing
        Util::Array<IndexT> indices;
        uint quad = 0;
        for (IndexT y = 0; y < numQuadsY; y++)
        {
            for (IndexT x = 0; x < numQuadsX; x++)
            {
                // вычислить локальный индекс плитки и смещения
                IndexT locX = x;
                IndexT locY = y;
                IndexT nextX = x + 1;
                IndexT nextY = y + 1;

                // получить данные буфера, чтобы мы могли их обновить
                IndexT vidx1, vidx2, vidx3, vidx4;
                vidx1 = locX + locY * numVertsX;
                vidx2 = nextX + locY * numVertsX;
                vidx3 = locX + nextY * numVertsX;
                vidx4 = nextX + nextY * numVertsX;

                // настроить треугольник трис
                indices.Append(vidx1, vidx3, vidx4);
                indices.Append(vidx4, vidx2, vidx1);
            }
        }

        raytracingState.vertexBuffer = CoreGraphics::AllocateVertices(vboInfo.size * vboInfo.elementSize * runtimeInfo.numTilesY * runtimeInfo.numTilesX);
        raytracingState.indexBuffer = CoreGraphics::AllocateIndices(indices.Size() * IndexType::SizeOf(IndexType::Index32) * runtimeInfo.numTilesY * runtimeInfo.numTilesX);

        CoreGraphics::ResourceTableSetConstantBuffer(raytracingState.meshGenTable,
                                        CoreGraphics::ResourceTableBuffer(raytracingState.constantsBuffer, TerrainMeshGenerate::Table_Batch::GenerationConstants_SLOT)
        );
        CoreGraphics::ResourceTableSetRWBuffer(raytracingState.meshGenTable,
                                       CoreGraphics::ResourceTableBuffer(runtimeInfo.vbo, TerrainMeshGenerate::Table_Batch::VertexInput_SLOT)
        );
        CoreGraphics::ResourceTableSetRWBuffer(raytracingState.meshGenTable,
                                       CoreGraphics::ResourceTableBuffer(runtimeInfo.ibo, TerrainMeshGenerate::Table_Batch::IndexInput_SLOT)
        );
        CoreGraphics::ResourceTableSetRWBuffer(raytracingState.meshGenTable,
                                       CoreGraphics::ResourceTableBuffer(
                                               CoreGraphics::GetVertexBuffer()
                                               , TerrainMeshGenerate::Table_Batch::VertexOutput_SLOT
                                               , raytracingState.vertexBuffer.size
                                               , raytracingState.vertexBuffer.offset
        ));

        CoreGraphics::ResourceTableSetRWBuffer(raytracingState.meshGenTable,
                                       CoreGraphics::ResourceTableBuffer(raytracingState.patchBuffer, TerrainMeshGenerate::Table_Batch::TerrainPatchData_SLOT)
        );
        CoreGraphics::ResourceTableCommitChanges(raytracingState.meshGenTable);

        CoreGraphics::BufferCreateInfo indexUploadBufferInfo;
        indexUploadBufferInfo.byteSize = indices.Size() * IndexType::SizeOf(IndexType::Index32);
        indexUploadBufferInfo.mode = CoreGraphics::BufferAccessMode::HostLocal;
        indexUploadBufferInfo.usageFlags = CoreGraphics::BufferUsageFlag::TransferBufferSource;
        indexUploadBufferInfo.data = indices.Begin();
        indexUploadBufferInfo.dataSize = indices.ByteSize();
        CoreGraphics::BufferId tempIndexBuffer = CoreGraphics::CreateBuffer(indexUploadBufferInfo);

        CoreGraphics::CmdBufferId cmdBuf = CoreGraphics::LockGraphicsSetupCommandBuffer("Настройка геометрии местности");

        // Индексный буфер статичен для всех патчей, поэтому давайте просто скопируем его.
        for (IndexT i = 0; i < runtimeInfo.numTilesY * runtimeInfo.numTilesX; i++)
        {
            CoreGraphics::BufferCopy fromCopy{ .offset = 0 }, toCopy{ .offset = raytracingState.indexBuffer.offset + i * (indices.ByteSize()) };
            CoreGraphics::CmdCopy(cmdBuf, tempIndexBuffer, { fromCopy }, CoreGraphics::GetIndexBuffer(), { toCopy }, indices.ByteSize());
        }
        CoreGraphics::UnlockGraphicsSetupCommandBuffer(cmdBuf);
        CoreGraphics::DestroyBuffer(tempIndexBuffer);

        // Приготовьтесь запустить вычислительный шейдер для смещения каждой вершины на основе карты высот.
        raytracingState.numPatches = runtimeInfo.numTilesY * runtimeInfo.numTilesX;
        raytracingState.numTris = numQuadsY * numQuadsX * 2;

        Graphics::RegisterEntity<Raytracing::RaytracingContext>(entity);

        CoreGraphics::PrimitiveGroup group;
        group.SetBaseIndex(0);
        group.SetBaseVertex(0);
        group.SetNumIndices(indices.Size());
        group.SetNumVertices(0);

        MaterialInterfaces::TerrainMaterial mat;
        mat.LowresAlbedoFallback = CoreGraphics::TextureGetBindlessHandle(terrainVirtualTileState.lowresAlbedo);
        mat.LowresMaterialFallback = CoreGraphics::TextureGetBindlessHandle(terrainVirtualTileState.lowresMaterial);
        mat.LowresNormalFallback = CoreGraphics::TextureGetBindlessHandle(terrainVirtualTileState.lowresNormal);

        /// Настройка с трассировкой лучей
        Util::Array<Math::mat4> patchTransforms;
        patchTransforms.Resize(runtimeInfo.sectionBoxes.Size());
        uint patchCounter = 0;
        for (uint y = 0; y < runtimeInfo.numTilesY; y++)
        {
            for (uint x = 0; x < runtimeInfo.numTilesX; x++)
            {
                Math::point pt(
                    x * terrainState.settings.tileWidth - terrainState.settings.worldSizeX / 2 + terrainState.settings.tileWidth / 2,
                    terrainState.settings.minHeight,
                    y * terrainState.settings.tileHeight - terrainState.settings.worldSizeZ / 2 + terrainState.settings.tileHeight / 2
                );

                patchTransforms[patchCounter++].translate(xyz(pt));
            }
        }

        uint materialOffset = Materials::MaterialLoader::RegisterTerrainMaterial(mat);
        Raytracing::RaytracingContext::SetupMesh(
            entity
            , Raytracing::UpdateType::Static
            , CoreGraphics::VertexComponent::Float3
            , CoreGraphics::IndexType::Index32
            , raytracingState.vertexBuffer
            , raytracingState.indexBuffer
            , group
            , vboInfo.elementSize
            , verts.ByteSize()
            , patchTransforms
            , materialOffset
            , MaterialTemplates::MaterialProperties::Terrain
            , CoreGraphics::VertexLayoutType::Normal);

        // Сделайте BLAS недействительным, когда вычислительный шейдер сместил все участки местности.
        raytracingState.terrainSetupCallback = [entity]()
        {
            Raytracing::RaytracingContext::InvalidateBLAS(entity);
        };
        raytracingState.setupBlasFrame = 0xFFFFFFFF;
    }

    runtimeInfo.loadBits = 0x0;
    runtimeInfo.lowresGenerated = false;
    runtimeInfo.decisionMap = Resources::CreateResource(decisionMap, "terrain"_atm, [&runtimeInfo](Resources::ResourceId id)
    {
        runtimeInfo.decisionMap = id;
        runtimeInfo.lowresGenerated = false;
        runtimeInfo.loadBits |= TerrainRuntimeInfo::DecisionMapLoaded;
    }, nullptr, false, false);

    runtimeInfo.heightMap = Resources::CreateResource(heightMap, "terrain"_atm, [&runtimeInfo, numVertsX, numVertsY](Resources::ResourceId id)
    {
        Threading::CriticalScope scope(&terrainState.syncPoint);
        runtimeInfo.heightMap = id;
        runtimeInfo.lowresGenerated = false;
        runtimeInfo.loadBits |= TerrainRuntimeInfo::HeightMapLoaded;
        terrainState.shadowMapInvalid = true;

        // Если мы используем трассировку лучей, запустите обновление сетки трассировки лучей.
        if (runtimeInfo.enableRayTracing)
        {
            raytracingState.updateMesh = true;

            TerrainMeshGenerate::GenerationConstants generationConstants;
            Math::mat4 transform;
            transform.store(generationConstants.Transform);
            generationConstants.MinHeight = runtimeInfo.minHeight;
            generationConstants.MaxHeight = runtimeInfo.maxHeight;
            generationConstants.VerticesPerPatch = numVertsX * numVertsY;
            generationConstants.WorldSize[0] = runtimeInfo.worldWidth;
            generationConstants.WorldSize[1] = runtimeInfo.worldHeight;
            generationConstants.HeightMap = CoreGraphics::TextureGetBindlessHandle(runtimeInfo.heightMap);
            CoreGraphics::BufferUpdate(raytracingState.constantsBuffer, generationConstants);
        }
    }, nullptr, false, false);
}

//------------------------------------------------------------------------------
/**
*/
TerrainBiomeId 
TerrainContext::CreateBiome(const BiomeSettings& settings)
{
    Ids::Id32 ret = terrainBiomeAllocator.Alloc();
    terrainBiomeAllocator.Set<TerrainBiome_Settings>(ret, settings);

    Util::Array<BiomeMaterial> mats =
    {
        settings.materials[BiomeSettings::BiomeMaterialLayer::Flat],
        settings.materials[BiomeSettings::BiomeMaterialLayer::Slope],
        settings.materials[BiomeSettings::BiomeMaterialLayer::Height],
        settings.materials[BiomeSettings::BiomeMaterialLayer::HeightSlope]
    };
    IndexT biomeIndex = terrainState.biomeCounter;
    for (int i = 0; i < mats.Size(); i++)
    {
        terrainState.biomeLoaded[biomeIndex][i] = 0x0;
        terrainState.biomeLowresGenerated[i] = false;
        Resources::CreateResource(mats[i].albedo.Value(), "terrain", [i, biomeIndex](Resources::ResourceId id)
        {
            Threading::CriticalScope scope(&terrainState.syncPoint);
            CoreGraphics::TextureIdLock _0(id);
            terrainState.biomeMaterials.MaterialAlbedos[biomeIndex][i] = CoreGraphics::TextureGetBindlessHandle(id);
            terrainState.biomeTextures.Append(id);
            terrainState.biomeLowresGenerated[biomeIndex] = false;
            terrainState.biomeLoaded[biomeIndex][i] |= BiomeLoadBits::AlbedoLoaded;
        }, nullptr, false, false);

        Resources::CreateResource(mats[i].normal.Value(), "terrain", [i, biomeIndex](Resources::ResourceId id)
        {
            Threading::CriticalScope scope(&terrainState.syncPoint);
            CoreGraphics::TextureIdLock _0(id);
            terrainState.biomeMaterials.MaterialNormals[biomeIndex][i] = CoreGraphics::TextureGetBindlessHandle(id);
            terrainState.biomeTextures.Append(id);
            terrainState.biomeLowresGenerated[biomeIndex] = false;
            terrainState.biomeLoaded[biomeIndex][i] |= BiomeLoadBits::NormalLoaded;
        }, nullptr, false, false);

        Resources::CreateResource(mats[i].material.Value(), "terrain", [i, biomeIndex](Resources::ResourceId id)
        {
            Threading::CriticalScope scope(&terrainState.syncPoint);
            CoreGraphics::TextureIdLock _0(id);
            terrainState.biomeMaterials.MaterialPBRs[biomeIndex][i] = CoreGraphics::TextureGetBindlessHandle(id);
            terrainState.biomeTextures.Append(id);
            terrainState.biomeLowresGenerated[biomeIndex] = false;
            terrainState.biomeLoaded[biomeIndex][i] |= BiomeLoadBits::MaterialLoaded;
        }, nullptr, false, false);
    }

    Resources::CreateResource(settings.biomeMask, "terrain", [biomeIndex](Resources::ResourceId id)
    {
        Threading::CriticalScope scope(&terrainState.syncPoint);
        CoreGraphics::TextureIdLock _0(id);
        terrainState.biomeMaterials.MaterialMasks[biomeIndex / 4][biomeIndex % 4] = CoreGraphics::TextureGetBindlessHandle(id);
        terrainState.biomeTextures.Append(id);
        terrainState.biomeMasks[terrainState.biomeCounter] = id;
        terrainState.biomeLowresGenerated[biomeIndex] = false;
        for (int i = 0; i < 4; i++)
            terrainState.biomeLoaded[biomeIndex][i] |= BiomeLoadBits::MaskLoaded;
    }, nullptr, false, false);

    if (settings.biomeParameters.useMaterialWeights)
    {
        Resources::CreateResource(settings.biomeParameters.weights, "terrain", [biomeIndex](Resources::ResourceId id)
        {
            Threading::CriticalScope scope(&terrainState.syncPoint);
            CoreGraphics::TextureIdLock _0(id);
            terrainState.biomeMaterials.MaterialWeights[biomeIndex / 4][biomeIndex % 4] = CoreGraphics::TextureGetBindlessHandle(id);
            terrainState.biomeTextures.Append(id);
            terrainState.biomeWeights[terrainState.biomeCounter] = id;
            terrainState.biomeLowresGenerated[biomeIndex] = false;
            for (int i = 0; i < 4; i++)
                terrainState.biomeLoaded[biomeIndex][i] |= BiomeLoadBits::WeightsLoaded;
        }, nullptr, false, false);
    }
    else
    {
        Threading::CriticalScope scope(&terrainState.syncPoint);
        for (int i = 0; i < 4; i++)
            terrainState.biomeLoaded[biomeIndex][i] |= BiomeLoadBits::WeightsLoaded;
    }

    //CoreGraphics::BufferUpdate(terrainState.biomeBuffer, TerrainState.biomeMaterials);
    //TerrainVirtualTileState.updateLowres = true;

    terrainBiomeAllocator.Set<TerrainBiome_Index>(ret, terrainState.biomeCounter);
    terrainState.biomeCounter++;
    return TerrainBiomeId(ret);
}

//------------------------------------------------------------------------------
/**
*/
void 
TerrainContext::SetBiomeSlopeThreshold(TerrainBiomeId id, float threshold)
{
    terrainBiomeAllocator.Get<TerrainBiome_Settings>(id.id).biomeParameters.slopeThreshold = threshold;
}

//------------------------------------------------------------------------------
/**
*/
void 
TerrainContext::SetBiomeHeightThreshold(TerrainBiomeId id, float threshold)
{
    terrainBiomeAllocator.Get<TerrainBiome_Settings>(id.id).biomeParameters.heightThreshold = threshold;
}

//------------------------------------------------------------------------------
/**
*/
void
TerrainContext::SetSun(const Graphics::GraphicsEntityId sun)
{
    terrainState.sun = sun;
}

//------------------------------------------------------------------------------
/**
*/
void 
TerrainContext::CullPatches(const Ptr<Graphics::View>& view, const Graphics::FrameContext& ctx)
{
    N_SCOPE(TerrainRunJobs, Terrain);
    Math::mat4 cameraTransform = Math::inverse(Graphics::CameraContext::GetView(view->GetCamera()));
    terrainVirtualTileState.indirectionUploadOffsets[ctx.bufferIndex] = 0;

    if (raytracingState.setupBlasFrame == ctx.frameIndex)
        raytracingState.terrainSetupCallback();

    n_assert(subtexturesDoneCounter == 0);
    subtexturesDoneCounter = 1;
    terrainVirtualTileState.subTextureNumOutputs = 0;

    SubTextureUpdateJobUniforms uniforms;
    uniforms.maxMip = IndirectionNumMips - 1;
    uniforms.physicalTileSize = PhysicalTextureTileSize;
    uniforms.subTextureWorldSize = SubTextureWorldSize;

    Jobs2::JobDispatch(
        [
            uniforms
            , camera = cameraTransform
        ]
    (SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset)
    {
        N_SCOPE(TerrainSubTextureUpdateJob, Terrain);

        // Перебрать рабочую группу
        for (IndexT i = 0; i < groupSize; i++)
        {
            // Получить индекс элемента
            IndexT index = i + invocationOffset;
            if (index >= totalJobs)
                return;

            const Terrain::SubTexture& subTexture = terrainVirtualTileState.subTextures[index];

            // замаскируйте координату y, умножив результат на 1, 0, 1
            Math::vec4 min = Math::vec4(subTexture.worldCoordinate.x, 0, subTexture.worldCoordinate.y, 0);
            Math::vec4 max = min + Math::vec4(uniforms.subTextureWorldSize, 0.0f, uniforms.subTextureWorldSize, 0.0f);
            Math::vec4 cameraXZ = camera.position * Math::vec4(1, 0, 1, 0);
            Math::vec4 nearestPoint = Math::minimize(Math::maximize(cameraXZ, min), max);
            float distance = length(nearestPoint - cameraXZ);

            // if we are outside the virtual area, just default the resolution to 0
            uint resolution = 0;
            uint lod = 0;
            uint t = 0;
            if (distance > SubTextureRange)
                goto skipResolution;

            // на каждом регулярном интервале расстояний увеличивайте t
            t = Math::max(1.0f, (distance / SubTextureSwapDistance));

            // вычислять lod логарифмически, так что переход к более высоким значениям lod происходит геометрически медленнее
            lod = Math::min((uint)Math::log2(t), uniforms.maxMip);

            // рассчитать разрешение, компенсируя максимальное разрешение с помощью lod
            resolution = SubTextureMaxPixels >> lod;

skipResolution:

            // рассчитать количество плиток, которое представляет собой окончательное разрешение, деленное на размер плитки
            // максимальное значение — maxResolution, а наименьшее — 1.
            uint tiles = resolution / uniforms.physicalTileSize;

            // заботьтесь только о подтекстурах с минимум 4 тайлами
            //плитки = плитки >= 4 ? плитки: 0;

            SubTextureUpdateState state = SubTextureUpdateState::NoChange;

            // установить состояние
            if (tiles >= 1)
            {
                if (subTexture.numTiles == 0)
                    state = SubTextureUpdateState::Created;
                else if (tiles > subTexture.numTiles)
                    state = SubTextureUpdateState::Grew;
                else if (tiles < subTexture.numTiles)
                    state = SubTextureUpdateState::Shrank;
            }
            else
            {
                // if tiles is now below the limit but it used to be bigger, delete 
                if (subTexture.numTiles > 0)
                    state = SubTextureUpdateState::Deleted;
            }

            if (state != SubTextureUpdateState::NoChange)
            {
                // В случае изменения выведите результат
                int outputIndex = Threading::Interlocked::Add(&terrainVirtualTileState.subTextureNumOutputs, 1);
                n_assert(outputIndex < SubTextureMaxUpdates);

                SubTextureUpdateJobOutput& output = terrainVirtualTileState.subTextureJobOutputs[outputIndex];
                output.index = index;
                output.oldMaxMip = subTexture.maxMip;
                output.oldTiles = subTexture.numTiles;
                output.oldCoord = subTexture.indirectionOffset;

                output.newMaxMip = tiles > 0 ? Math::log2(tiles) : 0;
                output.newTiles = tiles;

                output.mipBias = output.newMaxMip > output.oldMaxMip ? output.newMaxMip - output.oldMaxMip : 0.0f;

                // по умолчанию подтекстура не менялась
                output.updateState = state;
            }

        }
    }, terrainVirtualTileState.subTextures.Size(), 256, {}, &subtexturesDoneCounter, &subtexturesFinishedEvent);

    n_assert(sectionCullDoneCounter == 0);
    sectionCullDoneCounter = 1;  
    const Math::mat4& viewProj = Graphics::CameraContext::GetViewProjection(view->GetCamera());
    Util::Array<TerrainRuntimeInfo>& runtimes = terrainAllocator.GetArray<Terrain_RuntimeInfo>();

    Math::vec4 m_col_x[4];
    Math::vec4 m_col_y[4];
    Math::vec4 m_col_z[4];
    Math::vec4 m_col_w[4];
    m_col_x[0] = Math::splat_x(viewProj.r[0]);
    m_col_x[1] = Math::splat_x(viewProj.r[1]);
    m_col_x[2] = Math::splat_x(viewProj.r[2]);
    m_col_x[3] = Math::splat_x(viewProj.r[3]);

    m_col_y[0] = Math::splat_y(viewProj.r[0]);
    m_col_y[1] = Math::splat_y(viewProj.r[1]);
    m_col_y[2] = Math::splat_y(viewProj.r[2]);
    m_col_y[3] = Math::splat_y(viewProj.r[3]);

    m_col_z[0] = Math::splat_z(viewProj.r[0]);
    m_col_z[1] = Math::splat_z(viewProj.r[1]);
    m_col_z[2] = Math::splat_z(viewProj.r[2]);
    m_col_z[3] = Math::splat_z(viewProj.r[3]);

    m_col_w[0] = Math::splat_w(viewProj.r[0]);
    m_col_w[1] = Math::splat_w(viewProj.r[1]);
    m_col_w[2] = Math::splat_w(viewProj.r[2]);
    m_col_w[3] = Math::splat_w(viewProj.r[3]);

    terrainVirtualTileState.numPatchesThisFrame = 0;

    sectionCullDoneCounter = runtimes.Size();
    for (IndexT i = 0; i < runtimes.Size(); i++)
    {
        TerrainRuntimeInfo& rt = runtimes[i];
        Jobs2::JobDispatch(
            [
                boundingBoxes = rt.sectionBoxes.ConstBegin()
                , visibilities = rt.sectorVisible.Begin()
                , instanceCounter = &terrainVirtualTileState.numPatchesThisFrame
                , patchData = (Terrain::TerrainPatch*)CoreGraphics::BufferMap(terrainVirtualTileState.patchConstants.buffers[ctx.bufferIndex])
                , m_col_x
                , m_col_y
                , m_col_z
                , m_col_w
            ]
        (SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset)
        {
            N_SCOPE(BruteforceViewFrustumCulling, Visibility);

            // Перебрать рабочую группу
            for (IndexT i = 0; i < groupSize; i++)
            {
                // Получить индекс элемента
                IndexT index = i + invocationOffset;
                if (index >= totalJobs)
                    return;

                const Math::ClipStatus::Type clip = boundingBoxes[index].clipstatus(m_col_x, m_col_y, m_col_z, m_col_w);
                if (clip != Math::ClipStatus::Outside)
                {
                    uint offset = Threading::Interlocked::Add(instanceCounter, 1);

                    Terrain::TerrainPatch patch;
                    patch.PosOffset[0] = boundingBoxes[index].pmin.x;
                    patch.PosOffset[1] = boundingBoxes[index].pmin.z;
                    patch.UvOffset[0] = 0.0f;
                    patch.UvOffset[1] = 0.0f;
                    patchData[offset] = patch;
                    visibilities[index] = true;
                }
                else
                {
                    visibilities[index] = false;
                }
            }
        }, rt.sectionBoxes.Size(), 256, {}, &sectionCullDoneCounter, &sectionCullFinishedEvent);
        CoreGraphics::BufferUnmap(terrainVirtualTileState.patchConstants.buffers[ctx.bufferIndex]);
    }
    if (runtimes.IsEmpty())
        sectionCullFinishedEvent.Signal();
}

//------------------------------------------------------------------------------
/**
    Unpack from packed ushort vectors to full size
*/
void
UnpackPageDataEntry(uint* packed, uint& status, uint& subTextureIndex, uint& mip, uint& maxMip, uint& subTextureTileX, uint& subTextureTileY)
{
    status = packed[0] & 0x3;
    subTextureIndex = packed[0] >> 2;
    mip = packed[1] & 0xF;
    maxMip = (packed[1] >> 4) & 0xF;
    subTextureTileX = (packed[1] >> 8) & 0xFF;
    subTextureTileY = (packed[1] >> 16) & 0xFF;

    // Удалите эти
    n_assert(subTextureTileX == packed[2]);
    n_assert(subTextureTileY == packed[3]);
}

//------------------------------------------------------------------------------
/**
*/
template<typename T>
uint
Upload(T* data, uint size, uint alignment)
{
    uint& offset = terrainVirtualTileState.indirectionUploadOffsets[CoreGraphics::GetBufferedFrameIndex()];
    uint uploadOffset = Math::align(offset, alignment);
    CoreGraphics::BufferUpdateArray(terrainVirtualTileState.indirectionUploadBuffers.Buffer(), data, size, offset);
    offset = uploadOffset + size * sizeof(T);
    return uploadOffset;
}

//------------------------------------------------------------------------------
/**
*/
void
IndirectionUpdate(
    uint mip
    , uint physicalOffsetX
    , uint physicalOffsetY
    , uint indirectionOffsetX
    , uint indirectionOffsetY
    , uint subTextureTileX
    , uint subTextureTileY
)
{
    IndirectionEntry entry;
    entry.mip = mip;
    entry.physicalOffsetX = physicalOffsetX;
    entry.physicalOffsetY = physicalOffsetY;

    // Вычислить пиксель косвенности в подтекстуре
    uint indirectionPixelX = (indirectionOffsetX >> mip) + subTextureTileX;
    uint indirectionPixelY = (indirectionOffsetY >> mip) + subTextureTileY;

    terrainVirtualTileState.indirectionEntryUpdates.Append(entry);
    terrainVirtualTileState.indirectionTextureCopies.Append(CoreGraphics::TextureCopy{ Math::rectangle<SizeT>(indirectionPixelX, indirectionPixelY, indirectionPixelX + 1, indirectionPixelY + 1), (uint)mip, 0 });

    uint mipOffset = terrainVirtualTileState.indirectionMipOffsets[mip];
    uint size = terrainVirtualTileState.indirectionMipSizes[mip];
    terrainVirtualTileState.indirectionBuffer[mipOffset + indirectionPixelX + indirectionPixelY * size] = entry;
}

//------------------------------------------------------------------------------
/**
*/
void
IndirectionErase(
    uint mip
    , uint indirectionOffsetX
    , uint indirectionOffsetY
    , uint subTextureTileX
    , uint subTextureTileY
)
{
    IndirectionEntry entry{ 0xF, 0x3FFF, 0x3FFF };

    // Вычислить пиксель косвенности в подтекстуре
    uint indirectionPixelX = (indirectionOffsetX >> mip) + subTextureTileX;
    uint indirectionPixelY = (indirectionOffsetY >> mip) + subTextureTileY;

    terrainVirtualTileState.indirectionEntryUpdates.Append(entry);
    terrainVirtualTileState.indirectionTextureCopies.Append(CoreGraphics::TextureCopy{ Math::rectangle<SizeT>(indirectionPixelX, indirectionPixelY, indirectionPixelX + 1, indirectionPixelY + 1), (uint)mip, 0 });

    uint mipOffset = terrainVirtualTileState.indirectionMipOffsets[mip];
    uint size = terrainVirtualTileState.indirectionMipSizes[mip];
    terrainVirtualTileState.indirectionBuffer[mipOffset + indirectionPixelX + indirectionPixelY * size] = entry;
}

//------------------------------------------------------------------------------
/**
    Copies mip chain from old region to new region which is bigger by mapping mips 0..X to 1..X in the new region
*/
void
IndirectionMoveGrow(
    uint oldMaxMip
    , uint oldTiles
    , const Math::uint2& oldCoord
    , uint newMaxMip
    , uint newTiles
    , const Math::uint2& newCoord)
{
    // Рассчитайте разницу в mips, которая является нашим смещением mip.
    uint mipDiff = newMaxMip - oldMaxMip;

    // Если подтекст разросся, скопируйте мипчейн старого раздела в новый раздел, сместив его на разницу в мипах.
    // Lod — уровень MIP для нового тайла.
    for (uint i = 0; i <= oldMaxMip; i++)
    {
        // Нам нужно переназначить все старые тексели косвенного направления подтекстуры на новую область.
        // Для сдвига 64x64 -> 256x256 это будет означать отображение mips.
        // [0, 1, 2, 3, ...] -> [2, 3, 4, 5, ...] 
        uint newMip = i + mipDiff;
        uint width = oldTiles >> i;
        n_assert(newTiles >> newMip == width);
        uint dataSize = width * sizeof(IndirectionEntry);

        // newCoord и oldCoord находятся в соответствующем базисе mip 0, поэтому нам нужно их сместить.
        // чтобы получить правильные координаты
        Math::uint2 mippedNewCoord{ newCoord.x >> newMip, newCoord.y >> newMip };
        Math::uint2 mippedOldCoord{ oldCoord.x >> i, oldCoord.y >> i };

        terrainVirtualTileState.indirectionTextureFromCopiesThisFrame.Append(CoreGraphics::TextureCopy{ Math::rectangle<SizeT>(mippedOldCoord.x, mippedOldCoord.y, mippedOldCoord.x + width, mippedOldCoord.y + width), i, 0 });
        terrainVirtualTileState.indirectionTextureToCopiesThisFrame.Append(CoreGraphics::TextureCopy{ Math::rectangle<SizeT>(mippedNewCoord.x, mippedNewCoord.y, mippedNewCoord.x + width, mippedNewCoord.y + width), newMip, 0 });

        // Обновить буфер ЦП
        uint fromMipOffset = terrainVirtualTileState.indirectionMipOffsets[i];
        uint fromSize = terrainVirtualTileState.indirectionMipSizes[i];
        uint fromStart = mippedOldCoord.x + mippedOldCoord.y * fromSize;
        uint toMipOffset = terrainVirtualTileState.indirectionMipOffsets[newMip];
        uint toSize = terrainVirtualTileState.indirectionMipSizes[newMip];
        uint toStart = mippedNewCoord.x + mippedNewCoord.y * toSize;

        for (uint j = 0; j < width; j++)
        {
            uint fromRowOffset = fromSize * j;
            uint toRowOffset = toSize * j;
            memmove(terrainVirtualTileState.indirectionBuffer.Begin() + toMipOffset + toRowOffset + toStart, terrainVirtualTileState.indirectionBuffer.Begin() + fromMipOffset + fromRowOffset + fromStart, dataSize);
        }
    }
}

//------------------------------------------------------------------------------
/**
    Копирует цепочку mip из старого региона в новый регион меньшего размера, сопоставляя mips 0..X с 0..X-1 в новом регионе.
*/
void
IndirectionMoveShrink(
    uint oldMaxMip
    , uint oldTiles
    , const Math::uint2& oldCoord
    , uint newMaxMip
    , uint newTiles
    , const Math::uint2& newCoord)
{
    // Если мы уменьшимся, старые MIPS должны быть выше, поэтому это должно быть наше смещение смещения MIP.
    uint mipDiff = oldMaxMip - newMaxMip;

    // Поскольку мы сжимаемся, нам нужно просмотреть новые мипы и скопировать старые данные.
    for (uint i = 0; i <= newMaxMip; i++)
    {
        // Переназначение старой большей подтекстуры на новую меньшую требует от нас переноса копии из источника в следующие mips.
        // Для сдвига 256x256 -> 64x64 это будет означать копирование mips.
        // [2, 3, 4, 5, ...] -> [0, 1, 2, 3, ...]
        uint oldMip = mipDiff + i;
        uint width = newTiles >> i;
        n_assert(oldTiles >> oldMip == width);
        uint dataSize = width * sizeof(IndirectionEntry);

        // newCoord и oldCoord находятся в соответствующем базисе mip 0, поэтому нам нужно их сместить.
        // чтобы получить правильные координаты
        Math::uint2 mippedNewCoord{ newCoord.x >> i, newCoord.y >> i };
        Math::uint2 mippedOldCoord{ oldCoord.x >> oldMip, oldCoord.y >> oldMip };

        terrainVirtualTileState.indirectionTextureFromCopiesThisFrame.Append(CoreGraphics::TextureCopy{ Math::rectangle<SizeT>(mippedOldCoord.x, mippedOldCoord.y, mippedOldCoord.x + width, mippedOldCoord.y + width), oldMip, 0 });
        terrainVirtualTileState.indirectionTextureToCopiesThisFrame.Append(CoreGraphics::TextureCopy{ Math::rectangle<SizeT>(mippedNewCoord.x, mippedNewCoord.y, mippedNewCoord.x + width, mippedNewCoord.y + width), i, 0 });

        // Обновить буфер ЦП
        uint fromMipOffset = terrainVirtualTileState.indirectionMipOffsets[oldMip];
        uint fromSize = terrainVirtualTileState.indirectionMipSizes[oldMip];
        uint fromStart = mippedOldCoord.x + mippedOldCoord.y * fromSize;
        uint toMipOffset = terrainVirtualTileState.indirectionMipOffsets[i];
        uint toSize = terrainVirtualTileState.indirectionMipSizes[i];
        uint toStart = mippedNewCoord.x + mippedNewCoord.y * toSize;

        for (uint j = 0; j < width; j++)
        {
            uint fromRowOffset = fromSize * j;
            uint toRowOffset = toSize * j;
            memmove(terrainVirtualTileState.indirectionBuffer.Begin() + toMipOffset + toRowOffset + toStart, terrainVirtualTileState.indirectionBuffer.Begin() + fromMipOffset + fromRowOffset + fromStart, dataSize);
        }
    }
}

//------------------------------------------------------------------------------
/**
    Очистить старый регион
*/
void
IndirectionClear(
    uint mips
    , uint tiles
    , const Math::uint2& coord)
{
    uint width = tiles;
    uint dataSize = width * width * sizeof(IndirectionEntry);

    // Возьмите самый большой MIP и заполните его 0 пикселей.
    Util::FixedArray<IndirectionEntry> pixels(width * width);
    pixels.Fill(IndirectionEntry{ 0xF, 0x3FFF, 0x3FFF });

    // Загрузите в графический процессор, наименьший MIP будет иметь достаточно значений, чтобы охватить все MIPS.
    uint offset = Upload(pixels.Begin(), pixels.Size(), 4);

    // пройти старый регион и сбросить пиксели косвенности
    for (uint i = 0; i <= mips; i++)
    {
        Math::uint2 mippedCoord{ coord.x >> i, coord.y >> i };
        uint width = tiles >> i;

        terrainVirtualTileState.indirectionBufferClearsThisFrame.Append(CoreGraphics::BufferCopy{ static_cast<uint>(offset) });
        terrainVirtualTileState.indirectionTextureClearsThisFrame.Append(CoreGraphics::TextureCopy{ Math::rectangle<SizeT>(mippedCoord.x, mippedCoord.y, mippedCoord.x + width, mippedCoord.y + width), i, 0 });

        // Обновить буфер ЦП
        uint mipOffset = terrainVirtualTileState.indirectionMipOffsets[i];
        uint rowSize = terrainVirtualTileState.indirectionMipSizes[i];
        uint rowStart = mippedCoord.x + mippedCoord.y * rowSize;

        for (uint j = 0; j < width; j++)
        {
            uint rowOffset = rowSize * j;
            memcpy(terrainVirtualTileState.indirectionBuffer.Begin() + mipOffset + rowOffset + rowStart, pixels.Begin(), width * sizeof(IndirectionEntry));
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
TerrainContext::UpdateLOD(const Ptr<Graphics::View>& view, const Graphics::FrameContext& ctx)
{
    N_SCOPE(TerrainUpdateVirtualTexturing, Terrain);
    Util::Array<TerrainRuntimeInfo>& runtimes = terrainAllocator.GetArray<Terrain_RuntimeInfo>();

    Math::mat4 sunTransform = Lighting::LightContext::GetTransform(terrainState.sun);
    if (terrainState.cachedSunDirection != sunTransform.z_axis)
    {
        Threading::CriticalScope scope(&terrainState.syncPoint);
        terrainState.shadowMapInvalid = true;
        terrainState.cachedSunDirection = sunTransform.z_axis;
    }

    Terrain::TerrainSystemUniforms systemUniforms;
    systemUniforms.TerrainPosBuffer = CoreGraphics::TextureGetBindlessHandle(FrameScript_default::Texture_TerrainPosBuffer());
    systemUniforms.MinLODDistance = 0.0f;
    systemUniforms.MaxLODDistance = terrainState.mipLoadDistance;
    systemUniforms.VirtualLodDistance = terrainState.mipLoadDistance;
    systemUniforms.MinTessellation = 1.0f;
    systemUniforms.MaxTessellation = 32.0f;
    systemUniforms.Debug = terrainState.debugRender;
    systemUniforms.NumBiomes = terrainState.biomeCounter;
    systemUniforms.AlbedoPhysicalCacheBuffer = CoreGraphics::TextureGetBindlessHandle(terrainVirtualTileState.physicalAlbedoCacheBC);
    systemUniforms.NormalPhysicalCacheBuffer = CoreGraphics::TextureGetBindlessHandle(terrainVirtualTileState.physicalNormalCacheBC);
    systemUniforms.MaterialPhysicalCacheBuffer = CoreGraphics::TextureGetBindlessHandle(terrainVirtualTileState.physicalMaterialCacheBC);
    systemUniforms.AlbedoLowresBuffer = CoreGraphics::TextureGetBindlessHandle(terrainVirtualTileState.lowresAlbedo);
    systemUniforms.NormalLowresBuffer = CoreGraphics::TextureGetBindlessHandle(terrainVirtualTileState.lowresNormal);
    systemUniforms.MaterialLowresBuffer = CoreGraphics::TextureGetBindlessHandle(terrainVirtualTileState.lowresMaterial);
    systemUniforms.IndirectionBuffer = CoreGraphics::TextureGetBindlessHandle(terrainVirtualTileState.indirectionTexture);

    bool biomesLoaded = true;
    for (IndexT j = 0; j < terrainState.biomeCounter; j++)
    {
        Threading::CriticalScope scope(&terrainState.syncPoint);
        BiomeParameters settings = terrainBiomeAllocator.Get<TerrainBiome_Settings>(j).biomeParameters;
        for (IndexT i = 0; i < 4; i++)
        {
            bool allBitsLoaded = AllBits(terrainState.biomeLoaded[j][i], BiomeLoadBits::AlbedoLoaded | BiomeLoadBits::NormalLoaded | BiomeLoadBits::MaterialLoaded | BiomeLoadBits::MaskLoaded | BiomeLoadBits::WeightsLoaded);
            biomesLoaded &= allBitsLoaded;
        }
        if (!terrainState.biomeLowresGenerated[j] && biomesLoaded)
        {
            terrainVirtualTileState.updateLowres = true;
            terrainState.biomeLowresGenerated[j] = true;
            CoreGraphics::BufferUpdate(terrainState.biomeBuffer, terrainState.biomeMaterials);
        }
        
        CoreGraphics::TextureId biomeMask = terrainState.biomeMasks[j];
        if (biomeMask != CoreGraphics::InvalidTextureId)
        {
            systemUniforms.BiomeRules[j][3] = CoreGraphics::TextureGetNumMips(terrainState.biomeMasks[j]);
        }
        systemUniforms.BiomeRules[j][0] = settings.slopeThreshold;
        systemUniforms.BiomeRules[j][1] = settings.heightThreshold;
        systemUniforms.BiomeRules[j][2] = settings.uvScaleFactor;
    }
    BufferUpdate(terrainState.systemConstants, systemUniforms, 0);

    struct PendingDelete
    {
        uint oldMaxMip;
        uint oldTiles;
        Math::uint2 oldCoord;
    };

    // Подождите, пока работа над подтекстурами завершит этот кадр.
    subtexturesFinishedEvent.Wait();

    for (IndexT i = 0; i < terrainVirtualTileState.subTextureNumOutputs; i++)
    {
        const Terrain::SubTextureUpdateJobOutput& output = terrainVirtualTileState.subTextureJobOutputs[i];
        Terrain::SubTexture& subTex = terrainVirtualTileState.subTextures[output.index];
        Terrain::TerrainSubTexture& compressedSubTex = terrainVirtualTileState.gpuSubTextures[output.index];

        switch (output.updateState)
        {
            case SubTextureUpdateState::Grew:
            case SubTextureUpdateState::Shrank:
            case SubTextureUpdateState::Created:
            {
                Math::uint2 newCoord = terrainVirtualTileState.indirectionOccupancy.Allocate(output.newTiles);
                if (newCoord.x == 0xFFFFFFFF || newCoord.y == 0xFFFFFFFF)
                    break;
                //n_assert(newCoord.x != 0xFFFFFFFF && newCoord.y != 0xFFFFFFFF);

                // обновить подтекстуру
                subTex.numTiles = output.newTiles;
                subTex.indirectionOffset = newCoord;
                subTex.maxMip = output.newMaxMip;
                terrainVirtualTileState.virtualSubtextureBufferUpdate = true;
                switch (output.updateState)
                {
                    case SubTextureUpdateState::Grew:
                        IndirectionClear(output.newMaxMip - output.oldMaxMip, output.newTiles, newCoord);
                        IndirectionMoveGrow(output.oldMaxMip, output.oldTiles, output.oldCoord, output.newMaxMip, output.newTiles, newCoord);
                        terrainVirtualTileState.indirectionOccupancy.Deallocate(output.oldCoord, output.oldTiles);
                        break;
                    case SubTextureUpdateState::Shrank:
                        IndirectionMoveShrink(output.oldMaxMip, output.oldTiles, output.oldCoord, output.newMaxMip, output.newTiles, newCoord);
                        terrainVirtualTileState.indirectionOccupancy.Deallocate(output.oldCoord, output.oldTiles);
                        break;
                    case SubTextureUpdateState::Created:
                        IndirectionClear(output.newMaxMip, output.newTiles, newCoord);
                        break;
                    default: break;
                }
                PackSubTexture(subTex, compressedSubTex);
                break;
            }
            case SubTextureUpdateState::Deleted:
                terrainVirtualTileState.indirectionOccupancy.Deallocate(output.oldCoord, output.oldTiles);
                subTex.numTiles = 0;
                subTex.indirectionOffset.x = 0xFFFFFFFF;
                subTex.indirectionOffset.y = 0xFFFFFFFF;
                subTex.maxMip = 0;
                terrainVirtualTileState.virtualSubtextureBufferUpdate = true;
                PackSubTexture(subTex, compressedSubTex);
                break;
            default:
                break;
        }
    }

    if (biomesLoaded)
    {
        // Обработка обратного чтения с графического процессора
        CoreGraphics::BufferInvalidate(terrainVirtualTileState.pageUpdateReadbackBuffers.buffers[ctx.bufferIndex]);
        Terrain::PageUpdateList* updateList = (Terrain::PageUpdateList*)CoreGraphics::BufferMap(terrainVirtualTileState.pageUpdateReadbackBuffers.buffers[ctx.bufferIndex]);
        terrainVirtualTileState.numPixels = updateList->NumEntries;
        n_assert(terrainVirtualTileState.numPixels < Terrain::MAX_PAGE_UPDATES);
        for (uint i = 0; i < updateList->NumEntries; i++)
        {
            uint status, subTextureIndex, mip, maxMip, subTextureTileX, subTextureTileY;
            UnpackPageDataEntry(updateList->Entry[i], status, subTextureIndex, mip, maxMip, subTextureTileX, subTextureTileY);

            // состояние обновления равно 1, если страница выделена, или 2, если раньше она была выделена, но с тех пор была освобождена
            uint updateState = status;
            SubTexture& subTexture = terrainVirtualTileState.subTextures[subTextureIndex];

            if (subTexture.numTiles == 0)
                continue;

            n_assert(mip < 0xF);
            n_assert(subTextureTileX < PhysicalTextureTileSize);
            n_assert(subTextureTileY < PhysicalTextureTileSize);

            // Если подтекстура изменила размер, просто проигнорируйте это обновление.
            if (subTexture.maxMip != maxMip)
                continue;

            TileCacheEntry cacheEntry;
            cacheEntry.entry.tileX = subTextureTileX;
            cacheEntry.entry.tileY = subTextureTileY;
            cacheEntry.entry.tiles = subTexture.numTiles >> mip;
            cacheEntry.entry.subTextureIndex = subTextureIndex;

            // Получите новые или кэшированные координаты из виртуального кэша текстур.
            TextureTileCache::CacheResult result = terrainVirtualTileState.physicalTextureTileCache.Cache(cacheEntry);
            if (result.didCache)
            {
                IndirectionUpdate(mip, result.cached.x, result.cached.y, subTexture.indirectionOffset.x, subTexture.indirectionOffset.y, subTextureTileX, subTextureTileY);

                // Рассчитайте некоторые показатели: метры на плитку, метры на пиксель, а затем дополненные метры на плитку.
                float metersPerTile = SubTextureWorldSize / float(subTexture.numTiles >> mip);
                float metersPerPixel = metersPerTile / float(PhysicalTextureTilePaddedSize);
                float metersPerTilePadded = metersPerTile + PhysicalTextureTilePadding * metersPerPixel;

                TerrainTileWrite::TileWrite write;
                write.WorldOffset[0] = subTexture.worldCoordinate.x + subTextureTileX * metersPerTile - metersPerPixel * PhysicalTextureTileHalfPadding;
                write.WorldOffset[1] = subTexture.worldCoordinate.y + subTextureTileY * metersPerTile - metersPerPixel * PhysicalTextureTileHalfPadding;
                n_assert(result.cached.x < USHRT_MAX && result.cached.y < USHRT_MAX);
                write.WriteOffset_MetersPerTile[0] = (result.cached.x & 0xFFFF) | ((result.cached.y & 0xFFFF) << 16);
                write.WriteOffset_MetersPerTile[1] = *reinterpret_cast<int*>(&metersPerTilePadded);

                terrainVirtualTileState.tileWrites.Append(write);
            }
            else
            {
                // Вычислить пиксель косвенности в подтекстуре
                uint indirectionPixelX = (subTexture.indirectionOffset.x >> mip) + subTextureTileX;
                uint indirectionPixelY = (subTexture.indirectionOffset.y >> mip) + subTextureTileY;
                uint mipOffset = terrainVirtualTileState.indirectionMipOffsets[mip];
                uint mipSize = terrainVirtualTileState.indirectionMipSizes[mip];

                // Если в кеше есть плитка, но косвенность очищена, выполните обновление только для косвенности.
                const IndirectionEntry& entry = terrainVirtualTileState.indirectionBuffer[mipOffset + indirectionPixelX + indirectionPixelY * mipSize];
                if (entry.mip == 0xF)
                    IndirectionUpdate(mip, result.cached.x, result.cached.y, subTexture.indirectionOffset.x, subTexture.indirectionOffset.y, subTextureTileX, subTextureTileY);
            }
        }
        CoreGraphics::BufferUnmap(terrainVirtualTileState.pageUpdateReadbackBuffers.buffers[ctx.bufferIndex]);

        IndexT i;

        // Константы настройки для обновлений плиток
        SizeT numPagesThisFrame = Math::min(TerrainTileWrite::MAX_TILES_PER_FRAME, terrainVirtualTileState.tileWrites.Size());
        for (i = 0; i < numPagesThisFrame; i++)
        {
            TerrainTileWrite::TileWrite& write = terrainVirtualTileState.tileWrites[i];
            terrainVirtualTileState.tileWritesThisFrame.Append(write);
        }
        if (i > 0)
        {
            terrainVirtualTileState.tileWrites.EraseRange(0, numPagesThisFrame - 1);
        }
        if (!terrainVirtualTileState.tileWritesThisFrame.IsEmpty())
            CoreGraphics::BufferUpdateArray(terrainVirtualTileState.tileWriteBufferSet.HostBuffer(), terrainVirtualTileState.tileWritesThisFrame);

        // Обновить буферы для косвенной загрузки пикселей.
        numPagesThisFrame = Math::min(TerrainTileWrite::MAX_TILES_PER_FRAME, terrainVirtualTileState.indirectionEntryUpdates.Size());
        uint offset = Upload(terrainVirtualTileState.indirectionEntryUpdates.Begin(), terrainVirtualTileState.indirectionEntryUpdates.Size(), 4);
        for (i = 0; i < numPagesThisFrame; i++)
        {
            // настройка косвенного обновления
            terrainVirtualTileState.indirectionBufferUpdatesThisFrame.Append(CoreGraphics::BufferCopy{ static_cast<uint>(offset + i * sizeof(Terrain::IndirectionEntry)) });
            terrainVirtualTileState.indirectionTextureUpdatesThisFrame.Append(terrainVirtualTileState.indirectionTextureCopies[i]);
        }
        if (i > 0)
        {
            terrainVirtualTileState.indirectionEntryUpdates.EraseRange(0, numPagesThisFrame);
            terrainVirtualTileState.indirectionTextureCopies.EraseRange(0, numPagesThisFrame);
        }

        // Очистить буфер загрузки
        CoreGraphics::BufferFlush(terrainVirtualTileState.indirectionUploadBuffers.buffers[ctx.bufferIndex], 0, terrainVirtualTileState.indirectionUploadOffsets[ctx.bufferIndex]);
    }

    // Подождите, пока задания закончатся
    sectionCullFinishedEvent.Wait();

    if (terrainVirtualTileState.virtualSubtextureBufferUpdate)
    {
        auto bla = reinterpret_cast<SubTextureCompressed*>(terrainVirtualTileState.gpuSubTextures.Begin());
        BufferUpdateArray(
            terrainVirtualTileState.subtextureStagingBuffers.buffers[ctx.bufferIndex],
            terrainVirtualTileState.gpuSubTextures.Begin(),
            terrainVirtualTileState.gpuSubTextures.Size());
    }

    // Настройка униформы для участков местности
    for (IndexT i = 0; i < runtimes.Size(); i++)
    {
        TerrainRuntimeInfo& rt = runtimes[i];

        TerrainRuntimeUniforms uniforms;
        Math::mat4().store(uniforms.Transform);
        uniforms.DecisionMap = TextureGetBindlessHandle(CoreGraphics::TextureId(rt.decisionMap));
        uniforms.HeightMap = TextureGetBindlessHandle(CoreGraphics::TextureId(rt.heightMap));

        CoreGraphics::TextureDimensions dims = CoreGraphics::TextureGetDimensions(rt.heightMap);
        CoreGraphics::TextureDimensions dataBufferDims = CoreGraphics::TextureGetDimensions(FrameScript_default::Texture_TerrainPosBuffer());
        uniforms.VirtualTerrainTextureSize[0] = dims.width;
        uniforms.VirtualTerrainTextureSize[1] = dims.height;
        uniforms.VirtualTerrainTextureSize[2] = 1.0f / dims.width;
        uniforms.VirtualTerrainTextureSize[3] = 1.0f / dims.height;
        uniforms.MaxHeight = rt.maxHeight;
        uniforms.MinHeight = rt.minHeight;
        uniforms.WorldSizeX = rt.worldWidth;
        uniforms.WorldSizeZ = rt.worldHeight;
        uniforms.DataBufferSize[0] = dataBufferDims.width;
        uniforms.DataBufferSize[1] = dataBufferDims.height;
        uniforms.NumTilesX = rt.numTilesX;
        uniforms.NumTilesY = rt.numTilesY;
        uniforms.TileWidth = rt.tileWidth;
        uniforms.TileHeight = rt.tileHeight;
        uniforms.VirtualTerrainPageSize[0] = 64.0f;
        uniforms.VirtualTerrainPageSize[1] = 64.0f;
        uniforms.VirtualTerrainSubTextureSize[0] = SubTextureWorldSize;
        uniforms.VirtualTerrainSubTextureSize[1] = SubTextureWorldSize;
        uniforms.VirtualTerrainNumSubTextures[0] = terrainState.settings.worldSizeX / SubTextureWorldSize;
        uniforms.VirtualTerrainNumSubTextures[1] = terrainState.settings.worldSizeZ / SubTextureWorldSize;
        uniforms.PhysicalInvPaddedTextureSize = 1.0f / PhysicalTexturePaddedSize;
        uniforms.PhysicalTileSize = PhysicalTextureTileSize;
        uniforms.PhysicalTilePaddedSize = PhysicalTextureTilePaddedSize;
        uniforms.PhysicalTilePadding = PhysicalTextureTileHalfPadding;

        CoreGraphics::TextureDimensions indirectionDims = CoreGraphics::TextureGetDimensions(terrainVirtualTileState.indirectionTexture);
        uniforms.VirtualTerrainNumPages[0] = indirectionDims.width;
        uniforms.VirtualTerrainNumPages[1] = indirectionDims.height;
        uniforms.VirtualTerrainNumMips = IndirectionNumMips;

        uniforms.LowresResolution[0] = uniforms.LowresResolution[1] = LowresFallbackSize;
        uniforms.LowresNumMips = LowresFallbackMips - 1;

        uniforms.IndirectionResolution[0] = uniforms.IndirectionResolution[1] = IndirectionTextureSize;
        uniforms.IndirectionNumMips = IndirectionNumMips - 1;

        uniforms.LowresFadeStart = SubTextureFadeStart * SubTextureFadeStart;
        uniforms.LowresFadeDistance = 1.0f / ((SubTextureRange - SubTextureFadeStart) * (SubTextureRange - SubTextureFadeStart));

        for (SizeT j = 0; j < terrainVirtualTileState.indirectionMipOffsets.Size(); j++)
        {
            uniforms.VirtualPageBufferMipOffsets[j / 4][j % 4] = terrainVirtualTileState.indirectionMipOffsets[j];
            uniforms.VirtualPageBufferMipSizes[j / 4][j % 4] = terrainVirtualTileState.indirectionMipSizes[j];
        }
        uniforms.VirtualPageBufferNumPages = CoreGraphics::BufferGetSize(terrainVirtualTileState.pageStatusBuffer);

        BufferUpdate(terrainVirtualTileState.runtimeConstants, uniforms, 0);
    }

    if (terrainState.shadowMapInvalid)
        terrainState.updateShadowMap = true;
}


//------------------------------------------------------------------------------
/**
*/
bool
TerrainContext::GetVisible()
{
    return terrainState.renderToggle;
}

//------------------------------------------------------------------------------
/**
*/
void
TerrainContext::SetVisible(bool visible)
{
    terrainState.renderToggle = visible;
}

//------------------------------------------------------------------------------
/**
*/
void 
TerrainContext::RenderUI(const Graphics::FrameContext& ctx)
{
    if (Core::CVarReadInt(Core::CVarGet("r_terrain_debug")) > 0)
    {
        if (ImGui::Begin("Отладка ландшафта 2"))
        {
            ImGui::SetWindowSize(ImVec2(240, 400), ImGuiCond_Once);
            ImGui::Checkbox("Отладка рендеринга", &terrainState.debugRender);
            ImGui::Checkbox("Render", &terrainState.renderToggle);
            ImGui::LabelText("Updates", "Количество обновлений %d", terrainVirtualTileState.numPixels);

            {
                ImGui::Text("Квадродерево занятости текстуры косвенности");
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 start = ImGui::GetCursorScreenPos();
                ImVec2 fullSize = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y);
                drawList->PushClipRect(
                    ImVec2{ start.x, start.y },
                    ImVec2{ Math::max(start.x + fullSize.x, start.x + 512.0f), Math::min(start.y + fullSize.y, start.y + 512.0f) }, true);

                terrainVirtualTileState.indirectionOccupancy.DebugRender(drawList, start, 0.25f);
                drawList->PopClipRect();

                // установите курсор назад, чтобы мы могли нарисовать нашу рамку
                ImGui::SetCursorScreenPos(start);
                ImGui::InvisibleButton("Квадродерево занятости текстуры косвенности", ImVec2(512.0f, 512.0f));
            }

            {
                /*
                ImGui::Text("Physical texture occupancy quadtree");
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 start = ImGui::GetCursorScreenPos();
                ImVec2 fullSize = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y);
                drawList->PushClipRect(
                    ImVec2{ start.x, start.y },
                    ImVec2{ Math::max(start.x + fullSize.x, start.x + 512.0f), Math::min(start.y + fullSize.y, start.y + 512.0f) }, true);

                terrainVirtualTileState.physicalTextureTileOccupancy.DebugRender(drawList, start, 0.0625f);
                drawList->PopClipRect();

                // set back cursor so we can draw our box
                ImGui::SetCursorScreenPos(start);
                ImGui::InvisibleButton("Physical texture occupancy quadtree", ImVec2(512.0f, 512.0f));
                */
            }

            {

                ImGui::NewLine();
                ImGui::Separator();
                ImGui::Text("Физический обычный кэш");
                CoreGraphics::TextureDimensions dims = CoreGraphics::TextureGetDimensions(terrainVirtualTileState.physicalNormalCacheBC);

                ImVec2 imageSize = { (float)dims.width, (float)dims.height };

                static Dynui::ImguiTextureId textureInfo;
                textureInfo.nebulaHandle = terrainVirtualTileState.physicalNormalCacheBC;
                textureInfo.mip = 0;
                textureInfo.layer = 0;

                imageSize.x = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
                float ratio = (float)dims.height / (float)dims.width;
                imageSize.y = imageSize.x * ratio;

                ImGui::Image((void*)&textureInfo, imageSize);
            }

            {

                ImGui::NewLine();
                ImGui::Separator();
                ImGui::Text("Карта теней местности");
                static int mip = 0;
                ImGui::InputInt("Mip", &mip, 1, 1);

                CoreGraphics::TextureDimensions dims = CoreGraphics::TextureGetDimensions(terrainState.terrainShadowMap);

                ImVec2 imageSize = { (float)dims.width, (float)dims.height };

                static Dynui::ImguiTextureId textureInfo;
                textureInfo.nebulaHandle = terrainState.terrainShadowMap;
                textureInfo.mip = mip;
                textureInfo.layer = 0;

                imageSize.x = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
                float ratio = (float)dims.height / (float)dims.width;
                imageSize.y = imageSize.x * ratio;

                ImGui::Image((void*)&textureInfo, imageSize);
            }
        }

        ImGui::End();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
TerrainContext::ClearCache()
{
    terrainVirtualTileState.physicalTextureTileCache.Clear();
}

//------------------------------------------------------------------------------
/**
*/
void 
TerrainContext::OnRenderDebug(uint32_t flags)
{
    using namespace CoreGraphics;
    ShapeRenderer* shapeRenderer = ShapeRenderer::Instance();
    Util::Array<TerrainRuntimeInfo>& runtimes = terrainAllocator.GetArray<Terrain_RuntimeInfo>();
    for (IndexT i = 0; i < runtimes.Size(); i++)
    {
        TerrainRuntimeInfo& rt = runtimes[i];
        for (IndexT j = 0; j < rt.sectionBoxes.Size(); j++)
        {
            RenderShape shape(RenderShape::Box, RenderShape::Wireframe, rt.sectionBoxes[j].to_mat4(), Math::vec4(0, 1, 0, 1));
            shapeRenderer->AddShape(shape);
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
Graphics::ContextEntityId
TerrainContext::Alloc()
{
    return terrainAllocator.Alloc();
}

//------------------------------------------------------------------------------
/**
*/
void
TerrainContext::Dealloc(Graphics::ContextEntityId id)
{
    terrainAllocator.Dealloc(id.id);
}

} // пространство имен Terrain
