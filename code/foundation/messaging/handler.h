#pragma once
#include "core/refcounted.h"
#include "messaging/message.h"
namespace Messaging
{
class Handler : public Core::RefCounted
{
    __DeclareClass(Handler);
public:
    Handler();
    virtual ~Handler();
    virtual void Open();
    virtual void Close();
    bool IsOpen() const;
    virtual bool HandleMessage(const Ptr<Message>& msg);
    virtual void DoWork();
protected:
    bool isOpen;
};
inline bool
Handler::IsOpen() const
{
    return this->isOpen;
}
}