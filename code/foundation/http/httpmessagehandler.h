#pragma once
#ifndef HTTP_HTTPMESSAGEHANDLER_H
#define HTTP_HTTPMESSAGEHANDLER_H
#include "interface/interfacehandlerbase.h"
#include "messaging/message.h"
#include "io/console.h"
#include "http/httpserver.h"
#include "http/httpprotocol.h"
namespace Http
{
class HttpMessageHandler : public Interface::InterfaceHandlerBase
{
    __DeclareClass(HttpMessageHandler);
public:
    HttpMessageHandler();
    virtual ~HttpMessageHandler();
    virtual void Open();
    virtual void Close();
    virtual bool HandleMessage(const Ptr<Messaging::Message>& msg);
    virtual void DoWork();
    void SetTcpPort(ushort port);
private:
    ushort DefaultTcpPort;
    void OnAttachRequestHandler(const Ptr<AttachRequestHandler>& msg);
    void OnRemoveRequestHandler(const Ptr<RemoveRequestHandler>& msg);
    Ptr<HttpServer> httpServer;
};
inline void
HttpMessageHandler::SetTcpPort(ushort port)
{
    this->DefaultTcpPort = port;
}
}
#endif