#pragma once
#ifndef MEMORY_POSIXMEMORYCONFIG_H
#define MEMORY_POSIXMEMORYCONFIG_H
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
    NetworkHeap,
    ScriptingHeap,
    NumHeapTypes,
    InvalidHeapType,
};
extern void* volatile Heaps[NumHeapTypes];
extern void SetupHeaps();
extern const char* GetHeapTypeName(HeapType heapType);
}
#endif