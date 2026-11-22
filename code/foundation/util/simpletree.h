#pragma once
#include "core/types.h"
#include "util/array.h"
namespace Util
{
template<class VALUETYPE> class SimpleTree
{
public:
    class Node
    {
    public:
        Node();
        Node(const Node& parent, const VALUETYPE& val);
        ~Node();
        const Node& operator[](IndexT i) const;
        Node& operator[](IndexT i);
        const Node& Child(IndexT i) const;
        Node& Child(IndexT i);
        bool HasParent() const;
        Node& Parent();
        const Node& Parent() const;
        void Clear();
        SizeT Size() const;
        bool IsEmpty() const;
        Node& Front() const;
        Node& Back() const;
        void Append(const VALUETYPE& val);
        void Erase(IndexT i);
        void Insert(IndexT index, const VALUETYPE& val);
        IndexT Find(const VALUETYPE& val) const;
        VALUETYPE& Value();
        const VALUETYPE& Value() const;
    private:
        Node* parent;
        VALUETYPE value;
        Array<Node*> children;
    };
    SimpleTree();
    Node& Root();
    const Node& Root() const;
private:
    Node rootNode;
};
template<class VALUETYPE>
SimpleTree<VALUETYPE>::Node::Node() :
    parent(0)
{
}
template<class VALUETYPE>
SimpleTree<VALUETYPE>::Node::Node(const Node& p, const VALUETYPE& val) :
    parent(const_cast<Node*>(&p)),
    value(val)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 != this->parent);
    #endif
}
template<class VALUETYPE>
SimpleTree<VALUETYPE>::Node::~Node()
{
    for (IndexT i = 0; i < this->children.Size(); i++)
    {
        delete children[i];
    }
}
template<class VALUETYPE>
const typename SimpleTree<VALUETYPE>::Node&
SimpleTree<VALUETYPE>::Node::operator[](IndexT i) const
{
    return *(this->children[i]);
}
template<class VALUETYPE>
typename SimpleTree<VALUETYPE>::Node&
SimpleTree<VALUETYPE>::Node::operator[](IndexT i)
{
    return *(this->children[i]);
}
template<class VALUETYPE>
const typename SimpleTree<VALUETYPE>::Node&
SimpleTree<VALUETYPE>::Node::Child(IndexT i) const
{
    return *(this->children[i]);
}
template<class VALUETYPE>
typename SimpleTree<VALUETYPE>::Node&
SimpleTree<VALUETYPE>::Node::Child(IndexT i)
{
    return *(this->children[i]);
}
template<class VALUETYPE>
bool
SimpleTree<VALUETYPE>::Node::HasParent() const
{
    return (0 != this->parent);
}
template<class VALUETYPE>
const typename SimpleTree<VALUETYPE>::Node&
SimpleTree<VALUETYPE>::Node::Parent() const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 != this->parent);
    #endif
    return *this->parent;
}
template<class VALUETYPE>
typename SimpleTree<VALUETYPE>::Node&
SimpleTree<VALUETYPE>::Node::Parent()
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 != this->parent);
    #endif
    return *this->parent;
}
template<class VALUETYPE>
void
SimpleTree<VALUETYPE>::Node::Clear()
{
    this->children.Clear();
}
template<class VALUETYPE>
SizeT
SimpleTree<VALUETYPE>::Node::Size() const
{
    return this->children.Size();
}
template<class VALUETYPE>
bool
SimpleTree<VALUETYPE>::Node::IsEmpty() const
{
    return this->children.IsEmpty();
}
template<class VALUETYPE>
typename SimpleTree<VALUETYPE>::Node&
SimpleTree<VALUETYPE>::Node::Front() const
{
    return *(this->children.Front());
}
template<class VALUETYPE>
typename SimpleTree<VALUETYPE>::Node&
SimpleTree<VALUETYPE>::Node::Back() const
{
    return *(this->children.Back());
}
template<class VALUETYPE>
void
SimpleTree<VALUETYPE>::Node::Append(const VALUETYPE& val)
{
    Node* newNode = new Node;
    newNode->parent = this;
    newNode->value = val;
    this->children.Append(newNode);
}
template<class VALUETYPE>
void
SimpleTree<VALUETYPE>::Node::Erase(IndexT i)
{
    delete this->children[i];
    this->children.EraseIndex(i);
}
template<class VALUETYPE>
const VALUETYPE&
SimpleTree<VALUETYPE>::Node::Value() const
{
    return this->value;
}
template<class VALUETYPE>
VALUETYPE&
SimpleTree<VALUETYPE>::Node::Value()
{
    return this->value;
}
template<class VALUETYPE>
IndexT
SimpleTree<VALUETYPE>::Node::Find(const VALUETYPE& val) const
{
    IndexT i;
    SizeT num = this->children.Size();
    for (i = 0; i < num; i++)
    {
        if (val == this->children[i]->Value())
        {
            return i;
        }
    }
    return InvalidIndex;
}
template<class VALUETYPE>
SimpleTree<VALUETYPE>::SimpleTree()
{
}
template<class VALUETYPE>
typename SimpleTree<VALUETYPE>::Node&
SimpleTree<VALUETYPE>::Root()
{
    return this->rootNode;
}
template<class VALUETYPE>
const typename SimpleTree<VALUETYPE>::Node&
SimpleTree<VALUETYPE>::Root() const
{
    return this->rootNode;
}
}