#pragma once
#include "ids/id.h"
#include "barrier.h"
#ifdef CreateSemaphore
#pragma push_macro("CreateSemaphore")
#undef CreateSemaphore
#endif
namespace CoreGraphics
{
ID_24_8_TYPE(SemaphoreId);
enum SemaphoreType
{
    Binary,
    Timeline
};
struct SemaphoreCreateInfo
{
#if NEBULA_GRAPHICS_DEBUG
    const char* name = nullptr;
#endif
    SemaphoreType type;
};
SemaphoreId CreateSemaphore(const SemaphoreCreateInfo& info);
void DestroySemaphore(const SemaphoreId& semaphore);
uint64_t SemaphoreGetValue(const SemaphoreId& semaphore);
void SemaphoreSignal(const SemaphoreId& semaphore);
void SemaphoreReset(const SemaphoreId& semaphore);
}
#pragma pop_macro("CreateSemaphore")