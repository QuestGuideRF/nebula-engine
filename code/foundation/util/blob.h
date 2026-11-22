#pragma once
#include "core/types.h"
#include "memory/heap.h"
#include "memory/poolarrayallocator.h"
#include "io/uri.h"
namespace Util
{
class Blob
{
public:
    static void Setup();
    static void Shutdown();
    void* operator new(size_t s);
    void operator delete(void* ptr);
    Blob();
    Blob(std::nullptr_t t);
    Blob(const void* ptr, size_t size);
    Blob(size_t size);
    Blob(const Blob& rhs);
    Blob(Blob&& rhs) noexcept;
    ~Blob();
    void operator=(const Blob& rhs);
    void operator=(Blob&& rhs) noexcept;
    bool operator==(const Blob& rhs) const;
    bool operator!=(const Blob& rhs) const;
    bool operator>(const Blob& rhs) const;
    bool operator<(const Blob& rhs) const;
    bool operator>=(const Blob& rhs) const;
    bool operator<=(const Blob& rhs) const;
    bool IsValid() const;
    void Reserve(size_t size);
    void Trim(size_t size);
    void Set(const void* ptr, size_t size);
    void SetFromBase64(const void* ptr, size_t size);
    void SetFromFile(const IO::URI & uri);
    void SetChunk(const void* from, size_t size, size_t internalOffset);
    void* GetPtr() const;
    size_t Size() const;
    uint32_t HashCode() const;
    Util::Blob GetBase64() const;
private:
    void Delete();
    void Allocate(size_t size);
    void Copy(const void* ptr, size_t size);
    void GrowTo(size_t size);
    int BinaryCompare(const Blob& rhs) const;
    static Memory::Heap* DataHeap;
    void* ptr = nullptr;
    size_t size = 0;
    size_t allocSize = 0;
};
__forceinline void*
Blob::operator new(size_t size)
{
#if NEBULA_DEBUG
    n_assert(size == sizeof(Blob));
#endif
    return Memory::Alloc(Memory::ObjectHeap, size);
}
__forceinline void
Blob::operator delete(void* ptr)
{
    return Memory::Free(Memory::ObjectHeap, ptr);
}
inline void
Blob::Setup()
{
    n_assert(0 == DataHeap);
    DataHeap = new Memory::Heap("Util.Blob.DataHeap");
}
inline void
Blob::Shutdown()
{
    n_assert(0 != DataHeap);
    delete DataHeap;
    DataHeap = 0;
}
inline
Blob::Blob() :
    ptr(0),
    size(0),
    allocSize(0)
{
}
inline
Blob::Blob(std::nullptr_t t) :
    ptr(0),
    size(0),
    allocSize(0)
{
}
inline
Blob::Blob(const void* fromPtr, size_t fromSize) :
    ptr(0),
    size(0),
    allocSize(0)
{
    this->Copy(fromPtr, fromSize);
}
inline
Blob::Blob(const Blob& rhs) :
    ptr(0),
    size(0),
    allocSize(0)
{
    if (rhs.IsValid())
    {
        this->Copy(rhs.ptr, rhs.size);
    }
}
inline
Blob::Blob(Blob&& rhs) noexcept :
    ptr(0),
    size(0),
    allocSize(0)
{
    if (rhs.IsValid())
    {
        this->ptr = rhs.ptr;
        this->size = rhs.size;
        this->allocSize = rhs.allocSize;
        rhs.ptr = nullptr;
        rhs.size = 0;
        rhs.allocSize = 0;
    }
}
inline
Blob::Blob(size_t s) :
    ptr(0),
    size(0),
    allocSize(0)
{
    this->Allocate(s);
}
inline bool
Blob::IsValid() const
{
    return (0 != this->ptr);
}
inline void
Blob::Delete()
{
    if (this->IsValid())
    {
        n_assert(0 != DataHeap);
        DataHeap->Free((void*)this->ptr);
        this->ptr = 0;
        this->size = 0;
        this->allocSize = 0;
    }
}
inline
Blob::~Blob()
{
    this->Delete();
}
inline void
Blob::Allocate(size_t s)
{
    n_assert(!this->IsValid());
    n_assert(0 != DataHeap);
    this->ptr = DataHeap->Alloc(s);
    this->allocSize = s;
    this->size = s;
}
inline void
Blob::Copy(const void* fromPtr, size_t fromSize)
{
    n_assert((0 != fromPtr) && (fromSize > 0));
    if ((0 == this->ptr) || (this->allocSize < fromSize))
    {
        this->Delete();
        this->Allocate(fromSize);
    }
    this->size = fromSize;
    Memory::Copy(fromPtr, (void*)this->ptr, fromSize);
}
inline void
Blob::GrowTo(size_t size)
{
    n_assert(this->allocSize < size);
    void* newPtr = DataHeap->Alloc(size);
    Memory::Copy(this->ptr, newPtr, this->size);
    this->Delete();
    this->ptr = newPtr;
    this->allocSize = size;
}
inline void
Blob::operator=(const Blob& rhs)
{
    if (rhs.IsValid())
    {
        this->Copy(rhs.ptr, rhs.size);
    }
}
inline void
Blob::operator=(Blob&& rhs) noexcept
{
    if (this != &rhs)
    {
        if (this->IsValid())
        {
            this->Delete();
        }
        this->ptr = rhs.ptr;
        this->size = rhs.size;
        this->allocSize = rhs.allocSize;
        rhs.ptr = nullptr;
        rhs.size = 0;
        rhs.allocSize = 0;
    }
}
inline bool
Blob::operator==(const Blob& rhs) const
{
    return (this->BinaryCompare(rhs) == 0);
}
inline bool
Blob::operator!=(const Blob& rhs) const
{
    return (this->BinaryCompare(rhs) != 0);
}
inline bool
Blob::operator>(const Blob& rhs) const
{
    return (this->BinaryCompare(rhs) > 0);
}
inline bool
Blob::operator<(const Blob& rhs) const
{
    return (this->BinaryCompare(rhs) < 0);
}
inline bool
Blob::operator>=(const Blob& rhs) const
{
    return (this->BinaryCompare(rhs) >= 0);
}
inline bool
Blob::operator<=(const Blob& rhs) const
{
    return (this->BinaryCompare(rhs) <= 0);
}
inline void
Blob::Reserve(size_t s)
{
    if (this->IsValid())
    {
        if (this->allocSize < s)
        {
            this->Delete();
            this->Allocate(s);
        }
    }
    else
    {
        this->Allocate(s);
    }
    this->size = s;
}
inline void
Blob::Trim(size_t trimSize)
{
    n_assert(trimSize <= this->size);
    this->size = trimSize;
}
inline void
Blob::Set(const void* fromPtr, size_t fromSize)
{
    this->Copy(fromPtr, fromSize);
}
inline void
Blob::SetChunk(const void* from, size_t size, size_t internalOffset)
{
    n_assert((0 != from) && (size > 0));
    n_assert(nullptr != this->ptr)
    size_t newSize = (internalOffset + size);
    if (newSize > this->allocSize)
    {
        this->GrowTo(newSize);
    }
    Memory::Copy(from, (void*)((byte*)this->ptr + internalOffset), size);
}
inline void*
Blob::GetPtr() const
{
    n_assert(this->IsValid());
    return this->ptr;
}
inline size_t
Blob::Size() const
{
    n_assert(this->IsValid());
    return this->size;
}
inline uint32_t
Blob::HashCode() const
{
    IndexT hash = 0;
    const char* charPtr = (const char*) this->ptr;
    for (size_t i = 0; i < this->size; i++)
    {
        hash += charPtr[i];
        hash += hash << 10;
        hash ^= hash >>  6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    hash &= ~(1<<31);
    return hash;
}
}