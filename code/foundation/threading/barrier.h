#pragma once
#if (__WIN32__)
#include "threading/win360/win360barrier.h"
#elif ( __OSX__ || __APPLE__ || __linux__ )
#include "threading/posix/posixbarrier.h"
#else
#error "Барьер не реализован на этой платформе!"
#endif