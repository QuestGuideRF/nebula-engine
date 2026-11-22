#pragma once
#include "core/types.h"
namespace Util
{
template<typename T>
class Delegate;
template<typename RETTYPE, typename ... ARGTYPES>
class Delegate<RETTYPE(ARGTYPES...)>
{
public:
    Delegate();
    Delegate(Delegate<RETTYPE(ARGTYPES...)> const& rhs);
    Delegate(Delegate<RETTYPE(ARGTYPES...)>&& rhs);
    template <typename LAMBDA>
    Delegate(LAMBDA const& lambda);
    void operator=(Delegate<RETTYPE(ARGTYPES...)> const& rhs);
    void operator=(Delegate<RETTYPE(ARGTYPES...)>&& rhs);
    bool operator==(std::nullptr_t);
    bool operator!();
    template <typename LAMBDA>
    Delegate<RETTYPE(ARGTYPES ...)>& operator=(LAMBDA const& instance);
    RETTYPE operator()(ARGTYPES ... args) const;
    template<class CLASS, RETTYPE (CLASS::*METHOD)(ARGTYPES ...)> static Delegate<RETTYPE(ARGTYPES ...)> FromMethod(CLASS* objPtr);
    template<RETTYPE(*FUNCTION)(ARGTYPES ...)> static Delegate<RETTYPE(ARGTYPES ...)> FromFunction();
    template <typename LAMBDA> static Delegate<RETTYPE(ARGTYPES ...)> FromLambda(const LAMBDA & instance);
    template<class CLASS> const CLASS* GetObject() const;
    bool IsValid();
private:
    using StubType = RETTYPE(*)(void*, ARGTYPES...);
    template<class CLASS, RETTYPE(CLASS::*METHOD)(ARGTYPES ...)> static RETTYPE MethodStub(void* objPtr, ARGTYPES ... args);
    template<RETTYPE(*FUNCTION)(ARGTYPES ... )> static RETTYPE FunctionStub(void* dummyPtr, ARGTYPES ... args);
    template <typename LAMBDA> static RETTYPE LambdaStub(void* objPtr, ARGTYPES... arg);
    void Assign(void* obj, typename Delegate<RETTYPE(ARGTYPES...)>::StubType stub);
    void* objPtr;
    StubType stubPtr;
};
template<typename RETTYPE, typename ... ARGTYPES>
Delegate<RETTYPE(ARGTYPES ...)>::Delegate() :
    objPtr(nullptr),
    stubPtr(nullptr)
{
}
template<typename RETTYPE, typename ... ARGTYPES>
Delegate<RETTYPE(ARGTYPES ...)>::Delegate(Delegate<RETTYPE(ARGTYPES...)> const& rhs) :
    objPtr(rhs.objPtr),
    stubPtr(rhs.stubPtr)
{
}
template<typename RETTYPE, typename ... ARGTYPES>
Delegate<RETTYPE(ARGTYPES ...)>::Delegate(Delegate<RETTYPE(ARGTYPES...)>&& rhs) :
    objPtr(rhs.objPtr),
    stubPtr(rhs.stubPtr)
{
    rhs.objPtr = nullptr;
    rhs.stubPtr = nullptr;
}
template<typename RETTYPE, typename ... ARGTYPES>
template <typename LAMBDA>
Delegate<RETTYPE(ARGTYPES...)>::Delegate(LAMBDA const& lambda)
{
    static_assert(sizeof(LAMBDA) == 1ULL, "Util::Delegate принимает лямбда-выражения, содержащие переменные захвата! Прочитайте описание вверху util/delegate.h.");
    Assign((void*)(&lambda), LambdaStub<LAMBDA>);
}
template<typename RETTYPE, typename ... ARGTYPES>
void
Delegate<RETTYPE(ARGTYPES...)>::operator=(Delegate<RETTYPE(ARGTYPES...)> const& rhs)
{
    if (this != &rhs)
    {
        this->objPtr = rhs.objPtr;
        this->stubPtr = rhs.stubPtr;
    }
}
template<typename RETTYPE, typename ... ARGTYPES>
void
Delegate<RETTYPE(ARGTYPES...)>::operator=(Delegate<RETTYPE(ARGTYPES...)>&& rhs)
{
    if (this != &rhs)
    {
        this->objPtr = rhs.objPtr;
        this->stubPtr = rhs.stubPtr;
        rhs.objPtr = nullptr;
        rhs.stubPtr = nullptr;
    }
}
template<typename RETTYPE, typename ... ARGTYPES>
template <typename LAMBDA>
Delegate<RETTYPE(ARGTYPES...)>&
Delegate<RETTYPE(ARGTYPES...)>::operator=(LAMBDA const& instance)
{
    static_assert(sizeof(LAMBDA) == 1ULL, "Util::Delegate принимает лямбда-выражения, содержащие переменные захвата! Прочитайте описание вверху util/delegate.h.");
    Assign((void*)(&instance), LambdaStub<LAMBDA>);
    return *this;
}
template<typename RETTYPE, typename ... ARGTYPES>
bool
Delegate<RETTYPE(ARGTYPES...)>::operator==(std::nullptr_t)
{
    return this->stubPtr == nullptr;
}
template<typename RETTYPE, typename ... ARGTYPES>
bool
Delegate<RETTYPE(ARGTYPES...)>::operator!()
{
    return this->stubPtr == nullptr;
}
template<typename RETTYPE, typename ... ARGTYPES> RETTYPE
Delegate<RETTYPE(ARGTYPES...)>::operator()(ARGTYPES ... args) const
{
    return (*this->stubPtr)(this->objPtr, args...);
}
template<typename RETTYPE, typename ... ARGTYPES>
template<class CLASS, RETTYPE (CLASS::*METHOD)(ARGTYPES ...)>
Delegate<RETTYPE(ARGTYPES...)>
Delegate<RETTYPE(ARGTYPES...)>::FromMethod(CLASS* objPtr_)
{
    Delegate<RETTYPE(ARGTYPES...)> del;
    del.objPtr = objPtr_;
    del.stubPtr = &MethodStub<CLASS,METHOD>;
    return del;
}
template<typename RETTYPE, typename ... ARGTYPES>
template<RETTYPE(*FUNCTION)(ARGTYPES ...)>
Delegate<RETTYPE(ARGTYPES...)>
Delegate<RETTYPE(ARGTYPES...)>::FromFunction()
{
    Delegate<RETTYPE(ARGTYPES...)> del;
    del.objPtr = 0;
    del.stubPtr = &FunctionStub<FUNCTION>;
    return del;
}
template<typename RETTYPE, typename ... ARGTYPES>
template<typename LAMBDA>
Delegate<RETTYPE(ARGTYPES...)>
Delegate<RETTYPE(ARGTYPES...)>::FromLambda(const LAMBDA & instance)
{
    static_assert(sizeof(LAMBDA) == 1ULL, "Util::Delegate принимает лямбда-выражения, содержащие переменные захвата! Прочитайте описание вверху util/delegate.h.");
    return ((void*)(&instance), LambdaStub<LAMBDA>);
}
template<typename RETTYPE, typename ... ARGTYPES>
template<class CLASS, RETTYPE (CLASS::*METHOD)(ARGTYPES ...)>
RETTYPE
Delegate<RETTYPE(ARGTYPES...)>::MethodStub(void* objPtr_, ARGTYPES ... arg_)
{
    CLASS* obj = static_cast<CLASS*>(objPtr_);
    return (obj->*METHOD)(arg_...);
}
template<typename RETTYPE, typename ... ARGTYPES>
template<RETTYPE(*FUNCTION)(ARGTYPES ...)>
RETTYPE
Delegate<RETTYPE(ARGTYPES...)>::FunctionStub(void* dummyPtr, ARGTYPES ... arg_)
{
    return (*FUNCTION)(arg_...);
}
template<typename RETTYPE, typename ... ARGTYPES>
template<typename LAMBDA>
RETTYPE
Delegate<RETTYPE(ARGTYPES...)>::LambdaStub(void* objPtr, ARGTYPES ... arg_)
{
    LAMBDA* p = static_cast<LAMBDA*>(objPtr);
    return (p->operator())(arg_...);
}
template<typename RETTYPE, typename ... ARGTYPES>
template<class CLASS>
inline const CLASS*
Delegate<RETTYPE(ARGTYPES...)>::GetObject() const
{
    return (CLASS*)this->objPtr;
}
template<typename RETTYPE, typename ... ARGTYPES>
bool
Util::Delegate<RETTYPE(ARGTYPES...)>::IsValid()
{
    return (0 != this->stubPtr);
}
template<typename RETTYPE, typename ... ARGTYPES>
void
Delegate<RETTYPE(ARGTYPES...)>::Assign(void* obj, typename Delegate<RETTYPE(ARGTYPES...)>::StubType stub)
{
    this->objPtr = obj;
    this->stubPtr = stub;
}
}