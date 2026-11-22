#pragma once
#if __WIN32__
#include "threading/win32/win32threadid.h"
#elif __OSX__
#include "threading/osx/osxthreadid.h"
#elif __linux__
#include "threading/linux/linuxthreadid.h"
#else
#error "Threading::ThreadId не реализован на этой платформе!"
#endif