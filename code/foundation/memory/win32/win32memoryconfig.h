#pragma once
#include "core/config.h"
#include "core/debug.h"
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
extern HANDLE volatile Heaps[NumHeapTypes];
extern void SetupHeaps();
extern const char* GetHeapTypeName(HeapType heapType);
__forceinline unsigned char*
__HeapAlignPointerAndWritePadding16(unsigned char* ptr)
{
    unsigned char paddingMask = size_t(ptr) & 15;
    ptr = (unsigned char*)(size_t(ptr + 16) & ~15);
    ptr[-1] = paddingMask;
    return ptr;
}
__forceinline unsigned char*
__HeapUnalignPointer16(unsigned char* ptr)
{
    return (unsigned char*)(size_t(ptr - 16) | ptr[-1]);
}
__forceinline LPVOID
__HeapAlloc16(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes)
{
    unsigned char* ptr = (unsigned char*) ::HeapAlloc(hHeap, dwFlags, dwBytes + 16);
    ptr = __HeapAlignPointerAndWritePadding16(ptr);
    return (LPVOID) ptr;
}
__forceinline LPVOID
__HeapReAlloc16(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes)
{
    unsigned char* ptr = (unsigned char*) lpMem;
    unsigned char* rawPtr = __HeapUnalignPointer16(ptr);
    ptr = (unsigned char*) ::HeapReAlloc(hHeap, (dwFlags | HEAP_REALLOC_IN_PLACE_ONLY), rawPtr, dwBytes + 16);
    if (0 == ptr)
    {
        SIZE_T rawSize = ::HeapSize(hHeap, dwFlags, rawPtr);
        ptr = (unsigned char*) ::HeapAlloc(hHeap, dwFlags, dwBytes + 16);
        ptr = __HeapAlignPointerAndWritePadding16(ptr);
        SIZE_T copySize = dwBytes <= (rawSize - 16) ? dwBytes : (rawSize - 16);
        ::CopyMemory(ptr, lpMem, copySize);
        ::HeapFree(hHeap, dwFlags, rawPtr);
    }
    else
    {
        ptr = __HeapAlignPointerAndWritePadding16(ptr);
    }
    return (LPVOID) ptr;
}
__forceinline BOOL
__HeapFree16(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem)
{
    unsigned char* ptr = (unsigned char*) lpMem;
    ptr = __HeapUnalignPointer16(ptr);
    return ::HeapFree(hHeap, dwFlags, ptr);
}
__forceinline SIZE_T
__HeapSize16(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem)
{
    unsigned char* ptr = (unsigned char*) lpMem;
    ptr = __HeapUnalignPointer16(ptr);
    return ::HeapSize(hHeap, dwFlags, ptr);
}
}