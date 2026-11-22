#pragma once
#include "core/types.h"
#include "math/scalar.h"
#if NEBULA_DEBUG
#if __WIN32__
#pragma warning( push )
#pragma warning(disable: 4189)
#endif
#define NEBULA_UNUSED_ATTR
#endif
namespace Core
{
    class RefCounted;
}
template<class TYPE>
class Ptr
{
public:
    Ptr();
    Ptr(TYPE* p);
    Ptr(std::nullptr_t rhs);
    Ptr(const Ptr<TYPE>& p);
    Ptr(Ptr<TYPE>&& p);
    template <class OTHERTYPE>
    friend class Ptr;
    template<class OTHERTYPE>
    Ptr(OTHERTYPE* rhs) : ptr(nullptr)
    {
        this->ptr = nullptr;
        static_assert(std::is_base_of<TYPE, OTHERTYPE>::value, "Неявное приведение предполагает, что левая сторона должна быть основанием правой.");
        TYPE* p = reinterpret_cast<TYPE*>(rhs);
        if (p != this->ptr)
        {
            this->ptr = p;
            if (this->ptr)
                this->ptr->AddRef();
        }
    }
    template <class OTHERTYPE>
    Ptr(const Ptr<OTHERTYPE>& rhs) : ptr(nullptr)
    {
        this->ptr = nullptr;
        static_assert(std::is_base_of<TYPE, OTHERTYPE>::value, "Неявное приведение предполагает, что левая сторона должна быть основанием правой.");
        TYPE* p = reinterpret_cast<TYPE*>(rhs.ptr);
        if (p != this->ptr)
        {
            this->ptr = p;
            if (nullptr != this->ptr)
                this->ptr->AddRef();
        }
    }
    template <class OTHERTYPE>
    Ptr(Ptr<OTHERTYPE>&& rhs)
    {
        this->ptr = nullptr;
        static_assert(std::is_base_of<TYPE, OTHERTYPE>::value, "Неявное приведение предполагает, что левая сторона должна быть основанием правой.");
        TYPE* p = reinterpret_cast<TYPE*>(rhs.ptr);
        this->ptr = p;
        rhs.ptr = nullptr;
    }
    ~Ptr();
    void operator=(TYPE* rhs);
    void operator=(const Ptr<TYPE>& rhs);
    void operator=(Ptr<TYPE>&& rhs);
    void operator=(std::nullptr_t rhs);
    template<class OTHERTYPE>
    void operator=(OTHERTYPE* rhs)
    {
        static_assert(std::is_base_of<TYPE, OTHERTYPE>::value, "Неявное присваивание предполагает, что левая часть должна быть основанием правой.");
        TYPE* p = reinterpret_cast<TYPE*>(rhs);
        if (this->ptr != p)
        {
            if (this->ptr != nullptr)
                this->ptr->Release();
            this->ptr = p;
            if (this->ptr != nullptr) this->ptr->AddRef();
        }
    }
    template<class OTHERTYPE>
    void operator=(const Ptr<OTHERTYPE>& rhs)
    {
        static_assert(std::is_base_of<TYPE, OTHERTYPE>::value, "Неявное присваивание предполагает, что левая часть должна быть основанием правой.");
        TYPE* p = reinterpret_cast<TYPE*>(rhs.ptr);
        if (this->ptr != p)
        {
            if (this->ptr != nullptr)
                this->ptr->Release();
            this->ptr = p;
            if (this->ptr != nullptr) this->ptr->AddRef();
        }
    }
    template<class OTHERTYPE>
    void operator=(Ptr<OTHERTYPE>&& rhs)
    {
        static_assert(std::is_base_of<TYPE, OTHERTYPE>::value, "Неявное присваивание предполагает, что левая часть должна быть основанием правой.");
        TYPE* p = reinterpret_cast<TYPE*>(rhs.ptr);
        if (this->ptr != p)
        {
            if (this->ptr != nullptr)
                this->ptr->Release();
            this->ptr = p;
            rhs.ptr = nullptr;
        }
    }
    bool operator==(const Ptr<TYPE>& rhs) const;
    bool operator!=(const Ptr<TYPE>& rhs) const;
    bool operator==(const TYPE* rhs) const;
    bool operator!=(const TYPE* rhs) const;
    TYPE* operator->() const;
    TYPE& operator*() const;
    operator TYPE*() const;
    template<class DERIVED> const Ptr<DERIVED>& downcast() const;
    template<class BASE> const Ptr<BASE>& upcast() const;
    template<class OTHERTYPE> const Ptr<OTHERTYPE>& cast() const;
    bool isvalid() const;
    TYPE* get() const;
    TYPE* get_unsafe() const;
    uint32_t HashCode() const;
private:
    TYPE* ptr;
};
template<class TYPE>
Ptr<TYPE>::Ptr() :
    ptr(0)
{
}
template<class TYPE>
Ptr<TYPE>::Ptr(TYPE* p) :
    ptr(p)
{
    if (0 != this->ptr)
    {
        this->ptr->AddRef();
    }
}
template<class TYPE>
inline Ptr<TYPE>::Ptr(std::nullptr_t rhs) :
    ptr(nullptr)
{
}
template<class TYPE>
Ptr<TYPE>::Ptr(const Ptr<TYPE>& p) :
    ptr(p.ptr)
{
     if (0 != this->ptr)
    {
        this->ptr->AddRef();
    }
}
template<class TYPE>
Ptr<TYPE>::Ptr(Ptr<TYPE>&& p) :
    ptr(p.ptr)
{
    p.ptr = nullptr;
}
template<class TYPE>
Ptr<TYPE>::~Ptr()
{
    if (0 != this->ptr)
    {
        this->ptr->Release();
        this->ptr = 0;
    }
}
template<class TYPE>
void
Ptr<TYPE>::operator=(TYPE* rhs)
{
    if (this->ptr != rhs)
    {
        if (this->ptr != nullptr)
            this->ptr->Release();
        this->ptr = rhs;
        if (this->ptr != nullptr)
            this->ptr->AddRef();
    }
}
template<class TYPE>
void
Ptr<TYPE>::operator=(const Ptr<TYPE>& rhs)
{
    if (this->ptr != rhs.ptr)
    {
        if (this->ptr != nullptr)
            this->ptr->Release();
        this->ptr = rhs.ptr;
        if (this->ptr != nullptr)
            this->ptr->AddRef();
    }
}
template<class TYPE>
void
Ptr<TYPE>::operator=(Ptr<TYPE>&& rhs)
{
    if (this->ptr != rhs.ptr)
    {
        if (this->ptr)
            this->ptr->Release();
        this->ptr = rhs.ptr;
        rhs.ptr = nullptr;
    }
}
template<class TYPE>
inline void
Ptr<TYPE>::operator=(std::nullptr_t rhs)
{
    if (this->ptr != rhs)
    {
        if (this->ptr != nullptr) this->ptr->Release();
        this->ptr = rhs;
    }
}
template<class TYPE>
bool
Ptr<TYPE>::operator==(const Ptr<TYPE>& rhs) const
{
    return (this->ptr == rhs.ptr);
}
template<class TYPE>
bool
Ptr<TYPE>::operator!=(const Ptr<TYPE>& rhs) const
{
    return (this->ptr != rhs.ptr);
}
template<class TYPE>
bool
Ptr<TYPE>::operator==(const TYPE* rhs) const
{
    return (this->ptr == rhs);
}
template<class TYPE>
bool
Ptr<TYPE>::operator!=(const TYPE* rhs) const
{
    return (this->ptr != rhs);
}
template<class TYPE>
TYPE*
Ptr<TYPE>::operator->() const
{
    n_assert2(this->ptr, "Доступ к NULL-указателю в Ptr::operator->()!");
    return this->ptr;
}
template<class TYPE>
TYPE&
Ptr<TYPE>::operator*() const
{
    n_assert2(this->ptr, "Доступ к NULL-указателю в Ptr::operator*()!");
    return *this->ptr;
}
template<class TYPE>
Ptr<TYPE>::operator TYPE*() const
{
    n_assert2(this->ptr, "Доступ к NULL-указателю в Ptr::operator TYPE*()!");
    return this->ptr;
}
template<class TYPE>
template<class DERIVED> const Ptr<DERIVED>&
Ptr<TYPE>::downcast() const
{
#if (NEBULA_DEBUG == 1)
    static_assert(std::is_base_of<TYPE, DERIVED>::value, "Несовместимые типы");
#endif
    return *reinterpret_cast<const Ptr<DERIVED>*>(this);
}
template<class TYPE>
template<class BASE> const Ptr<BASE>&
Ptr<TYPE>::upcast() const
{
#if (NEBULA_DEBUG == 1)
    static_assert(std::is_base_of<BASE, TYPE>::value, "Несовместимые типы");
#endif
    return *reinterpret_cast<const Ptr<BASE>*>(this);
}
template<class TYPE>
template<class OTHERTYPE> const Ptr<OTHERTYPE>&
Ptr<TYPE>::cast() const
{
    return *reinterpret_cast<const Ptr<OTHERTYPE>*>(this);
}
template<class TYPE>
bool
Ptr<TYPE>::isvalid() const
{
    return (nullptr != this->ptr);
}
template<class TYPE>
TYPE*
Ptr<TYPE>::get() const
{
    n_assert2(this->ptr, "NULL pointer access in Ptr::get()!");
    return this->ptr;
}
template<class TYPE>
TYPE*
Ptr<TYPE>::get_unsafe() const
{
    return this->ptr;
}
template<class TYPE>
uint32_t
Ptr<TYPE>::HashCode() const
{
    return Math::pointerhash((void *)this->ptr);
}
#if __WIN32__ && NEBULA_DEBUG
#pragma warning( pop )
#endif
#ifdef NEBULA_UNUSED_ATTR
#undef NEBULA_UNUSED_ATTR
#endif