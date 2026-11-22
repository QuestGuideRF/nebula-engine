#pragma once
#include "interface/interfacebase.h"
#include "io/console.h"
#include "io/ioserver.h"
#include "io/iointerfaceprotocol.h"
namespace IO
{
class IoInterface : public Interface::InterfaceBase
{
    __DeclareClass(IoInterface);
    __DeclareInterfaceSingleton(IoInterface);
public:
    IoInterface();
    virtual ~IoInterface();
    virtual void Open();
    virtual void Close();
};
}