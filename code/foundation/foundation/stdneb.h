#include "core/config.h"
#include "util/compilerhelper.h"
#if __WIN32__
#include "core/win32/precompiled.h"
#elif __linux__
#include "core/posix/precompiled.h"
#else
#error "precompiled.h не реализован на этой платформе"
#endif