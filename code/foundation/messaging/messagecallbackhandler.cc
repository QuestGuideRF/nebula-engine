//------------------------------------------------------------------------------
//  messagecallbackhandler.cc
//  (C) 2013 Густав Стербрант
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "messagecallbackhandler.h"

namespace Messaging
{
Util::Array<Ptr<Messaging::Message> >  MessageCallbackHandler::Messages;
Util::Array<Util::Delegate<void(const Ptr<Messaging::Message>&)> >  MessageCallbackHandler::Callbacks;

//------------------------------------------------------------------------------
/**
*/
void 
MessageCallbackHandler::Update()
{
    IndexT i;
    for (i = 0; i < Messages.Size(); i++)
    {
        const Util::Delegate<void(const Ptr<Messaging::Message>&)>& del = Callbacks[i];
        const Ptr<Messaging::Message>& msg = Messages[i];

        // if the message is handled, invoke delegate
        if (msg->Handled() || msg->DeferredHandled())
        {
            // установить сообщение для обработки, если оно только что помечено как отложенное
            msg->SetHandled(true);

            // вызвать делегата
            del(msg);

            // удалить обратный вызов и уменьшить индекс
            Callbacks.EraseIndex(i);
            Messages.EraseIndex(i);
            i--;
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
MessageCallbackHandler::AbortCallback( const Ptr<Messaging::Message>& msg )
{
    n_assert(msg.isvalid());
    IndexT i = Messages.FindIndex(msg);
    if (i != InvalidIndex)
    {
        Callbacks.EraseIndex(i);
        Messages.EraseIndex(i);
    }   
}

} // пространство имен