#pragma once
#include "core/types.h"
namespace Util
{
template<class TYPE> class List
{
private:
    class Node;
public:
    class Iterator;
    List();
    List(const List<TYPE>& rhs);
    List(List<TYPE>&& rhs);
    ~List();
    void operator=(const List<TYPE>& rhs);
    void operator=(List<TYPE>&& rhs);
    bool IsEmpty() const;
    SizeT Size() const;
    void Clear();
    void AddList(const List<TYPE>& l);
    Iterator AddAfter(Iterator iter, const TYPE& e);
    Iterator AddBefore(Iterator iter, const TYPE& e);
    Iterator AddFront(const TYPE& e);
    Iterator AddBack(const TYPE& e);
    TYPE RemoveFront();
    TYPE RemoveBack();
    TYPE Remove(Iterator iter);
    TYPE& Front() const;
    TYPE& Back() const;
    Iterator Begin() const;
    Iterator End() const;
    Iterator Last() const;
    Iterator Find(const TYPE& e, Iterator start) const;
    class Iterator
    {
    public:
        Iterator();
        Iterator(Node* node);
        Iterator(const Iterator& rhs);
        const Iterator& operator=(const Iterator& rhs);
        bool operator==(const Iterator& rhs) const;
        bool operator!=(const Iterator& rhs) const;
        const Iterator& operator++();
        Iterator operator++(int);
        const Iterator& operator--();
        Iterator operator--(int);
        operator bool() const;
        TYPE* operator->() const;
        TYPE& operator*() const;
    private:
        friend class List<TYPE>;
        Node* GetNode() const;
        Node* node;
    };
private:
    class Node
    {
        friend class List;
        friend class Iterator;
        Node(const TYPE& v);
        ~Node();
        void SetNext(Node* n);
        Node* GetNext() const;
        void SetPrev(Node* p);
        Node* GetPrev() const;
        TYPE& Value();
        Node* next;
        Node* prev;
        TYPE value;
    };
    Node* front;
    Node* back;
};
template<class TYPE>
List<TYPE>::Node::Node(const TYPE& val) :
    next(0),
    prev(0),
    value(val)
{
}
template<class TYPE>
List<TYPE>::Node::~Node()
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 == this->next);
    n_assert(0 == this->prev);
    #endif
}
template<class TYPE>
void
List<TYPE>::Node::SetNext(Node* n)
{
    this->next = n;
}
template<class TYPE>
typename List<TYPE>::Node*
List<TYPE>::Node::GetNext() const
{
    return this->next;
}
template<class TYPE>
void
List<TYPE>::Node::SetPrev(Node* p)
{
    this->prev = p;
}
template<class TYPE>
typename List<TYPE>::Node*
List<TYPE>::Node::GetPrev() const
{
    return this->prev;
}
template<class TYPE>
TYPE&
List<TYPE>::Node::Value()
{
    return this->value;
}
template<class TYPE>
List<TYPE>::Iterator::Iterator() :
    node(0)
{
}
template<class TYPE>
List<TYPE>::Iterator::Iterator(Node* n) :
    node(n)
{
}
template<class TYPE>
List<TYPE>::Iterator::Iterator(const Iterator& rhs) :
    node(rhs.node)
{
}
template<class TYPE>
const typename List<TYPE>::Iterator&
List<TYPE>::Iterator::operator=(const Iterator& rhs)
{
    if (&rhs != this)
    {
        this->node = rhs.node;
    }
    return *this;
}
template<class TYPE>
bool
List<TYPE>::Iterator::operator==(const Iterator& rhs) const
{
    return (this->node == rhs.node);
}
template<class TYPE>
bool
List<TYPE>::Iterator::operator!=(const Iterator& rhs) const
{
    return (this->node != rhs.node);
}
template<class TYPE>
const typename List<TYPE>::Iterator&
List<TYPE>::Iterator::operator++()
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 != this->node);
    #endif
    this->node = this->node->GetNext();
    return *this;
}
template<class TYPE>
typename List<TYPE>::Iterator
List<TYPE>::Iterator::operator++(int)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 != this->node);
    #endif
    Iterator temp(this->node);
    this->node = this->node->GetNext();
    return temp;
}
template<class TYPE>
const typename List<TYPE>::Iterator&
List<TYPE>::Iterator::operator--()
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 != this->node);
    #endif
    this->node = this->node->GetPrev();
    return *this;
}
template<class TYPE>
typename List<TYPE>::Iterator
List<TYPE>::Iterator::operator--(int)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 != this->node);
    #endif
    Iterator temp(this->node);
    this->node = this->node->GetPrev();
    return temp;
}
template<class TYPE>
List<TYPE>::Iterator::operator bool() const
{
    return (0 != this->node);
}
template<class TYPE>
TYPE*
List<TYPE>::Iterator::operator->() const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->node);
    #endif
    return &(this->node->Value());
}
template<class TYPE>
TYPE&
List<TYPE>::Iterator::operator*() const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->node);
    #endif
    return this->node->Value();
}
template<class TYPE>
typename List<TYPE>::Node*
List<TYPE>::Iterator::GetNode() const
{
    return this->node;
}
template<class TYPE>
List<TYPE>::List() :
    front(0),
    back(0)
{
}
template<class TYPE>
List<TYPE>::List(const List<TYPE>& rhs) :
    front(0),
    back(0)
{
    this->AddList(rhs);
}
template<class TYPE>
List<TYPE>::List(List<TYPE>&& rhs) :
    front(rhs.front),
    back(rhs.back)
{
    rhs.front = nullptr;
    rhs.back = nullptr;
}
template<class TYPE>
List<TYPE>::~List()
{
    this->Clear();
}
template<class TYPE>
void
List<TYPE>::operator=(const List<TYPE>& rhs)
{
    this->Clear();
    this->AddList(rhs);
}
template<class TYPE>
bool
List<TYPE>::IsEmpty() const
{
    return (0 == this->front);
}
template<class TYPE>
SizeT
List<TYPE>::Size() const
{
    Iterator iter;
    SizeT size = 0;
    for (iter = this->Begin(); iter != this->End(); ++iter)
    {
        size++;
    }
    return size;
}
template<class TYPE>
void
List<TYPE>::Clear()
{
    while (this->back)
    {
        this->RemoveBack();
    }
}
template<class TYPE>
void
List<TYPE>::AddList(const List<TYPE>& rhs)
{
    Iterator iter;
    for (iter = rhs.Begin(); iter != rhs.End(); ++iter)
    {
        this->AddBack(*iter);
    }
}
template<class TYPE>
typename List<TYPE>::Iterator
List<TYPE>::AddAfter(Iterator iter, const TYPE& e)
{
    Node* node = new Node(e);
    if (0 == iter.GetNode())
    {
        #if NEBULA_BOUNDSCHECKS
        n_assert((0 == this->front) && (0 == this->back));
        #endif
        this->front = node;
        this->back  = node;
    }
    else
    {
        if (iter.GetNode() == this->back)
        {
            this->back = node;
        }
        if (0 != iter.GetNode()->GetNext())
        {
            iter.GetNode()->GetNext()->SetPrev(node);
        }
        node->SetNext(iter.GetNode()->GetNext());
        iter.GetNode()->SetNext(node);
        node->SetPrev(iter.GetNode());
    }
    return Iterator(node);
}
template<class TYPE>
typename List<TYPE>::Iterator
List<TYPE>::AddBefore(Iterator iter, const TYPE& e)
{
    Node *node = new Node(e);
    if (0 == iter.GetNode())
    {
        #if NEBULA_BOUNDSCHECKS
        n_assert((0 == this->front) && (0 == this->back));
        #endif
        this->front = node;
        this->back = node;
    }
    else
    {
        if (iter.GetNode() == this->front)
        {
            this->front = node;
        }
        if (0 != iter.GetNode()->GetPrev())
        {
            iter.GetNode()->GetPrev()->SetNext(node);
        }
        node->SetPrev(iter.GetNode()->GetPrev());
        iter.GetNode()->SetPrev(node);
        node->SetNext(iter.GetNode());
    }
    return Iterator(node);
}
template<class TYPE>
typename List<TYPE>::Iterator
List<TYPE>::AddFront(const TYPE& e)
{
    return this->AddBefore(this->front, e);
}
template<class TYPE>
typename List<TYPE>::Iterator
List<TYPE>::AddBack(const TYPE& e)
{
    return this->AddAfter(this->back, e);
}
template<class TYPE>
TYPE
List<TYPE>::Remove(Iterator iter)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(iter.GetNode());
    #endif
    Node* node = iter.GetNode();
    if (node->GetPrev())
    {
        node->GetPrev()->SetNext(node->GetNext());
    }
    if (node->GetNext())
    {
        node->GetNext()->SetPrev(node->GetPrev());
    }
    if (node == this->front)
    {
        this->front = node->GetNext();
    }
    if (node == this->back)
    {
        this->back = node->GetPrev();
    }
    node->SetNext(0);
    node->SetPrev(0);
    TYPE elm = node->Value();
    delete node;
    return elm;
}
template<class TYPE>
TYPE
List<TYPE>::RemoveFront()
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 != this->front);
    #endif
    return this->Remove(this->front);
}
template<class TYPE>
TYPE
List<TYPE>::RemoveBack()
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 != this->back);
    #endif
    return this->Remove(this->back);
}
template<class TYPE>
TYPE&
List<TYPE>::Front() const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 != this->front);
    #endif
    return this->front->Value();
}
template<class TYPE>
TYPE&
List<TYPE>::Back() const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 != this->back);
    #endif
    return this->back->Value();
}
template<class TYPE>
typename List<TYPE>::Iterator
List<TYPE>::Begin() const
{
    return Iterator(this->front);
}
template<class TYPE>
typename List<TYPE>::Iterator
List<TYPE>::End() const
{
    return 0;
}
template<class TYPE>
typename List<TYPE>::Iterator
List<TYPE>::Last() const
{
    return Iterator(this->back);
}
template<class TYPE>
typename List<TYPE>::Iterator
List<TYPE>::Find(const TYPE& e, Iterator start) const
{
    for (; start != this->End(); ++start)
    {
        if (*start == e)
        {
            return start;
        }
    }
    return 0;
}
}