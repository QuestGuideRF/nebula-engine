#pragma once
#include "core/ptr.h"
#include "messaging/port.h"
namespace Messaging
{
class Dispatcher : public Port
{
    __DeclareClass(Dispatcher);
public:
    Dispatcher();
    virtual void HandleMessage(const Ptr<Messaging::Message>& msg);
    void AttachPort(const Ptr<Port>& port);
    void RemovePort(const Ptr<Port>& port);
    bool HasPort(const Ptr<Port>& port) const;
private:
    Util::Array<Ptr<Port> > portArray;
    Util::Array<Util::Array<Ptr<Port> > > idPorts;
    Util::Dictionary<const Id*,IndexT> idPortMap;
};
}