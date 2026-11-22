#pragma once
#include "core/config.h"
namespace Memory
{
enum HeapType
{
    DefaultHeap = 0,
    ObjectHeap,
    ObjectArrayHeap,
    ResourceHeap,
    ScratchHeap,
    StringDataHeap,
    StreamDataHeap,
    PhysicsHeap,
    AppHeap,
    NumHeapTypes,
    InvalidHeapType,
};
extern malloc_zone_t* Heaps[NumHeapTypes];
extern void SetupHeaps();
extern const char* GetHeapTypeName(HeapType heapType);
}