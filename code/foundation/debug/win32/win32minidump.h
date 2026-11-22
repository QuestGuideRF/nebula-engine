#pragma once
#ifndef WIN32_WIN32MINIDUMP_H
#define WIN32_WIN32MINIDUMP_H
#include "core/types.h"
#include "util/string.h"
namespace Win32
{
class Win32MiniDump
{
public:
    static void Setup();
    static bool WriteMiniDump();
private:
    static bool WriteMiniDumpInternal(EXCEPTION_POINTERS* exceptionInfo);
    static Util::String BuildMiniDumpFilename();
    static LONG WINAPI ExceptionCallback(EXCEPTION_POINTERS* exceptionInfo);
};
}
#endif