#pragma once
#include "config.h"
#include "ids/id.h"
#include "ids/idpool.h"
#include "util/stringatom.h"
#include "coregraphics/config.h"
#include "coregraphics/commandbuffer.h"
#include "gpubuffertypes.h"
#include "ids/idallocator.h"
namespace CoreGraphics
{
struct CmdBufferId;
ID_24_8_TYPE(BufferId);
_DECL_ACQUIRE_RELEASE(BufferId);
enum BufferAccessMode
{
    DeviceLocal,
    HostLocal,
    HostCached,
    DeviceAndHost
};
enum BufferUsageFlag
{
    InvalidBufferUsage              = 0x0,
    TransferBufferSource            = 0x1,
    TransferBufferDestination       = 0x2,
    ConstantBuffer                  = 0x4,
    ConstantTexelBuffer             = 0x8,
    ReadWriteBuffer                 = 0x10,
    ReadWriteTexelBuffer            = 0x20,
    VertexBuffer                    = 0x40,
    IndexBuffer                     = 0x80,
    IndirectBuffer                  = 0x100,
    ShaderAddress                   = 0x200,
    AccelerationStructureData       = 0x400,
    AccelerationStructureScratch    = 0x800,
    AccelerationStructureInput      = 0x1000,
    AccelerationStructureInstances  = 0x2000,
    ShaderTable                     = 0x4000
};
typedef uint BufferUsageFlags;
enum BufferQueueSupport
{
    AutomaticQueueSupport       = 0x0,
    GraphicsQueueSupport        = 0x1,
    ComputeQueueSupport         = 0x2,
    TransferQueueSupport        = 0x4
};
typedef uint BufferQueueSupportFlags;
struct BufferCreateInfo
{
    BufferCreateInfo()
        : name(""_atm)
        , size(1)
        , elementSize(1)
        , byteSize(0)
        , mode(DeviceLocal)
        , usageFlags(InvalidBufferUsage)
        , queueSupport(AutomaticQueueSupport)
        , sparse(false)
        , data(nullptr)
        , dataSize(0)
    {}
    Util::StringAtom name;
    SizeT size;
    SizeT elementSize;
    SizeT byteSize;
    BufferAccessMode mode;
    BufferUsageFlags usageFlags;
    BufferQueueSupportFlags queueSupport;
    bool sparse;
    const void* data;
    uint dataSize;
};
struct BufferSparsePage
{
    SizeT offset;
    CoreGraphics::Alloc alloc;
};
const BufferId CreateBuffer(const BufferCreateInfo& info);
void DestroyBuffer(const BufferId id);
const BufferUsageFlags BufferGetType(const BufferId id);
const uint64_t BufferGetSize(const BufferId id);
const uint64_t BufferGetElementSize(const BufferId id);
const uint64_t BufferGetByteSize(const BufferId id);
const uint64_t BufferGetUploadMaxSize();
void* BufferMap(const BufferId id);
template <class T> T* BufferMap(const BufferId id);
void BufferUnmap(const BufferId id);
void BufferUpdate(const BufferId id, const void* data, const uint size, const uint offset = 0);
void BufferUpload(const CoreGraphics::CmdBufferId cmdBuf, const BufferId id, const void* data, const uint size, const uint offset);
template<class TYPE> void BufferUpdate(const BufferId id, const TYPE& data, const uint offset = 0);
template<class TYPE> void BufferUpdateArray(const BufferId id, const TYPE* data, const uint count, const uint offset = 0);
template<class TYPE> void BufferUpdateArray(const BufferId id, const Util::Array<TYPE>& data, const uint offset = 0);
template<class TYPE> void BufferUpdateArray(const BufferId id, const Util::FixedArray<TYPE>& data, const uint offset = 0);
template<class TYPE> void BufferUpload(const CoreGraphics::CmdBufferId cmdBuf, const BufferId id, const TYPE* data, const uint count = 1, const uint offset = 0);
void BufferFill(const CoreGraphics::CmdBufferId cmdBuf, const BufferId id, char pattern);
void BufferFlush(const BufferId id, uint64_t offset = 0ull, uint64_t size = NEBULA_WHOLE_BUFFER_SIZE);
void BufferInvalidate(const BufferId id, uint64_t offset = 0ull, uint64_t size = NEBULA_WHOLE_BUFFER_SIZE);
void BufferSparseEvict(const BufferId id, IndexT pageIndex);
void BufferSparseMakeResident(const BufferId id, IndexT pageIndex);
IndexT BufferSparseGetPageIndex(const BufferId id, SizeT offset);
SizeT BufferSparseGetPageSize(const BufferId id);
void BufferSparseCommitChanges(const BufferId id);
CoreGraphics::DeviceAddress BufferGetDeviceAddress(const BufferId id);
void BufferCopyWithStaging(const CoreGraphics::BufferId dest, const uint offset, const void* data, const uint size);
template <class T>
inline T*
BufferMap(const BufferId id)
{
    return reinterpret_cast<T*>(BufferMap(id));
}
template<class TYPE>
inline void
BufferUpdate(const BufferId id, const TYPE& data, const uint offset)
{
    BufferUpdate(id, (const void*)&data, sizeof(TYPE), offset);
}
template<class TYPE>
inline void
BufferUpdateArray(const BufferId id, const TYPE* data, const uint count, const uint offset)
{
    BufferUpdate(id, (const void*)data, sizeof(TYPE) * count, offset);
}
template<class TYPE>
void BufferUpdateArray(const BufferId id, const Util::Array<TYPE>& data, const uint offset)
{
    BufferUpdate(id, (const void*)data.Begin(), data.ByteSize(), offset);
}
template<class TYPE>
void BufferUpdateArray(const BufferId id, const Util::FixedArray<TYPE>& data, const uint offset)
{
    BufferUpdate(id, (const void*)data.Begin(), data.ByteSize(), offset);
}
template<class TYPE>
inline void
BufferUpload(const CoreGraphics::CmdBufferId cmdBuf, const BufferId id, const TYPE* data, const uint count, const uint offset)
{
    BufferUpload(cmdBuf, id, (const void*)data, sizeof(TYPE) * count, offset);
}
struct BufferSet
{
    BufferSet() {};
    BufferSet(const BufferCreateInfo& createInfo);
    BufferSet(BufferSet&& rhs);
    void operator=(BufferSet&& rhs);
    const CoreGraphics::BufferId Buffer();
    ~BufferSet();
    Util::FixedArray<CoreGraphics::BufferId> buffers;
};
struct BufferWithStaging
{
    BufferWithStaging() {};
    BufferWithStaging(const BufferCreateInfo& createInfo);
    BufferWithStaging(BufferWithStaging&& rhs);
    void operator=(BufferWithStaging&& rhs);
    const CoreGraphics::BufferId DeviceBuffer();
    const CoreGraphics::BufferId HostBuffer();
    void Flush(const CoreGraphics::CmdBufferId cmdBuf, SizeT numBytes);
    ~BufferWithStaging();
    CoreGraphics::BufferId deviceBuffer;
    BufferSet hostBuffers;
};
}