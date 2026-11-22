#pragma once
#include "threading/thread.h"
#include "threading/safequeue.h"
#include <functional>
#include "resourceid.h"
namespace IO
{
class IoServer;
}
namespace Resources
{
class ResourceLoaderThread : public Threading::Thread
{
    __DeclareClass(ResourceLoaderThread);
public:
    ResourceLoaderThread();
    virtual ~ResourceLoaderThread();
    void Wait();
private:
    friend class ResourceLoader;
    void DoWork() override;
    virtual void EmitWakeupSignal() override;
    Threading::SafeQueue<std::function<void()>> jobs;
    Threading::Event completeEvent;
    Ptr<IO::IoServer> ioServer;
};
}