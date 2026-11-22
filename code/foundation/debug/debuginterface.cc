//------------------------------------------------------------------------------
//  debuginterface.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "debug/debuginterface.h"
#include "debug/debughandler.h"
#include "messaging/runthroughhandlerthread.h"

namespace Debug
{
__ImplementClass(Debug::DebugInterface, 'DBIF', Interface::InterfaceBase);
__ImplementInterfaceSingleton(Debug::DebugInterface);

using namespace Interface;
using namespace Messaging;

//------------------------------------------------------------------------------
/**
*/
DebugInterface::DebugInterface()
{
    __ConstructInterfaceSingleton;
}

//------------------------------------------------------------------------------
/**
*/
DebugInterface::~DebugInterface()
{
    __DestructInterfaceSingleton;
}

//------------------------------------------------------------------------------
/**
*/
void
DebugInterface::Open()
{
    // настроить объект потока обработчика сообщений
    Ptr<RunThroughHandlerThread> handlerThread = RunThroughHandlerThread::Create();
    handlerThread->SetName("Поток интерфейса отладки");
    handlerThread->SetThreadAffinity(System::Cpu::Core4);
    handlerThread->AttachHandler(DebugHandler::Create());
    this->SetHandlerThread(handlerThread.cast<HandlerThreadBase>());

    InterfaceBase::Open();
}

} // Отладка пространства имен
