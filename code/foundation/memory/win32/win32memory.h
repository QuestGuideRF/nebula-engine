#pragma once
#include "core/config.h"
#include "core/debug.h"
#include "threading/interlocked.h"
#include "memory/win32/win32memoryconfig.h"
#include "memory/win32/winmemory.h"
#include <new>
#pragma warning (disable : 4595)
namespace Memory
{
extern long volatile TotalAllocCount;
extern long volatile TotalAllocSize;
extern long volatile HeapTypeAllocCount[NumHeapTypes];
extern long volatile HeapTypeAllocSize[NumHeapTypes];
extern unsigned int volatile MemoryLoggingThreshold;
extern HeapType volatile MemoryLoggingHeapType;
extern void* Alloc(HeapType heapType, size_t size, size_t align = 16);
extern void* Realloc(HeapType heapType, void* ptr, size_t size);
extern void Free(HeapType heapType, void* ptr);
extern void* AllocVirtual(size_t size);
extern void CommitVirtual(void* ptr, size_t size);
extern void DecommitVirtual(void* ptr, size_t size);
extern void FreeVirtual(void* ptr, size_t size);
extern char* DuplicateCString(const char* from);
extern bool IsOverlapping(const unsigned char* srcPtr, size_t srcSize, const unsigned char* dstPtr, size_t dstSize);
struct TotalMemoryStatus
{
    unsigned int totalPhysical;
    unsigned int availPhysical;
    unsigned int totalVirtual;
    unsigned int availVirtual;
};
extern TotalMemoryStatus GetTotalMemoryStatus();
extern void DumpTotalMemoryStatus();
#if NEBULA_MEMORY_ADVANCED_DEBUGGING
extern bool ValidateMemory();
extern void Checkpoint(const char* msg);
void EnableMemoryLogging(unsigned int threshold, HeapType heapType = InvalidHeapType);
void DisableMemoryLogging();
void ToggleMemoryLogging(unsigned int threshold, HeapType heapType = InvalidHeapType);
void DumpMemoryLeaks();
#endif
#if NEBULA_MEMORY_ADVANCED_DEBUGGING
#define __MEMORY_CHECKPOINT(s) Memory::Checkpoint(##s)
#else
#define __MEMORY_CHECKPOINT(s)
#endif
#define __MEMORY_VALIDATE(s)
}