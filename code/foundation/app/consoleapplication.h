#pragma once
#include "app/application.h"
#include "core/coreserver.h"
#include "io/ioserver.h"
namespace App
{
class ConsoleApplication : public Application
{
public:
    ConsoleApplication();
    virtual ~ConsoleApplication();
    virtual bool Open();
    virtual void Close();
protected:
    Ptr<Core::CoreServer> coreServer;
    Ptr<IO::IoServer> ioServer;
};
}