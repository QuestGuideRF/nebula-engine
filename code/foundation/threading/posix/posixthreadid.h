#pragma once
#include "core/types.h"
namespace Threading
{
typedef pthread_t ThreadId;
static const ThreadId InvalidThreadId = 0;
typedef int64_t ThreadIdStorage;
static_assert(sizeof(ThreadId) == sizeof(ThreadIdStorage));
}