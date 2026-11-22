#pragma once
#include "core/types.h"
namespace Threading
{
typedef DWORD ThreadId;
static const ThreadId InvalidThreadId = 0xffffffff;
typedef int32_t ThreadIdStorage;
static_assert(sizeof(ThreadId) == sizeof(ThreadIdStorage));
}