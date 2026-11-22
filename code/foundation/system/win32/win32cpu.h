#pragma once
#include "core/types.h"
namespace Win32
{
class Win32Cpu
{
public:
    typedef DWORD CoreId;
    static const CoreId InvalidCoreId       = 0xffffffff;
    static const CoreId MainThreadCore      = 0;
    static const CoreId IoThreadCore        = 2;
    static const CoreId RenderThreadCore    = 1;
    static const CoreId AudioThreadCore     = 3;
    static const CoreId MiscThreadCore      = 4;
    static const CoreId NetworkThreadCore   = 5;
    static const CoreId JobThreadFirstCore  = 6;
    static const CoreId RenderThreadFirstCore = 16;
};
}