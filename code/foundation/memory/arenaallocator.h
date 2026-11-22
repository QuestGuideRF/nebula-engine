#pragma once
#include "core/config.h"
#include "math/scalar.h"
#include "util/array.h"
#define SMALL_CHUNK 0x100
#define MEDIUM_CHUNK 0x1000
#define BIG_CHUNK 0x10000
namespace Memory
{
template <int ChunkSize>
class ArenaAllocator
{
public:
    ArenaAllocator();
    ~ArenaAllocator();
    ArenaAllocator(const ArenaAllocator& rhs);
    void operator=(const ArenaAllocator& rhs);
    ArenaAllocator(ArenaAllocator&& rhs);
    void operator=(ArenaAllocator&& rhs);
    template <typename T> T* Alloc();
    template <typename T> T* Alloc(SizeT num);
    void* Alloc(SizeT size);
    void NewChunk();
    void Release();
private:
    byte* currentChunk;
    byte* iterator;
    Util::Array<byte*> retiredChunks;
};
template <int ChunkSize>
inline
ArenaAllocator<ChunkSize>::ArenaAllocator() :
    currentChunk(nullptr),
    iterator(nullptr)
{
}
template <int ChunkSize>
inline
ArenaAllocator<ChunkSize>::~ArenaAllocator()
{
    this->Release();
}
template<int ChunkSize>
inline
ArenaAllocator<ChunkSize>::ArenaAllocator(ArenaAllocator&& rhs)
{
    this->retiredChunks = rhs.retiredChunks;
    this->currentChunk = rhs.currentChunk;
    this->iterator = rhs.iterator;
    rhs.retiredChunks.Clear();
    rhs.currentChunk = nullptr;
    rhs.iterator = nullptr;
}
template<int ChunkSize>
inline
ArenaAllocator<ChunkSize>::ArenaAllocator(const ArenaAllocator& rhs)
{
    this->retiredChunks = rhs.retiredChunks;
    this->currentChunk = rhs.currentChunk;
    this->iterator = rhs.iterator;
}
template<int ChunkSize>
inline void
ArenaAllocator<ChunkSize>::operator=(ArenaAllocator&& rhs)
{
    this->retiredChunks = rhs.retiredChunks;
    this->currentChunk = rhs.currentChunk;
    this->iterator = rhs.iterator;
    rhs.retiredChunks.Clear();
    rhs.currentChunk = nullptr;
    rhs.iterator = nullptr;
}
template<int ChunkSize>
inline void
ArenaAllocator<ChunkSize>::operator=(const ArenaAllocator& rhs)
{
    this->retiredChunks = rhs.retiredChunks;
    this->currentChunk = rhs.currentChunk;
    this->iterator = rhs.iterator;
}
template <int ChunkSize>
inline void
ArenaAllocator<ChunkSize>::NewChunk()
{
    if (this->currentChunk != nullptr)
        this->retiredChunks.Append(this->currentChunk);
    this->currentChunk = (byte*)Memory::Alloc(ObjectArrayHeap, ChunkSize);
    this->iterator = this->currentChunk;
}
template<int ChunkSize>
inline void ArenaAllocator<ChunkSize>::Release()
{
    IndexT i;
    for (i = 0; i < this->retiredChunks.Size(); i++)
    {
        Memory::Free(ObjectArrayHeap, this->retiredChunks[i]);
    }
    if (this->currentChunk)
        Memory::Free(ObjectArrayHeap, this->currentChunk);
    this->retiredChunks.Clear();
    this->currentChunk = nullptr;
    this->iterator = nullptr;
}
template <int ChunkSize>
template<typename T>
inline T*
ArenaAllocator<ChunkSize>::Alloc()
{
    T* ret = nullptr;
    if constexpr (sizeof(T) > ChunkSize)
    {
        byte* block = (byte*)Memory::Alloc(ObjectArrayHeap, sizeof(T));
        this->retiredChunks.Append(block);
        ret = new (block) T;
    }
    else
    {
        SizeT alignedSize = Math::align(sizeof(T), 16);
        if (this->iterator == nullptr)
        {
            this->NewChunk();
        }
        else
        {
            SizeT remainder = ChunkSize - SizeT(this->iterator - this->currentChunk);
            if (remainder < alignedSize)
                this->NewChunk();
        }
        ret = new (this->iterator) T;
        this->iterator += alignedSize;
    }
    return ret;
}
template <int ChunkSize>
template<typename T>
inline T*
ArenaAllocator<ChunkSize>::Alloc(SizeT num)
{
    T* ret = nullptr;
    if constexpr (sizeof(T) > ChunkSize)
    {
        byte* block = (byte*)Memory::Alloc(ObjectArrayHeap, sizeof(T) * num);
        this->retiredChunks.Append(block);
        ret = new (block) T[num];
    }
    else
    {
        SizeT alignedSize = Math::align(sizeof(T) * num, 16);
        if (this->iterator == nullptr)
        {
            this->NewChunk();
        }
        else
        {
            SizeT remainder = ChunkSize - SizeT(this->iterator - this->currentChunk);
            if (remainder < alignedSize)
                this->NewChunk();
        }
        ret = new (this->iterator) T[num];
        this->iterator += alignedSize;
    }
    return ret;
}
template <int ChunkSize>
inline void*
ArenaAllocator<ChunkSize>::Alloc(SizeT size)
{
    n_assert(size != 0);
    void* ret = nullptr;
    if (size > ChunkSize)
    {
        byte* block = (byte*)Memory::Alloc(ObjectArrayHeap, size);
        this->retiredChunks.Append(block);
        ret = block;
    }
    else
    {
        size = Math::align(size, 16);
        if (this->iterator == nullptr)
        {
            this->NewChunk();
        }
        else
        {
            PtrDiff remainder = this->currentChunk + ChunkSize - this->iterator;
            if (remainder < size)
                this->NewChunk();
        }
        ret = this->iterator;
        this->iterator += size;
    }
    return ret;
}
}