#pragma once
#include "ids/id.h"
#include "coregraphics/config.h"
namespace CoreGraphics
{
ID_24_8_TYPE(FenceId);
struct FenceCreateInfo
{
    bool createSignaled : 1;
};
FenceId CreateFence(const FenceCreateInfo& info);
void DestroyFence(const FenceId id);
bool FencePeek(const FenceId id);
bool FenceReset(const FenceId id);
bool FenceWait(const FenceId id, const uint64_t time);
bool FenceWaitAndReset(const FenceId id, const uint64_t time);
}