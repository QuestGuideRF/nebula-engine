#ifndef POSIX_SYSFUNC_H
#define POSIX_SYSFUNC_H
#include "core/types.h"
#include "core/exithandler.h"
namespace System
{
    class SystemInfo;
}
namespace Posix
{
class SysFunc
{
public:
    static void Setup();
    static void Exit(int exitCode);
    static void Error(const char* error);
    static void DebugOut(const char* msg);
    static void MessageBox(const char* msg);
    static void Sleep(double sec);
private:
    friend class Core::ExitHandler;
    static const Core::ExitHandler* RegisterExitHandler(const Core::ExitHandler* exitHandler);
    static bool volatile SetupCalled;
    static const Core::ExitHandler* ExitHandlers;
    static System::SystemInfo systemInfo;
};
};
#endif