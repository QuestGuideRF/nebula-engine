//------------------------------------------------------------------------------
//  core/coreserver.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "core/coreserver.h"
#include "threading/thread.h"

namespace Core
{
__ImplementClass(Core::CoreServer, 'CORS', Core::RefCounted);
__ImplementSingleton(Core::CoreServer);

using namespace IO;

//------------------------------------------------------------------------------
/**
*/
CoreServer::CoreServer() :
    companyName("gscept"),
    appName("Nebula"),
    rootDirectory("home:"),
    toolDirectory("root:"),
    isOpen(false)
{
    Threading::MainThreadId = Threading::Thread::GetMyThreadId();
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/***/
CoreServer::~CoreServer()
{
    if (this->IsOpen())
    {
        this->Close();
    }
    __DestructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
void
CoreServer::Open()
{
    n_assert(!this->IsOpen());
    this->isOpen = true;

    // создать консольный объект
    this->con = Console::Create();
    this->con->Open();
}

//------------------------------------------------------------------------------
/**
*/
void
CoreServer::Close()
{
    n_assert(this->IsOpen());

    // консоль выключения
    this->con = nullptr;

    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
*/
void 
CoreServer::Trigger()
{
    this->con->Update();
}
} // пространство имен Core
