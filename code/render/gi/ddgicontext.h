#pragma once
#include <gi/shaders/probe_finalize.h>
#include <gi/shaders/probe_update.h>
#include "graphics/graphicscontext.h"
namespace GI
{
union DDGIOptions
{
    struct
    {
        uint scrolling : 1;
        uint classify : 1;
        uint relocate : 1;
        uint lowPrecisionTextures : 1;
    } flags;
    uint32_t bits = 0x0;
};
class DDGIContext : public Graphics::GraphicsContext
{
    __DeclareContext()
public:
    DDGIContext();
    ~DDGIContext();
    static void Create();
    static void Discard();
    struct VolumeSetup
    {
        uint numProbesX, numProbesY, numProbesZ;
        uint numRaysPerProbe;
        Math::vec3 size;
        Math::vec3 position;
        float normalBias = 0.1f;
        float viewBias = 0.4f;
        float irradianceScale = 1.0f;
        float distanceExponent = 0.5f;
        float encodingGamma = 5.0f;
        float changeThreshold = 0.2f;
        float brightnessThreshold = 2.0f;
        float hysteresis = 0.97f;
        float blendCutoff = 0.0f;
        float blend = 0.0f;
        float updateBudget = 1.0f;
        DDGIOptions options;
    };
    static void SetupVolume(const Graphics::GraphicsEntityId id, const VolumeSetup& setup);
    static void SetPosition(const Graphics::GraphicsEntityId id, const Math::vec3& position);
    static void SetSize(const Graphics::GraphicsEntityId id, const Math::vec3& size);
    static void UpdateActiveVolumes(const Ptr<Graphics::View>& view, const Graphics::FrameContext& ctx);
#ifndef PUBLIC_BUILD
    static void OnRenderDebug(uint32_t flags);
#endif
private:
    struct Volume
    {
        uint numProbesX, numProbesY, numProbesZ;
        uint numRaysPerProbe;
        Math::vec3 size;
        Math::vec3 position;
        Math::bbox boundingBox;
        CoreGraphics::TextureId radiance;
        CoreGraphics::TextureId irradiance, distance, offsets, states, scrollSpace;
        CoreGraphics::BufferId volumeConstantBuffer;
        CoreGraphics::ResourceTableId updateProbesTable, blendProbesTable, relocateProbesTable;
        float normalBias = 0.1f;
        float viewBias = 0.4f;
        float irradianceScale = 1.0f;
        float distanceExponent = 50.0f;
        float encodingGamma = 5.0f;
        float changeThreshold = 0.2f;
        float brightnessThreshold = 2.0f;
        float backfaceThreshold = 0.25f;
        float minFrontfaceDistance = 0.1f;
        float hysteresis = 0.97f;
        float blendCutoff = 0.0f;
        float blend = 0.0f;
        ProbeUpdate::VolumeConstants volumeConstants;
        DDGIOptions options;
#ifndef PUBLIC_BUILD
        CoreGraphics::ResourceTableId debugResourceTable;
#endif
    };
    typedef Ids::IdAllocator<
        Volume
    > DDGIVolumeAllocator;
    static DDGIVolumeAllocator ddgiVolumeAllocator;
    static Graphics::ContextEntityId Alloc();
    static void Dealloc(Graphics::ContextEntityId id);
};
}