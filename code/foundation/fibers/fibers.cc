//------------------------------------------------------------------------------
//  jobs.cc
//  (C) 2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "system/cpu.h"
#include "profiling/profiling.h"
#include "fibers.h"
namespace Fibers
{

thread_local FiberThread* currentThread;

__ImplementClass(Fibers::FiberThread, 'FITH', Threading::Thread);
//------------------------------------------------------------------------------
/**
*/
FiberThread::FiberThread()
    : currentFiber{ nullptr, nullptr }
{
}

//------------------------------------------------------------------------------
/**
*/
FiberThread::~FiberThread()
{

}

//------------------------------------------------------------------------------
/**
*/
void
FiberThread::EmitWakeupSignal()
{
}

//------------------------------------------------------------------------------
/**
*/
void
FiberThread::DoWork()
{
#if NEBULA_ENABLE_PROFILING
    Profiling::ProfilingRegisterThread();
#endif

    // превратить нить в волокно
    Fibers::Fiber::ThreadToFiber(this->threadFiber);

    currentThread = this;
    while (!this->ThreadStopRequested())
    {
        NewFiber();
    }

    // преобразовать обратно в нить и уничтожить волокно
    Fibers::Fiber::FiberToThread(this->threadFiber);
}

//------------------------------------------------------------------------------
/**
*/
bool
FiberThread::HasWork()
{
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
FiberThread::SleepFiber(Threading::AtomicCounter* counter, int value)
{
    FiberQueue::Sleep(FiberWaitContext{ this->currentFiber.fiber, counter, value });

    // вернуться в основной цикл
    this->SwitchFiber();
}

//------------------------------------------------------------------------------
/**
*/
void
FiberThread::NewFiber()
{
    // волокна ожидания пробуждения
    if (!FiberQueue::WakeupFiber(this->currentFiber))
    {
        // if not, try dequeue pending fiber
        if (!FiberQueue::Dequeue(this->currentFiber))
        {
            // if all fails, return
            return;
        }
    }

    // установите текущее волокно, запустите его и по завершении уменьшите счетчик
    this->currentFiber.fiber->SwitchToFiber(this->threadFiber);
}

//------------------------------------------------------------------------------
/**
*/
void
FiberThread::SwitchFiber()
{
    this->threadFiber.SwitchToFiber(*this->currentFiber.fiber);
}

Threading::LockFreeQueue<FiberQueue::Job> FiberQueue::PendingJobsQueue;
Threading::LockFreeQueue<uint> FiberQueue::FiberIdQueue;
Util::FixedArray<Fibers::Fiber> FiberQueue::Fibers;
Util::FixedArray<FiberQueue::Job> FiberQueue::FiberContexts;

Threading::CriticalSection FiberQueue::SleepLock;
Util::Array<FiberWaitContext> FiberQueue::SleepingFibers;
Util::FixedArray<Ptr<FiberThread>> FiberQueue::Threads;

//------------------------------------------------------------------------------
/**
*/
FiberQueue::FiberQueue()
{
}

//------------------------------------------------------------------------------
/**
*/
FiberQueue::~FiberQueue()
{
}

//------------------------------------------------------------------------------
/**
*/
void
FiberQueue::Setup(const FiberQueueCreateInfo& info)
{
    FiberQueue::Threads.Resize(info.numThreads);
    for (uint i = 0; i < info.numThreads; i++)
    {
        FiberQueue::Threads[i] = Fibers::FiberThread::Create();
        FiberQueue::Threads[i]->SetName(Util::String::Sprintf("%s #%d", "Волокнистая нить", i));
        FiberQueue::Threads[i]->SetThreadAffinity(System::Cpu::Core0 << i);
        FiberQueue::Threads[i]->Start();
    }
    FiberQueue::Fibers.Resize(info.numFibers);
    FiberQueue::FiberContexts.Resize(info.numFibers);
    FiberQueue::PendingJobsQueue.Resize(65535);
    FiberQueue::FiberIdQueue.Resize(info.numFibers);
    for (int i = info.numFibers - 1; i >= 0; i--)
        FiberQueue::FiberIdQueue.Enqueue(i);
}

//------------------------------------------------------------------------------
/**
*/
void
FiberQueue::Discard()
{
    for (Ptr<FiberThread>& thread : FiberQueue::Threads)
    {
        thread->Stop();
    }
    FiberQueue::Threads.Clear();
}

//------------------------------------------------------------------------------
/**
*/
void
FiberQueue::Free(uint id)
{
    FiberQueue::FiberIdQueue.Enqueue(id);
}

//------------------------------------------------------------------------------
/**
*/
void
FiberQueue::Sleep(FiberWaitContext context)
{
    Threading::CriticalScope lock(&FiberQueue::SleepLock);
    FiberQueue::SleepingFibers.Append(context);
}

//------------------------------------------------------------------------------
/**
*/
bool
FiberQueue::WakeupFiber(Fibers::FiberContext& fiber)
{
    // пройти через волокна и проверить их состояние ожидания
    Threading::CriticalScope lock(&FiberQueue::SleepLock);
    for (IndexT i = 0; i < FiberQueue::SleepingFibers.Size(); i++)
    {
        const FiberWaitContext& waitFiber = FiberQueue::SleepingFibers[i];

        // волокно имеет условие ожидания выполнено, верните его
        if (*waitFiber.counter == waitFiber.value)
        {
            fiber.fiber = waitFiber.fiber;
            fiber.counter = waitFiber.counter;
            FiberQueue::SleepingFibers.EraseIndex(i);
            return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
FiberFunction(void* param)
{
    // получить информацию о вакансии
    FiberQueue::Job* job = (FiberQueue::Job*)param;

    // запустить функцию задания с контекстом задания
    job->function(job->context);

    // уменьшить счетчик
    Threading::Interlocked::Decrement(job->counter);
    n_assert(*job->counter >= 0);

    // освободить контекст
    FiberQueue::Free(job->id);

    // продолжить работу с потоком
    currentThread->SwitchFiber();
}

//------------------------------------------------------------------------------
/**
*/
bool
FiberQueue::Dequeue(Fibers::FiberContext& fiber)
{
    Job job;
    uint id;

    // if there are free jobs to pick up, do it
    if (FiberQueue::PendingJobsQueue.Dequeue(job))
    {
        if (FiberQueue::FiberIdQueue.Dequeue(id))
        {
            job.id = id;

            // обновить контекст и запустить конструктор на волокне
            FiberQueue::FiberContexts[id] = job;
            new (&FiberQueue::Fibers[id]) Fibers::Fiber{ Fibers::FiberFunction, &FiberQueue::FiberContexts[id] };

            fiber.counter = job.counter;
            fiber.fiber = &FiberQueue::Fibers[id];
            return true;
        }
        else
        {
            // if fiber pool fails, enqueue job again
            FiberQueue::PendingJobsQueue.Enqueue(job);
        }
    }

    return false;
}

} // пространство имен волокна
