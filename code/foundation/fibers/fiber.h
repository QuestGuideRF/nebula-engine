#pragma once
namespace Fibers
{
class Fiber
{
public:
    Fiber();
    Fiber(std::nullptr_t);
    Fiber(void(*Function)(void*), void* context);
    Fiber(const Fiber& rhs);
    ~Fiber();
    void operator=(const Fiber& rhs);
    static void ThreadToFiber(Fiber& fiber);
    static void FiberToThread(Fiber& fiber);
    void SwitchToFiber(Fiber& CurrentFiber);
private:
    void* handle;
    void* context;
};
}