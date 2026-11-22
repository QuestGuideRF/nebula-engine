#pragma once
#include "buffer.h"
namespace CoreGraphics
{
template <typename STORAGE>
struct PinnedBuffer
{
    PinnedBuffer() {};
    PinnedBuffer(const BufferCreateInfo& createInfo);
    PinnedBuffer(PinnedBuffer&& rhs);
    void operator=(PinnedBuffer&& rhs);
    void Write(const STORAGE& item, IndexT offset);
    void Write(const STORAGE* items, SizeT num, IndexT offset);
    void Flush(const CoreGraphics::CmdBufferId buf);
    uint iterator;
    Util::Array<CoreGraphics::BufferCopy> fromCopies, toCopies;
    Util::Array<uint> sizes;
    BufferSet hostBuffers;
    CoreGraphics::BufferId deviceBuffer;
};
template<typename STORAGE> inline
PinnedBuffer<STORAGE>::PinnedBuffer(const BufferCreateInfo& createInfo)
    : iterator(0)
{
    n_assert(createInfo.sparse == true);
    BufferCreateInfo hostBufferInfo = createInfo;
    hostBufferInfo.mode = CoreGraphics::HostLocal;
    hostBufferInfo.usageFlags |= CoreGraphics::TransferBufferSource;
    this->hostBuffers = BufferSet(hostBufferInfo);
    BufferCreateInfo deviceBufferInfo = createInfo;
    deviceBufferInfo.mode = CoreGraphics::DeviceLocal;
    deviceBufferInfo.usageFlags |= CoreGraphics::TransferBufferDestination;
    this->deviceBuffer = CoreGraphics::CreateBuffer(deviceBufferInfo);
}
template<typename STORAGE> inline
PinnedBuffer<STORAGE>::PinnedBuffer(PinnedBuffer&& rhs)
{
    this->hostBuffers = std::move(rhs.hostBuffers);
    if (this->deviceBuffer != InvalidBufferId)
        DestroyBuffer(this->deviceBuffer);
    this->deviceBuffer = std::move(rhs.deviceBuffer);
    this->iterator = rhs.iterator;
    rhs.iterator = 0;
}
template<typename STORAGE> inline void
PinnedBuffer<STORAGE>::operator=(PinnedBuffer&& rhs)
{
    this->hostBuffers = std::move(rhs.hostBuffers);
    if (this->deviceBuffer != InvalidBufferId)
        DestroyBuffer(this->deviceBuffer);
    this->deviceBuffer = std::move(rhs.deviceBuffer);
    this->iterator = rhs.iterator;
    rhs.iterator = 0;
}
template<typename STORAGE> inline void
PinnedBuffer<STORAGE>::Write(const STORAGE& item, IndexT offset)
{
    this->fromCopies.Append(BufferCopy{ .offset = this->iterator });
    this->toCopies.Append(BufferCopy{ .offset = offset });
    this->sizes.Append(sizeof(STORAGE));
    CoreGraphics::BufferUpdate(this->hostBuffers.Buffer(), item, this->iterator);
    this->iterator += sizeof(STORAGE);
}
template<typename STORAGE> inline void
PinnedBuffer<STORAGE>::Write(const STORAGE* items, SizeT num, IndexT offset)
{
    this->fromCopies.Append(BufferCopy{ .offset = this->iterator });
    this->toCopies.Append(BufferCopy{ .offset = offset });
    uint byteSize = sizeof(STORAGE) * num;
    this->sizes.Append(byteSize);
    CoreGraphics::BufferUpdate(this->hostBuffers.Buffer(), items, byteSize, this->iterator);
    this->iterator += byteSize;
}
template<typename STORAGE> inline void
PinnedBuffer<STORAGE>::Flush(const CoreGraphics::CmdBufferId buf)
{
    N_CMD_SCOPE(buf, NEBULA_MARKER_TRANSFER, "Обновление закрепленного буфера");
    for (auto size : this->sizes)
    {
        CoreGraphics::CmdCopy(buf, this->hostBuffers.Buffer(), this->fromCopies, this->deviceBuffer, this->toCopies, size);
    }
    this->fromCopies.Clear();
    this->toCopies.Clear();
    this->sizes.Clear();
    this->iterator = 0;
}
}