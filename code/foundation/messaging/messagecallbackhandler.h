#pragma once
#include "messaging/message.h"
#include "util/dictionary.h"
#include "util/delegate.h"
#include "core/ptr.h"
namespace Messaging
{
class MessageCallbackHandler
{
public:
    template<class CLASS, void (CLASS::*METHOD)(const Ptr<Messaging::Message>&)> static void AddCallback(const Ptr<Messaging::Message>& msg, CLASS* obj);
    static void AbortCallback(const Ptr<Messaging::Message>& msg);
    template<class CLASS> static void AbortCallbacks(CLASS* obj);
    static void Update();
private:
    static Util::Array<Ptr<Messaging::Message> > Messages;
    static Util::Array<Util::Delegate<void(const Ptr<Messaging::Message>&)> > Callbacks;
};
template<class CLASS>
void
MessageCallbackHandler::AbortCallbacks( CLASS* obj )
{
    n_assert(0 != obj);
    IndexT i;
    for (i = 0; i < Callbacks.Size(); i++)
    {
        const Util::Delegate<void(const Ptr<Messaging::Message>&)>& callback = Callbacks[i];
        if (callback.GetObject<CLASS>() == obj)
        {
            Callbacks.EraseIndex(i);
            Messages.EraseIndex(i);
            i--;
        }
    }
}
template<class CLASS, void (CLASS::*METHOD)(const Ptr<Messaging::Message>&)>
void
MessageCallbackHandler::AddCallback(const Ptr<Messaging::Message>& msg, CLASS* obj)
{
    Util::Delegate<void(const Ptr<Messaging::Message>&)> del = Util::Delegate<void(const Ptr<Messaging::Message>&)>::FromMethod<CLASS,METHOD>(obj);
    Messages.Append(msg);
    Callbacks.Append(del);
}
#define __SingleFireCallback(CLASS, METHOD, OBJ, MSG) Messaging::MessageCallbackHandler::AddCallback<CLASS, &CLASS::METHOD>(MSG, OBJ);
#define __AbortSingleFireCallback(MSG) Messaging::MessageCallbackHandler::AbortCallback(MSG);
#define __AbortSingleFireCallbacks(CLASS, OBJ) Messaging::MessageCallbackHandler::AbortCallbacks<CLASS>(OBJ);
}