#pragma once
#include "core/types.h"
namespace OSX
{
template<typename TYPE>
class OSXThreadLocalPtr
{
public:
    OSXThreadLocalPtr();
    ~OSXThreadLocalPtr();
    void set(TYPE* p);
    TYPE* get() const;
    bool isvalid() const;
private:
    pthread_key_t key;
};
template<typename TYPE>
OSXThreadLocalPtr<TYPE>::OSXThreadLocalPtr() :
    key(0)
{
    int res = pthread_key_create(&this->key, NULL);
    n_assert(0 == res);
    pthread_setspecific(this->key, 0);
}
template<typename TYPE>
OSXThreadLocalPtr<TYPE>::~OSXThreadLocalPtr()
{
    int res = pthread_key_delete(this->key);
    n_assert(0 == res);
    this->key = 0;
}
template<typename TYPE> void
OSXThreadLocalPtr<TYPE>::set(TYPE* p)
{
    pthread_setspecific(this->key, p);
}
template<typename TYPE> TYPE*
OSXThreadLocalPtr<TYPE>::get() const
{
    return (TYPE*) pthread_getspecific(this->key);
}
template<typename TYPE> bool
OSXThreadLocalPtr<TYPE>::isvalid() const
{
    return (0 != pthread_getspecific(this->key));
}
}