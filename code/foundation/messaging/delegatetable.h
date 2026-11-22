#pragma once
#include "util/delegate.h"
#include "messaging/id.h"
#include "messaging/message.h"
namespace Messaging
{
class DelegateTable
{
public:
    template<class CLASS, void (CLASS::*METHOD)(const Ptr<Message>&)> void Bind(const Id& msgId, CLASS* obj);
    bool Invoke(const Ptr<Message>& msg);
private:
    void AddDelegate(const Id& msgId, const Util::Delegate<void(const Ptr<Message>&)>& del);
    Util::Array<Util::Array<Util::Delegate<void(const Ptr<Message>&)> > > delegateArray;
    Util::Dictionary<const Id*, IndexT> idIndexMap;
};
template<class CLASS, void (CLASS::*METHOD)(const Ptr<Message>&)>
void
DelegateTable::Bind(const Id& msgId, CLASS* obj)
{
    Util::Delegate<void(const Ptr<Message>&)> del = Util::Delegate<void(const Ptr<Message>&)>::FromMethod<CLASS,METHOD>(obj);
    this->AddDelegate(msgId, del);
}
}