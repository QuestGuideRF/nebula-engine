#pragma once
#include "ids/id.h"
#include "ids/idgenerationpool.h"
#include "util/delegate.h"
#include "util/hashtable.h"
#include "util/arrayallocator.h"
#include "util/string.h"
#include "util/stringatom.h"
#include "util/fourcc.h"
#define __DeclareMsg(NAME, FOURCC, ...) \
class NAME : public Game::Message<NAME, __VA_ARGS__> \
{ \
    public: \
        NAME() = delete; \
        ~NAME() = delete; \
        \
        constexpr static const char* GetName() \
        { \
            return #NAME; \
        }; \
        \
        constexpr static const uint GetFourCC() \
        { \
            return FOURCC; \
        }; \
};
#define __RegisterMsg(MSGTYPE, FUNCTION) \
    MSGTYPE::Register(MSGTYPE::Delegate::FromFunction<FUNCTION>())
#define __this_RegisterMsg(MSGTYPE, METHOD) \
    MSGTYPE::Register(MSGTYPE::Delegate::FromMethod<std::remove_pointer<decltype(this)>::type, &std::remove_pointer<decltype(this)>::type::METHOD>(this))
template<class T>
using UnqualifiedType = typename std::remove_const<typename std::remove_reference<T>::type>::type;
namespace Game
{
ID_32_TYPE(MessageListenerId)
struct MessageListener
{
    Util::FourCC messageId;
    MessageListenerId listenerId;
};
template <class MSG, class ... TYPES>
class Message
{
public:
    ID_32_TYPE(MessageQueueId)
    Message();
    ~Message();
    Message<MSG, TYPES...>(const Message<MSG, TYPES...>&) = delete;
    void operator=(const Message<MSG, TYPES...>&) = delete;
    using Delegate = Util::Delegate<void(TYPES...)>;
    using MessageQueue = typename Util::ArrayAllocator<Util::Tuple<UnqualifiedType<TYPES> ...>>;
    static MessageListener Register(Delegate&& callback);
    static void Deregister(MessageListener listener);
    static void Send(TYPES ... values);
    static MessageQueueId AllocateMessageQueue();
    static void Defer(MessageQueueId qid, TYPES ... values);
    static void DispatchMessageQueue(MessageQueueId id);
    static void DeAllocateMessageQueue(MessageQueueId id);
    static bool IsMessageQueueValid(MessageQueueId id);
    static void Distribute(TYPES ...);
    static void DeregisterAll();
    static bool IsValid(MessageListenerId listener);
    static MessageQueue& GetMessageQueue(MessageQueueId id);
protected:
    friend MSG;
    static Message<MSG, TYPES...>* Instance()
    {
        static Message<MSG, TYPES...> instance;
        return &instance;
    }
    Util::HashTable<MessageListenerId, IndexT> listenerMap;
    Util::ArrayAllocator<
        MessageListenerId,
        Delegate
    > callbacks;
    Ids::IdGenerationPool messageQueueIdPool;
    Util::Array<MessageQueue> messageQueues;
    MessageQueue distributedMessages;
protected:
    Util::StringAtom name;
    Util::FourCC fourcc;
    Ids::IdGenerationPool listenerPool;
private:
    template<std::size_t...Is>
    void send_expander(MessageQueue& data, const IndexT cid, const SizeT index, std::index_sequence<Is...>)
    {
        this->callbacks.Get<1>(cid)(std::get<Is>(data.Get<0>(index))...);
    }
    void send_expander(MessageQueue& data, const IndexT cid, const SizeT index)
    {
        this->send_expander(data, cid, index, std::make_index_sequence<sizeof...(TYPES)>());
    }
};
template <typename MSG, class ... TYPES>
inline
Message<MSG, TYPES...>::Message()
{
    this->name = MSG::GetName();
    this->fourcc = MSG::GetFourCC();
}
template <typename MSG, class ... TYPES>
inline
Message<MSG, TYPES...>::~Message()
{
}
template <typename MSG, class ... TYPES>
inline MessageListener
Message<MSG, TYPES...>::Register(Delegate&& callback)
{
    MessageListenerId l;
    Instance()->listenerPool.Allocate(l.id);
    IndexT index = Instance()->callbacks.Alloc();
    Instance()->callbacks.Set(index, l, callback);
    Instance()->listenerMap.Add(l, index);
    MessageListener listener = { Instance()->fourcc, l };
    return listener;
}
template <typename MSG, class ... TYPES>
inline void
Message<MSG, TYPES...>::Deregister(MessageListener listener)
{
    auto instance = Instance();
    n_assert(listener.messageId == instance->fourcc);
    IndexT index = instance->listenerMap[listener.listenerId];
    if (index != InvalidIndex)
    {
        instance->listenerMap.Erase(listener.listenerId);
        instance->callbacks.EraseIndexSwap(index);
        if (instance->callbacks.Size() != 0)
        {
            instance->listenerMap[instance->callbacks.Get<0>(index)] = index;
        }
    }
}
template <typename MSG, class ... TYPES>
inline void
Message<MSG, TYPES...>::Send(TYPES ... values)
{
    auto instance = Instance();
    SizeT size = instance->callbacks.Size();
    for (SizeT i = 0; i < size; ++i)
    {
        instance->callbacks.Get<1>(i)(values...);
    }
}
template<typename MSG, class ...TYPES>
inline typename Message<MSG, TYPES...>::MessageQueueId
Message<MSG, TYPES...>::AllocateMessageQueue()
{
    auto instance = Instance();
    MessageQueueId id;
    instance->messageQueueIdPool.Allocate(id.id);
    if (Ids::Index(id.id) >= instance->messageQueues.Size())
    {
        n_assert(instance->messageQueues.Size() == Ids::Index(id.id));
        instance->messageQueues.Append({});
    }
    return id;
}
template<typename MSG, class ...TYPES>
inline void
Message<MSG, TYPES...>::Defer(MessageQueueId qid, TYPES ...values)
{
    auto instance = Instance();
    SizeT index = Ids::Index(qid.id);
    auto i = instance->messageQueues[index].Alloc();
    instance->messageQueues[index].Set(i, std::make_tuple(values...));
}
template<typename MSG, class ... TYPES>
inline void
Message<MSG, TYPES...>::DispatchMessageQueue(MessageQueueId id)
{
    auto instance = Instance();
    n_assert(instance->messageQueues.Size() > Ids::Index(id.id));
    n_assert(instance->messageQueueIdPool.IsValid(id.id));
    auto& data = instance->messageQueues[Ids::Index(id.id)];
    SizeT size = data.Size();
    SizeT cidSize = instance->callbacks.Size();
    for (SizeT cid = 0; cid < cidSize; ++cid)
    {
        for (SizeT i = 0; i < size; i++)
        {
            instance->send_expander(data, cid, i);
        }
    }
    data.Clear();
}
template<typename MSG, class ...TYPES> inline void
Message<MSG, TYPES...>::DeAllocateMessageQueue(MessageQueueId id)
{
    auto instance = Instance();
    n_assert(instance->messageQueues.Size() > Ids::Index(id.id));
    n_assert(instance->messageQueueIdPool.IsValid(id.id));
    instance->messageQueues[Ids::Index(id.id)].Clear();
    instance->messageQueueIdPool.Deallocate(id.id);
}
template<typename MSG, class ...TYPES> inline bool
Message<MSG, TYPES...>::IsMessageQueueValid(MessageQueueId id)
{
    return Instance()->messageQueueIdPool.IsValid(id.id);
}
template <typename MSG, class ... TYPES>
inline void
Message<MSG, TYPES...>::Distribute(TYPES ...)
{
    n_error("Распространяемые по сети сообщения пока не поддерживаются!");
}
template <typename MSG, class ... TYPES>
inline void
Message<MSG, TYPES...>::DeregisterAll()
{
    auto instance = Instance();
    SizeT size = instance->callbacks.Size();
    for (SizeT i = 0; i < size; i++)
    {
        instance->listenerPool.Deallocate((instance->callbacks.template Get<0>(i)).id);
    }
    instance->callbacks.Clear();
    instance->distributedMessages.Clear();
    instance->listenerMap.Clear();
}
template <typename MSG, class ... TYPES>
inline bool
Message<MSG, TYPES...>::IsValid(MessageListenerId listener)
{
    return Instance()->listenerPool.IsValid(listener.id);
}
template <typename MSG, class ... TYPES>
inline typename Message<MSG, TYPES...>::MessageQueue&
Message<MSG, TYPES...>::GetMessageQueue(MessageQueueId id)
{
    return Instance()->messageQueues[id.id];
}
}