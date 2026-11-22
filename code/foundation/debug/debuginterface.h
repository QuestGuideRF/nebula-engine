#pragma once
#include "interface/interfacebase.h"
#include "core/singleton.h"
namespace Debug
{
class DebugInterface : public Interface::InterfaceBase
{
    __DeclareClass(DebugInterface);
    __DeclareInterfaceSingleton(DebugInterface);
public:
    DebugInterface();
    virtual ~DebugInterface();
    virtual void Open();
};
}