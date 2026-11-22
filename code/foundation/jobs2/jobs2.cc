//------------------------------------------------------------------------------
//  @file jobs2.cc
//  @copyright (C) 2021 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "profiling/profiling.h"
#include "io/ioserver.h"
#include "jobs2.h"

namespace Jobs2
{

Jobs2Context ctx;

__ImplementClass(Jobs2::JobThread, 'J2TH', Threading::Thread);
//------------------------------------------------------------------------------
/**
*/
JobThread::JobThread()
    : wakeupEvent{ false }
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
JobThread::~JobThread()
{
    if (this->IsRunning())
    {
        this->Stop();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
JobThread::SignalWorkAvailable()
{
    this->wakeupEvent.Signal();
}

//------------------------------------------------------------------------------
/**
*/
void
JobThread::EmitWakeupSignal()
{
    this->wakeupEvent.Signal();
}

//------------------------------------------------------------------------------
/**
*/
void
JobThread::DoWork()
{
    if (this->enableIo)
        IO::IoServer::Create();
    if (this->enableProfiling)
        Profiling::ProfilingRegisterThread();
    while (true)
    {
wait:
        // Подождите, пока появятся вакансии
        this->wakeupEvent.Wait();
        if (this->ThreadStopRequested())
            return;

next:
        ctx.jobLock.Enter();
        auto node = ctx.head;
        auto dragging = ctx.head;
        Jobs2::JobContext* job = nullptr;

        //авто это = ctx.jobs.Begin();
        int64_t jobIndex = -1;
        while (true)
        {
            // Если head имеет значение nullptr, мы либо проиграли гонку за работу.
            // или мы просмотрели весь список, но не нашли вакансий, для которых удовлетворялась бы зависимость,
            // так что давай подождем 
            if (ctx.head == nullptr || node == nullptr)
            {
                ctx.jobLock.Leave();
                goto wait;
            }

            bool dependencyDone = true;

            // Проверьте зависимости
            for (IndexT i = 0; i < node->job.numWaitCounters; i++)
                dependencyDone &= *node->job.waitCounters[i] == 0;

            // If dependency is not done, update iterators and progress
            if (!dependencyDone)
            {
                dragging = node;
                node = node->next;
                continue;
            }

            if (node->sequence != nullptr)
            {
                job = &node->sequence->job;
                n_assert(job->remainingGroups > 0);

                // Если ожидание не достигнуто, перейдите к следующему узлу в основном списке.
                if (job->numWaitCounters == 1 && *job->waitCounters[0] != 0)
                {
                    dragging = node;
                    node = node->next;
                    continue;
                }

                // Если мы потребляем последний пакет в последовательности, просто укажите последовательность на следующий узел.
                jobIndex = --job->remainingGroups;
                if (job->remainingGroups == 0)
                {
                    node->sequence = node->sequence->next;
                }
            }
            else
            {
                job = &node->job;
                n_assert(job->remainingGroups > 0);

                // Если мы потребляем последний пакет задания в этом запуске, отключите узел
                jobIndex = --job->remainingGroups;
                if (job->remainingGroups == 0)
                {
                    // Обновить указатели узлов
                    auto next = node->next;

                    // Если узел не является головным, отсоедините узел между перетаскиванием и текущим.
                    if (node != ctx.head)
                    {
                        // Если узел последний в списке, обновите хвост.
                        if (node == ctx.tail)
                            ctx.tail = dragging;
                        dragging->next = next;
                    }
                    else
                    {
                        // Если голова, то просто отвязываем головной узел
                        ctx.head = next;

                        // Если хвост и узел указывают на один и тот же узел, отсоедините хвост
                        if (node == ctx.tail)
                            ctx.tail = nullptr;
                    }
                }
            }

            // Если мы провалимся, значит, мы нашли работу.
            break;
        }
        ctx.jobLock.Leave();

        // Если jobIndex равен -1, это означает, что мы пытаемся запустить функцию для завершенного задания.
        n_assert(jobIndex != -1);

        // Функция запуска
        if (job->l.callable != nullptr)
            job->l(job->numInvocations, job->groupSize, jobIndex, jobIndex * job->groupSize);
        else
            job->func(job->numInvocations, job->groupSize, jobIndex, jobIndex * job->groupSize, job->data);

        // Уменьшите количество завершенных заданий и, если это было последнее, сообщите о завершенном событии.
        if (Threading::Interlocked::Decrement(&job->groupCompletionCounter) == 0)
        {
            // Если у нас есть счетчик заданий, сигнализируйте о событии только тогда, когда счетчик достигает 0.
            if (job->doneCounter != nullptr)
            {
                long numDispatchesLeft = Threading::Interlocked::Decrement(job->doneCounter);

                if (job->signalEvent != nullptr && numDispatchesLeft == 0)
                    job->signalEvent->Signal();
            }
            else
            {
                // Если у нас нет счетчика, просто сообщите об этом, когда мы закончим отправку.
                if (job->signalEvent != nullptr)
                    job->signalEvent->Signal();
            }

            // Поскольку другие потоки могут ожидать завершения этого задания, запустите пробуждение других потоков.
            for (Ptr<JobThread>& thread : ctx.threads)
            {
                thread->EmitWakeupSignal();
            }
        }

        // Сделайте еще один
        goto next;
    }
}

N_DECLARE_COUNTER(N_JOBS2_MEMORY_COUNTER, Jobs2RingBufferMemory)

//------------------------------------------------------------------------------
/**
*/
void
JobSystemInit(const JobSystemInitInfo& info)
{
    // Настройка потоков системы заданий
    ctx.threads.Resize(info.numThreads);
    for (IndexT i = 0; i < info.numThreads; i++)
    {
        Ptr<JobThread> thread = JobThread::Create();
        thread->enableIo = info.enableIo;
        thread->enableProfiling = info.enableProfiling;
        thread->SetName(Util::String::Sprintf("%s #%d", info.name.Value(), i));
        thread->SetThreadAffinity(info.affinity);
        thread->Start();
        ctx.threads[i] = thread;
    }

    ctx.numBuffers = info.numBuffers;
    ctx.iterator = 0;
    ctx.activeBuffer = 0;
    ctx.scratchMemory.Resize(info.numBuffers);
    ctx.scratchMemorySize = info.scratchMemorySize;
    for (IndexT i = 0; i < info.numBuffers; i++)
    {
        ctx.scratchMemory[i] = (byte*)Memory::Alloc(Memory::ObjectHeap, info.scratchMemorySize);
    }
    N_BUDGET_COUNTER_SETUP(N_JOBS2_MEMORY_COUNTER, info.scratchMemorySize);
    ctx.tail = nullptr;
    ctx.head = nullptr;
}

//------------------------------------------------------------------------------
/**
*/
void
JobSystemUninit()
{
    for (Ptr<JobThread>& thread : ctx.threads)
    {
        thread->Stop();
    }
    ctx.threads.Clear();
}

//------------------------------------------------------------------------------
/**
*/
void
JobNewFrame()
{
    ctx.iterator = 0;
    ctx.activeBuffer = (ctx.activeBuffer + 1) % ctx.numBuffers;
    N_BUDGET_COUNTER_RESET(N_JOBS2_MEMORY_COUNTER);
}

//------------------------------------------------------------------------------
/**
*/
void*
JobAlloc(SizeT bytes)
{
    // обязательно всегда дополняйте выравнивание до следующих 16 байт на случай, если 
    // используемый контекст должен быть согласован
    bytes = Math::align(bytes, 16);
    n_assert((ctx.iterator + bytes) < ctx.scratchMemorySize);
    void* ret = (ctx.scratchMemory[ctx.activeBuffer] + ctx.iterator);
    ctx.iterator += bytes;
    N_BUDGET_COUNTER_INCR(N_JOBS2_MEMORY_COUNTER, bytes);
    return ret;
}

//------------------------------------------------------------------------------
/**
*/
void
JobSequencePlaceholder(SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset, void* ctx)
{
    // Ничего не делайте, это просто хранится в узле последовательности для обеспечения достоверности.
}

Jobs2::JobNode* sequenceNode = nullptr;
Jobs2::JobNode* sequenceTail = nullptr;
const Threading::AtomicCounter* prevDoneCounter = nullptr;
Threading::ThreadId sequenceThread;

//------------------------------------------------------------------------------
/**
*/
void
JobBeginSequence(
    const Util::FixedArray<const Threading::AtomicCounter*, true>& waitCounters
    , Threading::AtomicCounter* doneCounter
    , Threading::Event* signalEvent)
{
    n_assert(sequenceNode == nullptr);
    n_assert(sequenceTail == nullptr);
    sequenceThread = Threading::Thread::GetMyThreadId();

    // Рассчитать размер выделения, который представляет собой узел + счетчики
    SizeT dynamicAllocSize = sizeof(JobNode) + waitCounters.Size() * sizeof(const Threading::AtomicCounter*);
    auto mem = JobAlloc<char>(dynamicAllocSize);
    sequenceNode = (JobNode*)mem;
    sequenceNode->next = nullptr;
    sequenceNode->sequence = nullptr;
    sequenceTail = nullptr;

    // Копирование счетчиков ожидания
    sequenceNode->job.waitCounters = nullptr;
    if (waitCounters.Size() > 0)
    {
        sequenceNode->job.waitCounters = (const Threading::AtomicCounter**)(mem + sizeof(JobNode));
        memcpy(sequenceNode->job.waitCounters, waitCounters.Begin(), waitCounters.Size() * sizeof(const Threading::AtomicCounter*));
    }

    // Узел последовательности настройки с фиктивной функцией
    sequenceNode->job.func = JobSequencePlaceholder;
    sequenceNode->job.remainingGroups = 1;
    sequenceNode->job.numInvocations = 0;
    sequenceNode->job.groupSize = 0;
    sequenceNode->job.data = (void*)(mem + sizeof(JobNode));
    sequenceNode->job.numWaitCounters = (SizeT)waitCounters.Size();
    sequenceNode->job.doneCounter = doneCounter;
    sequenceNode->job.signalEvent = signalEvent;
}

//------------------------------------------------------------------------------
/**
*/
void
JobEndSequence(Threading::Event* signalEvent)
{
    n_assert(sequenceNode != nullptr);
    n_assert(sequenceThread == Threading::Thread::GetMyThreadId());
    if (sequenceNode->sequence != nullptr)
    {
        // Переместить указатель головы на последний элемент в списке
        ctx.jobLock.Enter();

        sequenceTail->job.doneCounter = sequenceNode->job.doneCounter;
        sequenceNode->job.doneCounter = nullptr;
        sequenceTail->job.signalEvent = sequenceNode->job.signalEvent;
        sequenceNode->job.signalEvent = nullptr;

        // Когда последовательность связана, установите указатель заголовка
        if (ctx.head == nullptr)
            ctx.head = sequenceNode;

        // Затем добавьте последовательность в конец текущего списка.
        if (ctx.tail != nullptr)
            ctx.tail->next = sequenceNode;

        // Наконец перенацелить хвост
        ctx.tail = sequenceNode;

        ctx.jobLock.Leave();
    }
    prevDoneCounter = nullptr;
    sequenceNode = nullptr;
    sequenceTail = nullptr;
}

} // пространство имен Jobs2
