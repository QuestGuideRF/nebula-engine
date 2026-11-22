#pragma once
#include "core/config.h"
#include "core/debug.h"
#include "memory/osx/osxmemoryconfig.h"
namespace Memory
{
extern int volatile TotalAllocCount;
extern int volatile TotalAllocSize;
extern int volatile HeapTypeAllocCount[NumHeapTypes];
extern int volatile HeapTypeAllocSize[NumHeapTypes];
extern bool volatile MemoryLoggingEnabled;
extern unsigned int volatile MemoryLoggingThreshold;
extern HeapType volatile MemoryLoogingHeapType;
extern void* Alloc(HeapType heapType, size_t size, size_t alignment=16);
extern void* Realloc(HeapType heapType, void* ptr, size_t size);
extern void Free(HeapType heapType, void* ptr);
extern char* DuplicateCString(const char* from);
extern bool IsOverlapping(const unsigned char* srcPtr, size_t srcSize, const unsigned char* dstPtr, size_t dstSize);
extern void Copy(const void* from, void* to, size_t numBytes);
extern void Clear(void* ptr, size_t numBytes);
extern void Fill(void* ptr, size_t numBytes, unsigned char value);
struct TotalMemoryStatus
{
    unsigned int totalPhysical;
    unsigned int availPhysical;
    unsigned int totalVirtual;
    unsigned int availVirtual;
};
extern TotalMemoryStatus GetTotalMemoryStatus();
#if NEBULA_MEMORY_STATS
void EnableMemoryLogging(unsigned int threshold, HeapType = InvalidHeapType);
void DisableMemoryLogging();
void ToggleMemoryLogging(unsigned int threshold, HeapType = InvalidHeapType);
#endif
#if NEBULA_MEMORY_ADVANCED_DEBUGGING
extern bool ValidateMemory();
extern void Checkpoint(const char* msg);
void DumpMemoryLeaks();
#endif
#if NEBULA_MEMORY_ADVANCED_DEBUGGING
#define __MEMORY_CHECKPOINT(s) Memory::Checkpoint(##s)
#else
#define __MEMORY_CHECKPOINT(s)
#endif
#define __MEMORY_VALIDATE(s)
}
#ifdef new
#undef new
#endif
#ifdef delete
#undef delete
#endif
extern void* operator new(size_t size);
extern void* operator new(size_t size, size_t align);
extern void* operator new[](size_t size);
extern void* operator new[](size_t size, size_t align);
extern void operator delete(void* p);
extern void operator delete[](void* p);
#define n_new(type) new type
#define n_new_array(type,size) new type[size]
#define n_delete(ptr) delete ptr
#define n_delete_array(ptr) delete[] ptr