#pragma once
#include "core/types.h"
namespace Linux
{
class LinuxThreadLocalData
{
public:
    static void SetupMainThread();
    static IndexT RegisterSlot();
    static void SetPointer(IndexT slot, void* ptr);
    static void* GetPointer(IndexT slot);
    static void ClearPointer(IndexT slot);
private:
    static void** GetThreadLocalTable();
    static const int MaxNumSlots = 4096;
    static pthread_key_t key;
    static IndexT volatile slot;
};
}