#pragma once
#if __WIN32__
#include "core/win32/win32singleton.h"
#elif __OSX__
#include "core/osx/osxsingleton.h"
#elif __linux__
#include "core/posix/posixsingleton.h"
#else
#error "РЕАЛИЗУЙТЕ МЕНЯ!"
#endif