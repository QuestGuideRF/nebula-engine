//------------------------------------------------------------------------------
// resourceloaderthread.cc
// (C) 2017-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "io/ioserver.h"
#include "resourceloaderthread.h"
#include "profiling/profiling.h"

namespace Resources
{

__ImplementClass(Resources::ResourceLoaderThread, 'RETH', Threading::Thread);
//------------------------------------------------------------------------------
/**
*/
ResourceLoaderThread::ResourceLoaderThread()
    : completeEvent(true)
    
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ResourceLoaderThread::~ResourceLoaderThread()
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
ResourceLoaderThread::DoWork()
{
    this->ioServer = IO::IoServer::Create();
    Profiling::ProfilingRegisterThread();
    Util::Array<std::function<void()>> arr;
    arr.Reserve(1000);
    while (!this->ThreadStopRequested())
    {
        this->completeEvent.Reset();

        this->jobs.DequeueAll(arr);
        for (IndexT i = 0; i < arr.Size(); i++)
        {
            arr[i]();
        }
        arr.Reset();

        // сигнализировать о том, что эта партия завершена
        this->completeEvent.Signal();

        // ждите новых вакансий!
        this->jobs.Wait();
    }

    this->ioServer = nullptr;
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceLoaderThread::EmitWakeupSignal()
{
    this->jobs.Signal();
}

//------------------------------------------------------------------------------
/**
*/
void
ResourceLoaderThread::Wait()
{
    this->completeEvent.Wait();
}

} // Ресурсы пространства имен
