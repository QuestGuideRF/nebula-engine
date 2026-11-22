#pragma once
#ifndef HTTP_HTTPINTERFACE_H
#define HTTP_HTTPINTERFACE_H
#include "core/types.h"
#include "interface/interfacebase.h"
#include "core/refcounted.h"
#include "core/singleton.h"
namespace Http
{
class HttpInterface : public Interface::InterfaceBase
{
    __DeclareClass(HttpInterface);
    __DeclareInterfaceSingleton(HttpInterface);
public:
    HttpInterface();
    virtual ~HttpInterface();
    virtual void Open();
    void SetTcpPort(ushort port);
protected:
    ushort tcpPort;
};
inline void
HttpInterface::SetTcpPort(ushort port)
{
    this->tcpPort = port;
}
}
#endif