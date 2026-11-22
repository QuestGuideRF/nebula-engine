#pragma once
#include "core/types.h"
#include "util/commandlineargs.h"
#if __WIN32__
#define ImplementNebulaApplication() \
void NebulaMain(const Util::CommandLineArgs& args); \
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int nShowCmd) \
{ \
    Util::CommandLineArgs args(lpCmdLine); \
    NebulaMain(args); \
    return 0; \
}
#else
#define ImplementNebulaApplication() \
void NebulaMain(const Util::CommandLineArgs& args); \
int __cdecl main(int argc, const char** argv) \
{ \
    Util::CommandLineArgs args(argc, argv); \
    NebulaMain(args); \
    return 0; \
}
#endif