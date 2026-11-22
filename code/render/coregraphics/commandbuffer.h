#pragma once
#include "ids/id.h"
#include "util/stack.h"
#include "coregraphics/config.h"
#include "coregraphics/primitivetopology.h"
#include "util/fixedarray.h"
#include "coregraphics/config.h"
#include "math/rectangle.h"
#include "coregraphics/indextype.h"
#include "ids/idallocator.h"
#include "coregraphics/memory.h"
#include "math/vec4.h"
namespace CoreGraphics
{
struct BufferId;
struct TextureId;
struct ResourceTableId;
struct VertexLayoutId;
class PrimitiveGroup;
struct ShaderProgramId;
struct BarrierId;
struct EventId;
struct PassId;
struct PipelineId;
struct BlasId;
struct TlasId;
struct TextureBarrierInfo;
struct BufferBarrierInfo;
struct AccelerationStructureBarrierInfo;
enum CmdBufferQueryBits
{
    NoQueries           = 0x0
    , Occlusion         = 0x1
    , Statistics        = 0x2
    , Timestamps        = 0x4
    , NumBits           = 3
};
__ImplementEnumBitOperators(CmdBufferQueryBits);
__ImplementEnumComparisonOperators(CmdBufferQueryBits);
enum class CmdPipelineBuildBits : uint
{
    NoInfoSet = 0,
    ShaderInfoSet = N_BIT(0),
    FramebufferLayoutInfoSet = N_BIT(1),
    InputAssemblyInfoSet = N_BIT(2),
    AllInfoSet = ShaderInfoSet | FramebufferLayoutInfoSet | InputAssemblyInfoSet,
    PipelineBuilt = N_BIT(3)
};
__ImplementEnumBitOperators(CmdPipelineBuildBits);
__ImplementEnumComparisonOperators(CmdPipelineBuildBits);
struct TextureCopy
{
    Math::rectangle<SizeT> region;
    uint mip;
    uint layer;
    ImageBits bits = ImageBits::ColorBits;
};
struct BufferCopy
{
    uint offset;
    uint rowLength = 0;
    uint imageHeight = 0;
};
#ifdef NEBULA_ENABLE_PROFILING
struct FrameProfilingMarker
{
    CoreGraphics::QueueType queue;
    Math::vec4 color;
    Util::String name;
    IndexT gpuBegin;
    IndexT gpuEnd;
    uint64_t start;
    uint64_t duration;
    Util::Array<FrameProfilingMarker> children;
};
#endif
ID_24_8_TYPE(CmdBufferPoolId);
struct CmdBufferPoolCreateInfo
{
#if NEBULA_GRAPHICS_DEBUG
    const char* name;
#endif
    CoreGraphics::QueueType queue;
    bool resetable : 1;
    bool shortlived : 1;
    CmdBufferPoolCreateInfo()
        : queue(CoreGraphics::InvalidQueueType)
        , resetable(false)
        , shortlived(false)
#if NEBULA_GRAPHICS_DEBUG
        , name(nullptr)
#endif
    {};
};
const CmdBufferPoolId CreateCmdBufferPool(const CmdBufferPoolCreateInfo& info);
void DestroyCmdBufferPool(const CmdBufferPoolId pool);
struct CmdBufferCreateInfo
{
    bool subBuffer : 1;
    CmdBufferPoolId pool;
    QueueType usage;
    CmdBufferQueryBits queryTypes;
#if NEBULA_GRAPHICS_DEBUG
    const char* name;
#endif
    CmdBufferCreateInfo()
        : subBuffer(false)
        , pool(InvalidCmdBufferPoolId)
        , usage(QueueType::GraphicsQueueType)
        , queryTypes(CmdBufferQueryBits::Timestamps)
#if NEBULA_GRAPHICS_DEBUG
        , name(nullptr)
#endif
    {};
};
struct CmdBufferBeginInfo
{
    bool submitOnce : 1;
    bool submitDuringPass : 1;
    bool resubmittable : 1;
};
struct CmdBufferClearInfo
{
    bool allowRelease : 1;
};
struct CmdBufferMarkerBundle
{
    Util::Stack<CoreGraphics::FrameProfilingMarker> markerStack;
    Util::Array<CoreGraphics::FrameProfilingMarker> finishedMarkers;
};
ID_24_8_TYPE(CmdBufferId);
_DECL_ACQUIRE_RELEASE(CmdBufferId);
const CmdBufferId CreateCmdBuffer(const CmdBufferCreateInfo& info);
void DestroyCmdBuffer(const CmdBufferId id);
void DeferredDestroyCmdBuffer(const CmdBufferId id);
void CmdBeginRecord(const CmdBufferId id, const CmdBufferBeginInfo& info);
void CmdEndRecord(const CmdBufferId id);
void CmdReset(const CmdBufferId id, const CmdBufferClearInfo& info);
void CmdSetVertexBuffer(const CmdBufferId id, IndexT streamIndex, const CoreGraphics::BufferId& buffer, SizeT bufferOffset);
void CmdSetVertexLayout(const CmdBufferId id, const CoreGraphics::VertexLayoutId& vl);
void CmdSetIndexBuffer(const CmdBufferId id, const IndexType::Code indexType, const CoreGraphics::BufferId& buffer, SizeT bufferOffset);
void CmdSetPrimitiveTopology(const CmdBufferId id, const CoreGraphics::PrimitiveTopology::Code topo);
void CmdSetShaderProgram(const CmdBufferId id, const CoreGraphics::ShaderProgramId pro, bool bindGlobals = true);
void CmdSetResourceTable(const CmdBufferId id, const CoreGraphics::ResourceTableId table, const IndexT slot, CoreGraphics::ShaderPipeline pipeline, const Util::FixedArray<uint, true>& offsets = nullptr);
void CmdSetResourceTable(const CmdBufferId id, const CoreGraphics::ResourceTableId table, const IndexT slot, CoreGraphics::ShaderPipeline pipeline, uint32_t numOffsets, uint32_t* offsets);
void CmdPushConstants(const CmdBufferId id, ShaderPipeline pipeline, uint offset, uint size, const void* data);
void CmdSetGraphicsPipeline(const CmdBufferId id);
void CmdSetGraphicsPipeline(const CmdBufferId buf, const PipelineId pipeline);
void CmdSetRayTracingPipeline(const CmdBufferId buf, const PipelineId pipeline);
void CmdBarrier(
            const CmdBufferId id,
            CoreGraphics::PipelineStage fromStage,
            CoreGraphics::PipelineStage toStage,
            CoreGraphics::BarrierDomain domain,
            const IndexT fromQueue = InvalidIndex,
            const IndexT toQueue = InvalidIndex,
            const char* name = nullptr
);
void CmdBarrier(
            const CmdBufferId id,
            CoreGraphics::PipelineStage fromStage,
            CoreGraphics::PipelineStage toStage,
            CoreGraphics::BarrierDomain domain,
            const Util::FixedArray<TextureBarrierInfo, true>& textures,
            const IndexT fromQueue = InvalidIndex,
            const IndexT toQueue = InvalidIndex,
            const char* name = nullptr
);
void CmdBarrier(
            const CmdBufferId id,
            CoreGraphics::PipelineStage fromStage,
            CoreGraphics::PipelineStage toStage,
            CoreGraphics::BarrierDomain domain,
            const Util::FixedArray<BufferBarrierInfo, true>& buffers,
            const IndexT fromQueue = InvalidIndex,
            const IndexT toQueue = InvalidIndex,
            const char* name = nullptr
);
void CmdBarrier(
            const CmdBufferId id,
            CoreGraphics::PipelineStage fromStage,
            CoreGraphics::PipelineStage toStage,
            CoreGraphics::BarrierDomain domain,
            const Util::FixedArray<AccelerationStructureBarrierInfo, true>& accelerationStructures,
            const IndexT fromQueue = InvalidIndex,
            const IndexT toQueue = InvalidIndex,
            const char* name = nullptr
);
void CmdBarrier(
            const CmdBufferId id,
            CoreGraphics::PipelineStage fromStage,
            CoreGraphics::PipelineStage toStage,
            CoreGraphics::BarrierDomain domain,
            const Util::FixedArray<TextureBarrierInfo, true>& textures,
            const Util::FixedArray<BufferBarrierInfo, true>& buffers,
            const Util::FixedArray<AccelerationStructureBarrierInfo, true>& accelerationStructures,
            const IndexT fromQueue = InvalidIndex,
            const IndexT toQueue = InvalidIndex,
            const char* name = nullptr
);
void CmdHandover(
            const CmdBufferId from,
            const CmdBufferId to,
            CoreGraphics::PipelineStage fromStage,
            CoreGraphics::PipelineStage toStage,
            const Util::FixedArray<TextureBarrierInfo, true>& textures,
            const Util::FixedArray<BufferBarrierInfo, true>& buffers,
            const IndexT fromQueue = InvalidIndex,
            const IndexT toQueue = InvalidIndex,
            const char* name = nullptr
);
void CmdBarrier(const CmdBufferId id, const CoreGraphics::BarrierId barrier);
void CmdSignalEvent(const CmdBufferId id, const CoreGraphics::EventId ev, const CoreGraphics::PipelineStage stage);
void CmdWaitEvent(
    const CmdBufferId id
    , const EventId ev
    , const CoreGraphics::PipelineStage waitStage
    , const CoreGraphics::PipelineStage signalStage
);
void CmdResetEvent(const CmdBufferId id, const CoreGraphics::EventId ev, const CoreGraphics::PipelineStage stage);
void CmdBeginPass(const CmdBufferId id, const CoreGraphics::PassId pass);
void CmdNextSubpass(const CmdBufferId id);
void CmdEndPass(const CmdBufferId id);
void CmdDraw(const CmdBufferId id, const CoreGraphics::PrimitiveGroup& pg);
void CmdDraw(const CmdBufferId id, SizeT numInstances, const CoreGraphics::PrimitiveGroup& pg);
void CmdDraw(const CmdBufferId id, SizeT numInstances, IndexT baseInstance, const CoreGraphics::PrimitiveGroup& pg);
void CmdDrawIndirect(const CmdBufferId id, const CoreGraphics::BufferId buffer, IndexT bufferOffset, SizeT numDraws, SizeT stride);
void CmdDrawIndirectIndexed(const CmdBufferId id, const CoreGraphics::BufferId buffer, IndexT bufferOffset, SizeT numDraws, SizeT stride);
void CmdDispatch(const CmdBufferId id, int dimX, int dimY, int dimZ);
void CmdResolve(const CmdBufferId id, const CoreGraphics::TextureId source, const CoreGraphics::TextureCopy sourceCopy, const CoreGraphics::TextureId dest, const CoreGraphics::TextureCopy destCopy);
void CmdBuildBlas(const CmdBufferId id, const CoreGraphics::BlasId blas);
void CmdBuildTlas(const CmdBufferId id, const CoreGraphics::TlasId tlas);
struct RayDispatchTable
{
    struct Entry
    {
        CoreGraphics::DeviceAddress baseAddress;
        CoreGraphics::DeviceSize entrySize;
        ubyte numEntries;
    };
    Entry genEntry, missEntry, hitEntry, callableEntry;
};
void CmdRaysDispatch(const CmdBufferId id, const RayDispatchTable& table, int dimX, int dimY, int dimZ);
void CmdDrawMeshlets(const CmdBufferId id, int dimX, int dimY, int dimZ);
void CmdCopy(
    const CmdBufferId id
    , const CoreGraphics::TextureId fromTexture
    , const Util::Array<CoreGraphics::TextureCopy, 4>& from
    , const CoreGraphics::TextureId toTexture
    , const Util::Array<CoreGraphics::TextureCopy, 4>& to
);
void CmdCopy(
    const CmdBufferId id
    , const CoreGraphics::TextureId fromTexture
    , const Util::Array<CoreGraphics::TextureCopy, 4>& from
    , const CoreGraphics::BufferId toBuffer
    , const Util::Array<CoreGraphics::BufferCopy, 4>& to
);
void CmdCopy(
    const CmdBufferId id
    , const CoreGraphics::BufferId fromBuffer
    , const Util::Array<CoreGraphics::BufferCopy, 4>& from
    , const CoreGraphics::BufferId toBuffer
    , const Util::Array<CoreGraphics::BufferCopy, 4>& to
    , const SizeT size
);
void CmdCopy(
    const CmdBufferId id
    , const CoreGraphics::BufferId fromBuffer
    , const Util::Array<CoreGraphics::BufferCopy, 4>& from
    , const CoreGraphics::TextureId toTexture
    , const Util::Array<CoreGraphics::TextureCopy, 4>& to
);
void CmdBlit(
    const CmdBufferId id
    , const CoreGraphics::TextureId fromTexture
    , const CoreGraphics::TextureCopy& from
    , const CoreGraphics::TextureId toTexture
    , const CoreGraphics::TextureCopy& to
);
void CmdSetViewports(const CmdBufferId id, const Util::FixedArray<Math::rectangle<int>>& viewports);
void CmdSetScissors(const CmdBufferId id, const Util::FixedArray<Math::rectangle<int>>& rects);
void CmdSetViewport(const CmdBufferId id, const Math::rectangle<int>& rect, int index);
void CmdSetScissorRect(const CmdBufferId id, const Math::rectangle<int>& rect, int index);
void CmdSetStencilRef(const CmdBufferId id, const uint frontRef, const uint backRef);
void CmdSetStencilReadMask(const CmdBufferId id, const uint readMask);
void CmdSetStencilWriteMask(const CmdBufferId id, const uint writeMask);
void CmdUpdateBuffer(
    const CmdBufferId id
    , const CoreGraphics::BufferId buffer
    , uint offset
    , uint size
    , const void* data
);
void CmdStartOcclusionQueries(const CmdBufferId id);
void CmdEndOcclusionQueries(const CmdBufferId id);
void CmdStartPipelineQueries(const CmdBufferId id);
void CmdEndPipelineQueries(const CmdBufferId id);
#if NEBULA_GRAPHICS_DEBUG
void CmdBeginMarker(const CmdBufferId id, const Math::vec4& color, const char* name);
void CmdEndMarker(const CmdBufferId id);
void CmdInsertMarker(const CmdBufferId id, const Math::vec4& color, const char* name);
#endif
void CmdFinishQueries(const CmdBufferId id);
#if NEBULA_ENABLE_PROFILING
bool CmdRecordsMarkers(const CmdBufferId id);
Util::Array<CoreGraphics::FrameProfilingMarker>&& CmdMoveProfilingMarkers(const CmdBufferId id);
uint CmdGetMarkerOffset(const CmdBufferId id);
#endif
#if NEBULA_GRAPHICS_DEBUG
struct CmdMarkerScope
{
    const CmdBufferId id;
    CmdMarkerScope(const CmdBufferId id, const Math::vec4& color, const char* name)
        : id(id)
    {
        CoreGraphics::CmdBeginMarker(id, color, name);
    }
    ~CmdMarkerScope()
    {
        CoreGraphics::CmdEndMarker(this->id);
    }
};
#endif
#if NEBULA_GRAPHICS_DEBUG
    #define N_CMD_SCOPE(buf, color, name) CoreGraphics::CmdMarkerScope __##cmdscope(buf, color, name)
#else
    #define N_CMD_SCOPE(x, y, z)
#endif
}