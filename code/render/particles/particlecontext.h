#pragma once
#include "graphics/graphicscontext.h"
#include "models/model.h"
#include "models/nodes/modelnode.h"
#include "models/nodes/particlesystemnode.h"
#include "jobs/jobs.h"
#include "jobs2/jobs2.h"
#include "util/ringbuffer.h"
#include "particle.h"
namespace Particles
{
class EmitterAttrs;
class EmitterMesh;
class EnvelopeSampleBuffer;
class ParticleContext : public Graphics::GraphicsContext
{
    __DeclareContext();
public:
    enum PlayMode
    {
        RestartIfPlaying,
        IgnoreIfPlaying
    };
    ParticleContext();
    virtual ~ParticleContext();
    static void Create();
    static void Setup(const Graphics::GraphicsEntityId id);
    static void ShowParticle(const Graphics::GraphicsEntityId id);
    static void HideParticle(const Graphics::GraphicsEntityId id);
    static void Play(const Graphics::GraphicsEntityId id, const PlayMode mode);
    static void Stop(const Graphics::GraphicsEntityId id);
    static void UpdateParticles(const Graphics::FrameContext& ctx);
    static void OnPrepareView(const Ptr<Graphics::View>& view, const Graphics::FrameContext& ctx);
    static void WaitForParticleUpdates(const Graphics::FrameContext& ctx);
    static CoreGraphics::BufferId GetParticleIndexBuffer();
    static CoreGraphics::BufferId GetParticleVertexBuffer();
    static CoreGraphics::VertexLayoutId GetParticleVertexLayout();
    static CoreGraphics::PrimitiveGroup& GetParticlePrimitiveGroup();
#ifndef PUBLIC_DEBUG
    static void OnRenderDebug(uint32_t flags);
#endif
    static CoreGraphics::MeshId DefaultEmitterMesh;
    static Threading::AtomicCounter ConstantUpdateCounter;
    static Threading::Event totalCompletionEvent;
private:
    struct ParticleRuntime
    {
        Timing::Time stepTime;
        Timing::Time prevEmissionTime;
        Timing::Time emissionStartTimeOffset;
        IndexT emissionCounter : 30;
        bool initial : 1;
        bool playing : 1;
        bool stopping : 1;
        bool stopped : 1;
        bool restarting : 1;
        bool firstFrame : 1;
    };
    struct ParticleJobOutput
    {
        Math::bbox bbox;
        unsigned int numLivingParticles;
    };
    struct ParticleSystemRuntime
    {
        uint32_t renderableIndex;
        Util::RingBuffer<Particle> particles;
        Math::mat4 transform;
        Math::bbox boundingBox;
        SizeT emissionCounter;
        ParticleJobUniformData uniformData;
        SizeT outputCapacity;
        ParticleJobSliceOutputData outputData;
        Util::FixedArray<CoreGraphics::MeshId> meshPerFrame;
        CoreGraphics::MeshId mesh;
        uint32_t baseVertex, numParticles;
    };
    enum
    {
        ParticleSystems,
        ModelId,
        Runtime,
        ModelContextId
    };
    typedef Ids::IdAllocator<
        Util::Array<ParticleSystemRuntime>,
        Graphics::ContextEntityId,
        ParticleRuntime,
        Graphics::GraphicsEntityId
    > ParticleContextAllocator;
    static ParticleContextAllocator particleContextAllocator;
    static void EmitParticles(ParticleRuntime& rt, ParticleSystemRuntime& srt, Models::ParticleSystemNode* node, float stepTime);
    static void EmitParticle(ParticleRuntime& rt, ParticleSystemRuntime& srt, const Particles::EmitterAttrs& attrs, const Particles::EmitterMesh& mesh, const Particles::EnvelopeSampleBuffer& buffer, IndexT sampleIndex, float initialAge);
    static void RunParticleStep(ParticleRuntime& rt, ParticleSystemRuntime& srt, float stepTime, bool generateVtxList);
    static Graphics::ContextEntityId Alloc();
    static void Dealloc(Graphics::ContextEntityId id);
};
inline
Graphics::ContextEntityId
ParticleContext::Alloc()
{
    return particleContextAllocator.Alloc();
}
inline
void
ParticleContext::Dealloc(Graphics::ContextEntityId id)
{
    particleContextAllocator.Dealloc(id.id);
}
}