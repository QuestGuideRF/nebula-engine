#pragma once
#include "interlocked.h"
#include "util/fixedarray.h"
namespace Threading
{
template <class TYPE>
class LockFreeQueue
{
public:
    LockFreeQueue();
    ~LockFreeQueue();
    void Resize(const SizeT size);
    void Enqueue(const TYPE& item);
    bool Dequeue(TYPE& item);
    SizeT Size();
    bool IsEmpty();
private:
    struct Node
    {
        TYPE data;
        Node* next;
        Node()
            : data(TYPE())
            , next(nullptr)
        {};
    };
    Node* head;
    Node* tail;
    volatile SizeT size;
    Node* Alloc();
    void Dealloc(Node* node);
    bool CompareAndSwap(Node** currentValue, Node* expectValue, Node* newValue);
    Node* freeHead;
    Node* freeTail;
    Util::FixedArray<Node> storage;
    SizeT capacity;
};
template<class TYPE>
inline
LockFreeQueue<TYPE>::LockFreeQueue()
    : size(0)
    , capacity(0)
{
}
template<class TYPE>
inline
LockFreeQueue<TYPE>::~LockFreeQueue()
{
}
template<class TYPE>
inline void
LockFreeQueue<TYPE>::Resize(const SizeT size)
{
    this->capacity = size;
    this->storage.Resize(size);
    Node* prev = nullptr;
    for (IndexT i = 0; i < this->storage.Size(); i++)
    {
        if (prev != nullptr)
            prev->next = &this->storage[i];
        prev = &this->storage[i];
    }
    this->freeHead = this->storage.Begin();
    this->freeTail = this->storage.End() - 1;
}
template<class TYPE>
inline void
LockFreeQueue<TYPE>::Enqueue(const TYPE& item)
{
    int size = Interlocked::Increment(&this->size);
    n_assert(size <= this->capacity);
    Node* newNode = this->Alloc();
    newNode->data = item;
    newNode->next = nullptr;
    Node* oldTail = nullptr;
    while (true)
    {
        oldTail = this->tail;
        if (CompareAndSwap(&this->tail, oldTail, newNode))
            break;
    }
    if (oldTail != nullptr)
        oldTail->next = newNode;
    CompareAndSwap(&this->head, nullptr, newNode);
    CompareAndSwap(&this->tail, nullptr, newNode);
}
template<class TYPE>
inline bool
LockFreeQueue<TYPE>::Dequeue(TYPE& item)
{
    Node* oldHead;
    Node* newHead;
    while (true)
    {
        oldHead = this->head;
        if (oldHead == nullptr)
            return false;
        newHead = oldHead->next;
        if (CompareAndSwap(&this->head, oldHead, newHead))
            break;
    }
    oldHead->next = nullptr;
    item = oldHead->data;
    this->Dealloc(oldHead);
    int size = Interlocked::Decrement(&this->size);
    n_assert(size >= 0);
    return true;
}
template<class TYPE>
inline SizeT
LockFreeQueue<TYPE>::Size()
{
    return this->size;
}
template<class TYPE>
inline bool
LockFreeQueue<TYPE>::IsEmpty()
{
    return this->size == 0;
}
template<class TYPE>
__forceinline typename LockFreeQueue<TYPE>::Node*
LockFreeQueue<TYPE>::Alloc()
{
    Node* oldHead = nullptr;
    Node* newHead = nullptr;
    while (true)
    {
        oldHead = this->freeHead;
        if (oldHead == nullptr)
            continue;
        newHead = oldHead->next;
        if (CompareAndSwap(&this->freeHead, oldHead, newHead))
            break;
    }
    n_assert(oldHead != nullptr);
    oldHead->next = nullptr;
    return oldHead;
}
template<class TYPE>
__forceinline void
LockFreeQueue<TYPE>::Dealloc(LockFreeQueue<TYPE>::Node* node)
{
    n_assert(node != nullptr);
    Node* oldTail = nullptr;
    while (true)
    {
        oldTail = this->freeTail;
        if (CompareAndSwap(&this->freeTail, oldTail, node))
            break;
    }
    if (oldTail != nullptr)
        oldTail->next = node;
    CompareAndSwap(&this->freeHead, nullptr, node);
    CompareAndSwap(&this->freeTail, nullptr, node);
}
template<class TYPE>
inline bool
LockFreeQueue<TYPE>::CompareAndSwap(Node** currentValue, Node* expectValue, Node* newValue)
{
    return Interlocked::CompareExchangePointer((void* volatile*)currentValue, (void*)newValue, (void*)expectValue) == expectValue;
}
}