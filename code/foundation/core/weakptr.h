#pragma once
#include "core/ptr.h"
template<class TYPE>
class WeakPtr
{
public:
    WeakPtr();
    WeakPtr(TYPE* p);
    WeakPtr(const Ptr<TYPE>& p);
    WeakPtr(const WeakPtr<TYPE>& p);
    ~WeakPtr();
    void operator=(const Ptr<TYPE>& rhs);
    void operator=(const WeakPtr<TYPE>& rhs);
    void operator=(TYPE* rhs);
    TYPE* operator->() const;
    TYPE& operator*() const;
    operator TYPE*() const;
    bool isvalid() const;
    TYPE* get() const;
    TYPE* get_unsafe() const;
private:
    TYPE* ptr;
};
template<class TYPE>
WeakPtr<TYPE>::WeakPtr() :
    ptr(0)
{
}
template<class TYPE>
WeakPtr<TYPE>::WeakPtr(TYPE* p) :
    ptr(p)
{
}
template<class TYPE>
WeakPtr<TYPE>::WeakPtr(const Ptr<TYPE>& p) :
    ptr(p.get_unsafe())
{
}
template<class TYPE>
WeakPtr<TYPE>::WeakPtr(const WeakPtr<TYPE>& p) :
    ptr(p.ptr)
{
}
template<class TYPE>
WeakPtr<TYPE>::~WeakPtr()
{
    this->ptr = 0;
}
template<class TYPE>
void
WeakPtr<TYPE>::operator=(const Ptr<TYPE>& rhs)
{
    this->ptr = rhs.get_unsafe();
}
template<class TYPE>
void
WeakPtr<TYPE>::operator=(const WeakPtr<TYPE>& rhs)
{
    this->ptr = rhs.ptr;
}
template<class TYPE>
void
WeakPtr<TYPE>::operator=(TYPE* rhs)
{
    this->ptr = rhs;
}
template<class TYPE>
TYPE*
WeakPtr<TYPE>::operator->() const
{
    n_assert2(this->ptr, "Доступ к NULL-указателю в WeakPtr::operator->()!");
    return this->ptr;
}
template<class TYPE>
TYPE&
WeakPtr<TYPE>::operator*() const
{
    n_assert2(this->ptr, "Доступ к NULL-указателю в WeakPtr::operator*()!");
    return *this->ptr;
}
template<class TYPE>
WeakPtr<TYPE>::operator TYPE*() const
{
    n_assert2(this->ptr, "Доступ к NULL-указателю в WeakPtr::operator TYPE*()!");
    return this->ptr;
}
template<class TYPE>
bool
WeakPtr<TYPE>::isvalid() const
{
    return (0 != this->ptr);
}
template<class TYPE>
TYPE*
WeakPtr<TYPE>::get() const
{
    n_assert2(this->ptr, "NULL pointer access in WeakPtr::get()!");
    return this->ptr;
}
template<class TYPE>
TYPE*
WeakPtr<TYPE>::get_unsafe() const
{
    return this->ptr;
}