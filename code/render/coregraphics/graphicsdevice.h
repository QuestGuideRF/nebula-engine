#pragma once
#include "pass.h"
#include "profiling/profiling.h"
#include "frame/framescript.h"
#include "primitivegroup.h"
#include "buffer.h"
#include "imagefileformat.h"
#include "io/stream.h"
#include "fence.h"
#include "debug/debugcounter.h"
#include "coregraphics/rendereventhandler.h"
#include "coregraphics/resourcetable.h"
#include "memory/rangeallocator.h"
#include "memory.h"
#include "util/set.h"
namespace CoreGraphics
{
extern bool RayTracingSupported;
extern bool DynamicVertexInputSupported;
extern bool VariableRateShadingSupported;
extern bool MeshShadersSupported;
extern bool NvidiaCheckpointsSupported;
extern uint ReadWriteBufferAlignment;
extern uint ConstantBufferAlignment;
extern uint64_t MaxConstantBufferSize;
extern uint MaxPushConstantSize;
extern uint64_t SparseAddressSize;
extern uint MaxPerStageConstantBuffers;
extern uint MaxPerStageReadWriteBuffers;
extern uint MaxPerStageSampledImages;
extern uint MaxPerStageReadWriteImages;
extern uint MaxPerStageSamplers;
extern uint MaxPerStageInputAttachments;
extern uint MaxResourceTableConstantBuffers;
extern uint MaxResourceTableDynamicOffsetConstantBuffers;
extern uint MaxResourceTableReadWriteBuffers;
extern uint MaxResourceTableDynamicOffsetReadWriteBuffers;
extern uint MaxResourceTableSampledImages;
extern uint MaxResourceTableReadWriteImages;
extern uint MaxResourceTableSamplers;
extern uint MaxResourceTableInputAttachments;
extern uint MemoryRangeGranularity;
extern uint TimestampPeriod;
extern uint AccelerationStructureScratchAlignment;
extern uint ShaderGroupAlignment;
extern uint64_t ShaderGroupSize;
extern uint MaxRecursionDepth;
struct GraphicsDeviceCreateInfo
{
    uint64_t globalConstantBufferMemorySize;
    uint64_t globalVertexBufferMemorySize;
    uint64_t globalIndexBufferMemorySize;
    uint64_t globalUploadMemorySize;
    uint64_t memoryHeaps[NumMemoryPoolTypes];
    uint64_t maxOcclusionQueries, maxTimestampQueries, maxStatisticsQueries;
    byte numBufferedFrames : 3;
    bool enableValidation : 1;
    struct Features
    {
        bool enableRayTracing : 1;
        bool enableMeshShaders : 1;
        bool enableVariableRateShading : 1;
        bool enableGPUCrashAnalytics : 1;
    } features;
};
bool CreateGraphicsDevice(const GraphicsDeviceCreateInfo& info);
void DestroyGraphicsDevice();
SizeT GetNumBufferedFrames();
IndexT GetBufferedFrameIndex();
struct DrawThreadResult
{
    CoreGraphics::CmdBufferId buf;
    Threading::Event* event;
};
struct SubmissionWaitEvent
{
    SubmissionWaitEvent();
    const bool operator==(const std::nullptr_t) const;
    const bool operator!=(const std::nullptr_t) const;
    void operator=(const std::nullptr_t);
#if __VULKAN__
    uint64_t timelineIndex;
#endif
    CoreGraphics::QueueType queue;
};
struct GraphicsDeviceState
{
    Util::Array<CoreGraphics::TextureId> backBuffers;
    CoreGraphics::CmdBufferPoolId setupTransferCommandBufferPool;
    Util::Array<CoreGraphics::CmdBufferId> setupTransferCommandBuffers;
    Util::Array<CoreGraphics::CmdBufferId> handoverTransferCommandBuffers;
    CoreGraphics::CmdBufferPoolId setupGraphicsCommandBufferPool;
    Util::Array<CoreGraphics::CmdBufferId> setupGraphicsCommandBuffers;
    Util::FixedArray<CoreGraphics::FenceId> presentFences;
    Util::FixedArray<CoreGraphics::SemaphoreId> renderingFinishedSemaphores;
    uint globalConstantBufferMaxValue;
    Util::FixedArray<CoreGraphics::BufferId> globalConstantBuffer;
    CoreGraphics::ResourceTableId tickResourceTableGraphics;
    CoreGraphics::ResourceTableId tickResourceTableCompute;
    CoreGraphics::ResourceTableId frameResourceTableGraphics;
    CoreGraphics::ResourceTableId frameResourceTableCompute;
    Util::Array<Ptr<CoreGraphics::RenderEventHandler> > eventHandlers;
    Memory::RangeAllocator vertexAllocator;
    CoreGraphics::BufferId vertexBuffer;
    Memory::RangeAllocator indexAllocator;
    CoreGraphics::BufferId indexBuffer;
    Memory::RangeAllocator uploadAllocator;
    uint maxNumBufferedFrames = 1;
    uint32_t currentBufferedFrameIndex = 0;
    bool isOpen : 1;
    bool inNotifyEventHandlers : 1;
    bool renderWireframe : 1;
    bool visualizeMipMaps : 1;
    bool enableValidation : 1;
    IndexT currentFrameIndex = 0;
    _declare_counter(NumImageBytesAllocated);
    _declare_counter(NumBufferBytesAllocated);
    _declare_counter(NumBytesAllocated);
    _declare_counter(GraphicsDeviceNumComputes);
    _declare_counter(GraphicsDeviceNumPrimitives);
    _declare_counter(GraphicsDeviceNumDrawCalls);
#ifdef NEBULA_ENABLE_PROFILING
    Util::Array<FrameProfilingMarker> frameProfilingMarkers;
#endif
};
struct GraphicsDeviceThreadState
{
    CoreGraphics::CmdBufferId graphicsCommandBuffer, computeCommandBuffer;
    CmdPipelineBuildBits currentPipelineBits;
    CoreGraphics::PrimitiveTopology::Code primitiveTopology;
    CoreGraphics::PrimitiveGroup primitiveGroup;
    CoreGraphics::PassId pass;
    bool isOpen : 1;
    bool inNotifyEventHandlers : 1;
    bool renderWireframe : 1;
    bool visualizeMipMaps : 1;
    bool usePatches : 1;
    bool enableValidation : 1;
};
void AttachEventHandler(const Ptr<CoreGraphics::RenderEventHandler>& h);
void RemoveEventHandler(const Ptr<CoreGraphics::RenderEventHandler>& h);
bool NotifyEventHandlers(const CoreGraphics::RenderEvent& e);
void AddBackBufferTexture(const CoreGraphics::TextureId tex);
void RemoveBackBufferTexture(const CoreGraphics::TextureId tex);
const CoreGraphics::CmdBufferId LockTransferSetupCommandBuffer();
void UnlockTransferSetupCommandBuffer(CoreGraphics::CmdBufferId cmdBuf);
const CoreGraphics::CmdBufferId LockGraphicsSetupCommandBuffer(const char* name = nullptr);
void UnlockGraphicsSetupCommandBuffer(CoreGraphics::CmdBufferId cmdBuf);
const CoreGraphics::CmdBufferId LockTransferHandoverSetupCommandBuffer();
void UnlockTransferHandoverSetupCommandBuffer(CoreGraphics::CmdBufferId cmdBuf);
bool PollSubmissionIndex(const CoreGraphics::QueueType queue, uint64_t index);
SubmissionWaitEvent SubmitCommandBuffers(
    const Util::Array<CoreGraphics::CmdBufferId, 8>& cmds
    , CoreGraphics::QueueType type
    , Util::Array<CoreGraphics::SubmissionWaitEvent, 8> waitEvents = nullptr
#if NEBULA_GRAPHICS_DEBUG
    , const char* name = nullptr
#endif
);
void SubmitImmediateCommandBuffers();
void UnlockConstantUpdates();
template<class TYPE> uint64_t SetConstants(const TYPE& data);
template<class TYPE> uint64_t SetConstants(const TYPE* data, SizeT elements);
template<class TYPE> void SetConstants(ConstantBufferOffset offset, const TYPE& data);
template<class TYPE> void SetConstants(ConstantBufferOffset offset, const TYPE* data, SizeT numElements);
void LockConstantUpdates();
void SetConstantsInternal(ConstantBufferOffset offset, const void* data, SizeT size);
ConstantBufferOffset AllocateConstantBufferMemory(uint size);
CoreGraphics::BufferId GetConstantBuffer(IndexT i);
const VertexAlloc AllocateVertices(const SizeT numVertices, const SizeT vertexSize);
const VertexAlloc AllocateVertices(const SizeT bytes);
void DeallocateVertices(const VertexAlloc& alloc);
const CoreGraphics::BufferId GetVertexBuffer();
const VertexAlloc AllocateIndices(const SizeT numIndices, const IndexType::Code indexType);
const VertexAlloc AllocateIndices(const SizeT bytes);
void DeallocateIndices(const VertexAlloc& alloc);
const CoreGraphics::BufferId GetIndexBuffer();
Util::Pair<Memory::RangeAllocation, CoreGraphics::BufferId> AllocateUpload(const SizeT numBytes, const SizeT alignment = 1);
template<class TYPE> Util::Pair<Memory::RangeAllocation, CoreGraphics::BufferId> Upload(const TYPE& data, const SizeT alignment = 1);
template<class TYPE> Util::Pair<Memory::RangeAllocation, CoreGraphics::BufferId> UploadArray(const TYPE* data, SizeT elements, const SizeT alignment = 1);
template<class TYPE> void Upload(const CoreGraphics::BufferId buffer, uint offset, const TYPE& data);
template<class TYPE> void Upload(const CoreGraphics::BufferId buffer, uint offset, const TYPE* data, SizeT elements);
void UploadInternal(const CoreGraphics::BufferId buffer, uint offset, const void* data, SizeT size);
void FreeUploads(const Util::Array<Memory::RangeAllocation>& allocations);
void FlushUploads(const Util::Array<Memory::RangeAllocation>& allocations);
void EnqueueUploadsFlushAndFree(const Util::Array<Memory::RangeAllocation>& allocations);
void ReloadShaderProgram(const CoreGraphics::ShaderProgramId& pro);
void WaitForQueue(CoreGraphics::QueueType queue);
void WaitAndClearPendingCommands();
void DelayedDeleteBuffer(const CoreGraphics::BufferId id);
void DelayedDeleteTexture(const CoreGraphics::TextureId id);
void DelayedDeleteTextureView(const CoreGraphics::TextureViewId id);
void DelayedDeleteCommandBuffer(const CoreGraphics::CmdBufferId id);
void DelayedFreeMemory(const CoreGraphics::Alloc alloc);
void DelayedDeleteDescriptorSet(const CoreGraphics::ResourceTableId id);
void DelayedDeletePass(const CoreGraphics::PassId id);
void DelayedDeleteBlas(const CoreGraphics::BlasId id);
void DelayedDeleteTlas(const CoreGraphics::TlasId id);
uint AllocateQueries(const CoreGraphics::QueryType type, uint numQueries);
void FinishQueries(const CoreGraphics::CmdBufferId cmdBuf, const CoreGraphics::QueryType type, IndexT* starts, SizeT* counts, SizeT numCopies);
IndexT GetQueueIndex(const QueueType queue);
const Util::Set<uint32_t>& GetQueueIndices();
void FinishFrame(IndexT frameIndex);
void NewFrame();
CoreGraphics::ImageFileFormat::Code SaveScreenshot(CoreGraphics::ImageFileFormat::Code fmt, const Ptr<IO::Stream>& outStream);
CoreGraphics::ImageFileFormat::Code SaveScreenshot(CoreGraphics::ImageFileFormat::Code fmt, const Ptr<IO::Stream>& outStream, const Math::rectangle<int>& rect, int x, int y);
bool GetVisualizeMipMaps();
void SetVisualizeMipMaps(bool val);
bool GetRenderWireframe();
void SetRenderWireframe(bool b);
#if NEBULA_ENABLE_PROFILING
IndexT Timestamp(CoreGraphics::QueueType queue, const CoreGraphics::PipelineStage stage, const char* name);
const Util::Array<FrameProfilingMarker>& GetProfilingMarkers();
SizeT GetNumDrawCalls();
#endif
#if NEBULA_GRAPHICS_DEBUG
template<typename OBJECT_ID_TYPE> void ObjectSetName(const OBJECT_ID_TYPE id, const char* name);
void QueueBeginMarker(const CoreGraphics::QueueType queue, const Math::vec4& color, const char* name);
void QueueEndMarker(const CoreGraphics::QueueType queue);
void QueueInsertMarker(const CoreGraphics::QueueType queue, const Math::vec4& color, const char* name);
#endif
template<class TYPE>
inline ConstantBufferOffset
SetConstants(const TYPE& data)
{
    const uint uploadSize = sizeof(TYPE);
    ConstantBufferOffset ret = AllocateConstantBufferMemory(uploadSize);
    SetConstantsInternal(ret, &data, uploadSize);
    return ret;
}
template<class TYPE>
inline ConstantBufferOffset
SetConstants(const TYPE* data, SizeT numElements)
{
    const uint uploadSize = sizeof(TYPE) * numElements;
    ConstantBufferOffset ret = AllocateConstantBufferMemory(uploadSize);
    SetConstantsInternal(ret, data, uploadSize);
    return ret;
}
template<class TYPE>
inline void
SetConstants(ConstantBufferOffset offset, const TYPE& data)
{
    SetConstantsInternal(offset, &data, sizeof(TYPE));
}
template<class TYPE>
inline void
SetConstants(ConstantBufferOffset offset, const TYPE* data, SizeT numElements)
{
    SetConstantsInternal(offset, data, sizeof(TYPE) * numElements);
}
template<class TYPE>
inline Util::Pair<Memory::RangeAllocation, CoreGraphics::BufferId>
Upload(const TYPE& data, const SizeT alignment)
{
    const uint uploadSize = sizeof(TYPE);
    auto [alloc, buffer] = AllocateUpload(uploadSize, alignment);
    if (buffer != CoreGraphics::InvalidBufferId)
        UploadInternal(buffer, alloc.offset, &data, uploadSize);
    return Util::MakePair(alloc, buffer);
}
template<class TYPE>
inline Util::Pair<Memory::RangeAllocation, CoreGraphics::BufferId>
UploadArray(const TYPE* data, SizeT numElements, const SizeT alignment)
{
    const uint uploadSize = sizeof(TYPE) * numElements;
    auto [alloc, buffer] = AllocateUpload(uploadSize, alignment);
    if (buffer != CoreGraphics::InvalidBufferId)
        UploadInternal(buffer, alloc.offset, data, uploadSize);
    return Util::MakePair(alloc, buffer);
}
template<class TYPE>
inline void
Upload(const CoreGraphics::BufferId buffer, const uint offset, const TYPE& data)
{
    const uint uploadSize = sizeof(TYPE);
    UploadInternal(buffer, offset, &data, uploadSize);
}
template<class TYPE>
inline void
Upload(const CoreGraphics::BufferId buffer, const uint offset, const TYPE* data, SizeT numElements)
{
    const uint uploadSize = sizeof(TYPE) * numElements;
    UploadInternal(buffer, offset, data, uploadSize);
}
template<>
inline Util::Pair<Memory::RangeAllocation, CoreGraphics::BufferId>
UploadArray(const void* data, SizeT numBytes, SizeT alignment)
{
    auto [alloc, buffer] = AllocateUpload(numBytes, alignment);
    if (buffer != CoreGraphics::InvalidBufferId)
        UploadInternal(buffer, alloc.offset, data, numBytes);
    return Util::MakePair(alloc, buffer);
}
}