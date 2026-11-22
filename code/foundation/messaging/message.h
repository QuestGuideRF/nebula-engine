#pragma once
#include "core/refcounted.h"
#include "io/binaryreader.h"
#include "io/binarywriter.h"
#include "messaging/id.h"
#include "threading/interlocked.h"
#define __DeclareMsgId \
public:\
    static Messaging::Id Id; \
    virtual const Messaging::Id& GetId() const;\
private:
#define __ImplementMsgId(type) \
    Messaging::Id type::Id; \
    const Messaging::Id& type::GetId() const { return type::Id; }
namespace Messaging
{
class MessageReader;
class MessageWriter;
class Port;
class Message : public Core::RefCounted
{
    __DeclareClass(Message);
    __DeclareMsgId;
public:
    Message();
    bool CheckId(const Messaging::Id& id) const;
    virtual void Encode(const Ptr<IO::BinaryWriter>& writer);
    virtual void Decode(const Ptr<IO::BinaryReader>& reader);
    void SetHandled(bool b);
    bool Handled() const;
    void SetDeferred(bool b);
    bool IsDeferred() const;
    void SetDeferredHandled(bool b);
    bool DeferredHandled() const;
    bool GetDistribute() const;
    void SetDistribute(bool b);
protected:
    volatile int handled;
    bool deferred;
    bool deferredHandled;
    bool distribute;
};
inline bool
Message::CheckId(const Messaging::Id& id) const
{
    return (id == this->GetId());
}
inline void
Message::SetHandled(bool b)
{
    Threading::Interlocked::Exchange(&this->handled, (int)b);
}
inline bool
Message::Handled() const
{
    return 0 != this->handled;
}
inline void
Message::SetDeferred(bool b)
{
    this->deferred = b;
}
inline bool
Message::IsDeferred() const
{
    return this->deferred;
}
inline void
Message::SetDeferredHandled(bool b)
{
    this->deferredHandled = b;
}
inline bool
Message::DeferredHandled() const
{
    return this->deferredHandled;
}
inline bool
Message::GetDistribute() const
{
    return distribute;
}
inline void
Message::SetDistribute(bool b)
{
    this->distribute = b;
}
}