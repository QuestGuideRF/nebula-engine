//------------------------------------------------------------------------------
//  vkbuffer.cc
//  (C) 2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "vkgraphicsdevice.h"
#include "vkcommandbuffer.h"
#include "vkbuffer.h"
#include "util/bit.h"
namespace Vulkan
{
VkBufferAllocator bufferAllocator;
VkBufferSparseExtensionAllocator bufferSparseExtensionAllocator;

//------------------------------------------------------------------------------
/**
*/
VkBuffer
BufferGetVk(const CoreGraphics::BufferId id)
{
    return bufferAllocator.ConstGet<Buffer_RuntimeInfo>(id.id).buf;
}

//------------------------------------------------------------------------------
/**
*/
VkDeviceMemory
BufferGetVkMemory(const CoreGraphics::BufferId id)
{
    return bufferAllocator.ConstGet<Buffer_LoadInfo>(id.id).mem.mem;
}

//------------------------------------------------------------------------------
/**
*/
VkDevice
BufferGetVkDevice(const CoreGraphics::BufferId id)
{
    return bufferAllocator.ConstGet<Buffer_LoadInfo>(id.id).dev;
}

} // пространство имен Вулкан
namespace CoreGraphics
{

using namespace Vulkan;
_IMPL_ACQUIRE_RELEASE(BufferId, bufferAllocator);

//------------------------------------------------------------------------------
/**
*/
const BufferId
CreateBuffer(const BufferCreateInfo& info)
{
    Ids::Id32 id = bufferAllocator.Alloc();
    VkBufferLoadInfo& loadInfo = bufferAllocator.Get<Buffer_LoadInfo>(id);
    VkBufferRuntimeInfo& runtimeInfo = bufferAllocator.Get<Buffer_RuntimeInfo>(id);
    VkBufferMapInfo& mapInfo = bufferAllocator.Get<Buffer_MapInfo>(id);

    loadInfo.sparseExtension = 0xFFFFFFFF;
    loadInfo.dev = Vulkan::GetCurrentDevice();
    runtimeInfo.usageFlags = info.usageFlags;

    VkBufferUsageFlags flags = 0;
    VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    Util::Set<uint32_t> queues;
    if (info.queueSupport != AutomaticQueueSupport)
    {
        if (info.queueSupport & GraphicsQueueSupport)
            queues.Add(CoreGraphics::GetQueueIndex(GraphicsQueueType));
        if (info.queueSupport & ComputeQueueSupport)
            queues.Add(CoreGraphics::GetQueueIndex(ComputeQueueType));
        if (info.queueSupport & TransferQueueSupport)
            queues.Add(CoreGraphics::GetQueueIndex(TransferQueueType));
    }
    else
    {
        if (info.usageFlags & CoreGraphics::TransferBufferSource)
        {
            queues.Add(CoreGraphics::GetQueueIndex(GraphicsQueueType));
            queues.Add(CoreGraphics::GetQueueIndex(TransferQueueType));
        }
        if (info.usageFlags & CoreGraphics::TransferBufferDestination)
        {
            queues.Add(CoreGraphics::GetQueueIndex(GraphicsQueueType));
            queues.Add(CoreGraphics::GetQueueIndex(TransferQueueType));
        }
    }

    constexpr uint UsageLookup[] =
    {
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR
    };

    flags = Util::BitmaskConvert(info.usageFlags, UsageLookup);
    VkBufferCreateFlags createFlags = 0x0;

    // принудительно добавить бит назначения, если у нас есть данные для загрузки
    if (info.mode == DeviceLocal && info.dataSize != 0)
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    if (queues.Size() > 1)
        sharingMode = VK_SHARING_MODE_CONCURRENT;
    if (info.sparse)
    {
        createFlags |= VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT;
        n_assert(info.data == nullptr);
    }

    // начни с создания буфера
    uint size = info.byteSize == 0 ? info.size * info.elementSize : info.byteSize;
    VkBufferCreateInfo bufinfo =
    {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        NULL,
        createFlags,												// использовать для разреженных буферов
        size,
        flags,
        sharingMode,												// доступны только из очереди создателей,
        (uint)queues.Size(),												// количество очередей в семье
        queues.Size() > 0 ? queues.KeysAsArray().Begin() : nullptr	// массив очередей, принадлежащих семейству
    };

    VkResult err = vkCreateBuffer(loadInfo.dev, &bufinfo, NULL, &runtimeInfo.buf);
    n_assert(err == VK_SUCCESS);

    CoreGraphics::MemoryPoolType pool = CoreGraphics::MemoryPool_DeviceLocal;
    if (info.mode == DeviceLocal)
        pool = CoreGraphics::MemoryPool_DeviceLocal;
    else if (info.mode == HostLocal)
        pool = CoreGraphics::MemoryPool_HostLocal;
    else if (info.mode == DeviceAndHost)
        pool = CoreGraphics::MemoryPool_DeviceAndHost;
    else if (info.mode == HostCached)
        pool = CoreGraphics::MemoryPool_HostCached;


    VkMemoryRequirements memoryReqs;
    vkGetBufferMemoryRequirements(loadInfo.dev, runtimeInfo.buf, &memoryReqs);

    uint baseAlignment = memoryReqs.alignment;
    if (AllBits(info.usageFlags, CoreGraphics::AccelerationStructureInstances))
        baseAlignment = 16;
    if (AllBits(info.usageFlags, CoreGraphics::ShaderTable))
        baseAlignment = Math::max(baseAlignment, CoreGraphics::ShaderGroupAlignment);

    if (info.sparse)
    {
        Ids::Id32 sparseExtension = bufferSparseExtensionAllocator.Alloc();
        loadInfo.sparseExtension = sparseExtension;
        BufferSparsePageTable& table = bufferSparseExtensionAllocator.Get<BufferExtension_SparsePageTable>(sparseExtension);

        n_assert(memoryReqs.size < CoreGraphics::SparseAddressSize);

        table.memoryReqs = memoryReqs;
        table.bindCounts = size / baseAlignment;
        table.pages.Resize(table.bindCounts);

        SizeT offset = 0;
        for (auto& page : table.pages)
        {
            CoreGraphics::BufferSparsePage p;
            p.offset = offset;
            p.alloc = CoreGraphics::Alloc{ VK_NULL_HANDLE, 0, 0, CoreGraphics::MemoryPool_DeviceLocal };
            offset += memoryReqs.alignment;
        }
    }
    else
    {
        // теперь привяжите память к буферу
        CoreGraphics::Alloc alloc = AllocateMemory(loadInfo.dev, runtimeInfo.buf, pool, baseAlignment);
        err = vkBindBufferMemory(loadInfo.dev, runtimeInfo.buf, alloc.mem, alloc.offset);
        n_assert(err == VK_SUCCESS);

        loadInfo.mem = alloc;

        if (info.mode == HostLocal || info.mode == HostCached || info.mode == DeviceAndHost)
        {
            // скопировать содержимое и очистить память
            char* data = (char*)GetMappedMemory(alloc);
            mapInfo.mappedMemory = data;

            // if we have data, copy the memory to the region
            if (info.data)
            {
                n_assert(info.dataSize <= bufinfo.size);
                memcpy(data, info.data, info.dataSize);

                // if not host-local memory, we need to flush the initial update
                if (info.mode == HostCached || info.mode == DeviceAndHost)
                {
                    VkMappedMemoryRange range;
                    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                    range.pNext = nullptr;
                    range.offset = Math::align_down(alloc.offset, CoreGraphics::MemoryRangeGranularity);
                    range.size = Math::align(alloc.size, CoreGraphics::MemoryRangeGranularity);
                    range.memory = alloc.mem;
                    VkResult res = vkFlushMappedMemoryRanges(loadInfo.dev, 1, &range);
                    n_assert(res == VK_SUCCESS);
                }
            }
        }
        else if (info.mode == DeviceLocal && info.data != nullptr)
        {
            // if device local and we provide a data pointer, create a temporary staging buffer and perform a copy
            VkBufferCreateInfo tempAllocInfo =
            {
                VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                NULL,
                0,									// использовать для разреженных буферов
                info.dataSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_SHARING_MODE_EXCLUSIVE,			// доступны только из очереди создателей,
                0,									// количество очередей в семье
                nullptr								// массив очередей, принадлежащих семейству
            };

            // создать временный буфер
            VkBuffer tempBuffer;
            err = vkCreateBuffer(loadInfo.dev, &tempAllocInfo, nullptr, &tempBuffer);

            // выделите для него некоторую локальную временную память
            CoreGraphics::Alloc tempAlloc = AllocateMemory(loadInfo.dev, tempBuffer, CoreGraphics::MemoryPool_HostLocal);
            err = vkBindBufferMemory(loadInfo.dev, tempBuffer, tempAlloc.mem, tempAlloc.offset);
            n_assert(err == VK_SUCCESS);

            // скопировать данные во временный буфер
            char* buf = (char*)GetMappedMemory(tempAlloc);
            memcpy(buf, info.data, info.dataSize);

            CoreGraphics::CmdBufferId cmd = CoreGraphics::LockGraphicsSetupCommandBuffer("Загрузка буфера");
            VkBufferCopy copy;
            copy.dstOffset = 0;
            copy.srcOffset = 0;
            copy.size = info.dataSize;

            // копировать из временного буфера в исходный буфер в контексте отправки ресурса
            vkCmdCopyBuffer(Vulkan::CmdBufferGetVk(cmd), tempBuffer, runtimeInfo.buf, 1, &copy);

            // добавить отложенное удаление для этого временного буфера
            Vulkan::DelayedDeleteVkBuffer(loadInfo.dev, tempBuffer);
            CoreGraphics::DelayedFreeMemory(tempAlloc);
            CoreGraphics::UnlockGraphicsSetupCommandBuffer(cmd);
        }
    }

    // настройка ресурса
    loadInfo.mode = info.mode;
    loadInfo.size = info.size;
    loadInfo.byteSize = size;
    loadInfo.elementSize = info.elementSize;

    BufferId ret = id;

#if NEBULA_GRAPHICS_DEBUG
    ObjectSetName(ret, info.name.Value());
#endif

    CoreGraphics::BufferIdRelease(ret);

    return ret;
}

//------------------------------------------------------------------------------
/**
*/
void
DestroyBuffer(const BufferId id)
{
    __Lock(bufferAllocator, id.id);
    VkBufferLoadInfo& loadInfo = bufferAllocator.Get<Buffer_LoadInfo>(id.id);

    CoreGraphics::DelayedDeleteBuffer(id);
    CoreGraphics::DelayedFreeMemory(loadInfo.mem);
    loadInfo.mem = CoreGraphics::Alloc{};
    bufferAllocator.Dealloc(id.id);
}

//------------------------------------------------------------------------------
/**
*/
const BufferUsageFlags
BufferGetType(const BufferId id)
{
    return bufferAllocator.ConstGet<Buffer_RuntimeInfo>(id.id).usageFlags;
}

//------------------------------------------------------------------------------
/**
*/
const uint64_t
BufferGetSize(const BufferId id)
{
    return bufferAllocator.ConstGet<Buffer_LoadInfo>(id.id).size;
}

//------------------------------------------------------------------------------
/**
*/
const uint64_t
BufferGetElementSize(const BufferId id)
{
    return bufferAllocator.ConstGet<Buffer_LoadInfo>(id.id).elementSize;
}

//------------------------------------------------------------------------------
/**
*/
const uint64_t
BufferGetByteSize(const BufferId id)
{
    return bufferAllocator.ConstGet<Buffer_LoadInfo>(id.id).byteSize;
}

//------------------------------------------------------------------------------
/**
*/
const uint64_t
BufferGetUploadMaxSize()
{
    return 65536;
}

//------------------------------------------------------------------------------
/**
*/
void*
BufferMap(const BufferId id)
{
    const VkBufferMapInfo& mapInfo = bufferAllocator.ConstGet<Buffer_MapInfo>(id.id);
    n_assert2(mapInfo.mappedMemory != nullptr, "Буфер должен быть создан как динамический или сопоставлен для поддержки сопоставления.");
    return mapInfo.mappedMemory;
}

//------------------------------------------------------------------------------
/**
*/
void
BufferUnmap(const BufferId id)
{
    // Ничего не делайте на Вулкане
}

//------------------------------------------------------------------------------
/**
*/
void
BufferUpdate(const BufferId id, const void* data, const uint size, const uint offset)
{
    const VkBufferMapInfo& map = bufferAllocator.ConstGet<Buffer_MapInfo>(id.id);

#if NEBULA_DEBUG
    const VkBufferLoadInfo& setup = bufferAllocator.ConstGet<Buffer_LoadInfo>(id.id);
    n_assert(size + offset <= (uint)setup.byteSize);
#endif
    byte* buf = (byte*)map.mappedMemory + offset;
    memcpy(buf, data, size);
}

//------------------------------------------------------------------------------
/**
*/
void
BufferUpload(const CoreGraphics::CmdBufferId cmdBuf, const BufferId id, const void* data, const uint size, const uint offset)
{
    n_assert(size <= (uint)BufferGetUploadMaxSize());
    CoreGraphics::CmdUpdateBuffer(cmdBuf, id, offset, size, data);
}

//------------------------------------------------------------------------------
/**
*/
void
BufferFill(const CoreGraphics::CmdBufferId cmdBuf, const BufferId id, char pattern)
{
    const VkBufferLoadInfo& setup = bufferAllocator.ConstGet<Buffer_LoadInfo>(id.id);

    uint64_t remainingBytes = setup.byteSize;
    uint numChunks = Math::divandroundup(setup.byteSize, BufferGetUploadMaxSize());
    int chunkOffset = 0;
    for (uint i = 0; i < numChunks; i++)
    {
        int chunkSize = Math::min(remainingBytes, BufferGetUploadMaxSize());
        char* buf = new char[chunkSize];
        memset(buf, pattern, chunkSize);
        vkCmdUpdateBuffer(Vulkan::CmdBufferGetVk(cmdBuf), Vulkan::BufferGetVk(id), chunkOffset, chunkSize, buf);
        chunkOffset += chunkSize;
        remainingBytes -= chunkSize;
        delete[] buf;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
BufferFlush(const BufferId id, uint64_t offset, uint64_t size)
{
    const VkBufferLoadInfo& loadInfo = bufferAllocator.ConstGet<Buffer_LoadInfo>(id.id);
    n_assert(size == NEBULA_WHOLE_BUFFER_SIZE ? true : (uint)offset + size <= loadInfo.byteSize);
    Flush(loadInfo.dev, loadInfo.mem, offset, size);
}

//------------------------------------------------------------------------------
/**
*/
void
BufferInvalidate(const BufferId id, uint64_t offset, uint64_t size)
{
    const VkBufferLoadInfo& loadInfo = bufferAllocator.ConstGet<Buffer_LoadInfo>(id.id);
    n_assert(size == NEBULA_WHOLE_BUFFER_SIZE ? true : (uint)offset + size <= loadInfo.byteSize);
    Invalidate(loadInfo.dev, loadInfo.mem, offset, size);
}

//------------------------------------------------------------------------------
/**
*/
void
BufferSparseEvict(const BufferId id, IndexT pageIndex)
{
    __Lock(bufferAllocator, id.id);
    Ids::Id32 sparseExtension = bufferAllocator.ConstGet<Buffer_LoadInfo>(id.id).sparseExtension;
    n_assert(sparseExtension != 0xFFFFFFFF);
    const BufferSparsePageTable& table = bufferSparseExtensionAllocator.ConstGet<BufferExtension_SparsePageTable>(sparseExtension);

    BufferSparsePage& page = table.pages[pageIndex];
    if (page.alloc.mem == VK_NULL_HANDLE)
        return;

    // освободить память
    CoreGraphics::FreeMemory(page.alloc);
    page.alloc.mem = VK_NULL_HANDLE;
    page.alloc.offset = 0;
    VkSparseMemoryBind binding =
    {
        .resourceOffset = pageIndex * table.memoryReqs.alignment,
        .size = table.memoryReqs.alignment,
        .memory = VK_NULL_HANDLE,
        .memoryOffset = 0,
        .flags = 0x0
    };

    // добавить ожидающее обновление страницы
    Util::Array<VkSparseMemoryBind>& pageBinds = bufferSparseExtensionAllocator.Get<BufferExtension_SparsePendingBinds>(sparseExtension);
    pageBinds.Append(binding);
}

//------------------------------------------------------------------------------
/**
*/
void
BufferSparseMakeResident(const BufferId id, IndexT pageIndex)
{
    __Lock(bufferAllocator, id.id);
    Ids::Id32 sparseExtension = bufferAllocator.ConstGet<Buffer_LoadInfo>(id.id).sparseExtension;
    n_assert(sparseExtension != 0xFFFFFFFF);
    const BufferSparsePageTable& table = bufferSparseExtensionAllocator.ConstGet<BufferExtension_SparsePageTable>(sparseExtension);

    BufferSparsePage& page = table.pages[pageIndex];
    if (page.alloc.mem != VK_NULL_HANDLE)
        return;

    VkDevice dev = GetCurrentDevice();
    page.alloc = Vulkan::AllocateMemory(dev, table.memoryReqs, table.memoryReqs.alignment);
    VkSparseMemoryBind binding =
    {
        .resourceOffset = pageIndex * table.memoryReqs.alignment,
        .size = table.memoryReqs.alignment,
        .memory = page.alloc.mem,
        .memoryOffset = page.alloc.offset,
        .flags = 0x0
    };

    Util::Array<VkSparseMemoryBind>& pageBinds = bufferSparseExtensionAllocator.Get<BufferExtension_SparsePendingBinds>(sparseExtension);
    pageBinds.Append(binding);
}

//------------------------------------------------------------------------------
/**
*/
IndexT
BufferSparseGetPageIndex(const BufferId id, SizeT offset)
{
    __Lock(bufferAllocator, id.id);
    Ids::Id32 sparseExtension = bufferAllocator.ConstGet<Buffer_LoadInfo>(id.id).sparseExtension;
    n_assert(sparseExtension != 0xFFFFFFFF);
    const BufferSparsePageTable& table = bufferSparseExtensionAllocator.ConstGet<BufferExtension_SparsePageTable>(sparseExtension);

    // The page index is simply the offset divided by the page size (which is stored in alignment)
    return offset / table.memoryReqs.alignment;
}

//------------------------------------------------------------------------------
/**
*/
SizeT
BufferSparseGetPageSize(const BufferId id)
{
    __Lock(bufferAllocator, id.id);
    Ids::Id32 sparseExtension = bufferAllocator.ConstGet<Buffer_LoadInfo>(id.id).sparseExtension;
    n_assert(sparseExtension != 0xFFFFFFFF);
    const BufferSparsePageTable& table = bufferSparseExtensionAllocator.ConstGet<BufferExtension_SparsePageTable>(sparseExtension);
    return table.memoryReqs.alignment;
}

//------------------------------------------------------------------------------
/**
*/
void
BufferSparseCommitChanges(const BufferId id)
{
    __Lock(bufferAllocator, id.id);
    VkBuffer buf = bufferAllocator.ConstGet<Buffer_RuntimeInfo>(id.id).buf;
    Ids::Id32 sparseExtension = bufferAllocator.ConstGet<Buffer_LoadInfo>(id.id).sparseExtension;
    n_assert(sparseExtension != 0xFFFFFFFF);
    const BufferSparsePageTable& table = bufferSparseExtensionAllocator.ConstGet<BufferExtension_SparsePageTable>(sparseExtension);

    Util::Array<VkSparseMemoryBind>& pageBinds = bufferSparseExtensionAllocator.Get<BufferExtension_SparsePendingBinds>(sparseExtension);

    // прервать досрочно, если у нас нет обновлений
    if (pageBinds.IsEmpty())
        return;

    // выполнить разреженную привязку, вызов привязки
    Vulkan::SparseBufferBind(buf, pageBinds);

    // очистить все ожидающие привязки
    pageBinds.Clear();
}

//------------------------------------------------------------------------------
/**
*/
CoreGraphics::DeviceAddress
BufferGetDeviceAddress(const BufferId id)
{
    VkDevice dev = BufferGetVkDevice(id);
    VkBufferDeviceAddressInfo deviceAddress =
    {
        VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        nullptr,
        BufferGetVk(id)
    };
    return vkGetBufferDeviceAddress(dev, &deviceAddress);
}

//------------------------------------------------------------------------------
/**
*/
void
BufferCopyWithStaging(const CoreGraphics::BufferId dest, const uint offset, const void* data, const uint size)
{
    // Создать буфер для копирования
    CoreGraphics::BufferCreateInfo bufInfo;
    bufInfo.byteSize = size;
    bufInfo.usageFlags = CoreGraphics::BufferUsageFlag::TransferBufferSource;
    bufInfo.mode = CoreGraphics::BufferAccessMode::HostLocal;
    bufInfo.queueSupport = CoreGraphics::GraphicsQueueSupport;
    bufInfo.data = data;
    bufInfo.dataSize = size;
    CoreGraphics::BufferId buf = CoreGraphics::CreateBuffer(bufInfo);

    // Выполнить копирование в очередь установки
    CoreGraphics::BufferCopy from, to;
    from.offset = 0;
    to.offset = offset;
    CoreGraphics::CmdBufferId cmdBuf = CoreGraphics::LockGraphicsSetupCommandBuffer("Загрузка промежуточного буфера");
    CoreGraphics::CmdCopy(cmdBuf, buf, { from }, dest, { to }, size);
    CoreGraphics::UnlockGraphicsSetupCommandBuffer(cmdBuf);

    // Уничтожить буфер
    CoreGraphics::DestroyBuffer(buf);
}

} // пространство имен CoreGraphics
