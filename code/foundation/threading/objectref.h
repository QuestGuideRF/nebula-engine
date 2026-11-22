#pragma once
#include "core/refcounted.h"
namespace Threading
{
class ObjectRef : public Core::RefCounted
{
    __DeclareClass(ObjectRef);
public:
    ObjectRef();
    virtual ~ObjectRef();
    bool IsValid() const;
    template<class TYPE> void Validate(TYPE* obj);
    void Invalidate();
    template<class TYPE> TYPE* Ref() const;
private:
    Core::RefCounted* volatile ref;
};
inline bool
ObjectRef::IsValid() const
{
    return (0 != this->ref);
}
template<class TYPE> inline void
ObjectRef::Validate(TYPE* obj)
{
    n_assert(0 != obj);
    n_assert(obj->IsA(TYPE::RTTI));
    n_assert(0 == this->ref);
    obj->AddRef();
    Interlocked::ExchangePointer((void* volatile*)&this->ref, obj);
}
inline void
ObjectRef::Invalidate()
{
    n_assert(0 != this->ref);
    this->ref->Release();
    Interlocked::ExchangePointer((void* volatile*)&this->ref, 0);
}
template<class TYPE> inline TYPE*
ObjectRef::Ref() const
{
    n_assert(0 != this->ref);
    n_assert(this->ref->IsA(TYPE::RTTI));
    return (TYPE*) this->ref;
}
}