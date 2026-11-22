//------------------------------------------------------------------------------
//  iointerface.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/iointerface.h"
#include "io/iointerfacehandler.h"
#include "messaging/blockinghandlerthread.h"

namespace IO
{
__ImplementClass(IO::IoInterface, 'IIOF', Interface::InterfaceBase);
__ImplementInterfaceSingleton(IO::IoInterface);

using namespace Messaging;
using namespace Interface;

//------------------------------------------------------------------------------
/**
*/
IoInterface::IoInterface()
{
    __ConstructInterfaceSingleton;
}

//------------------------------------------------------------------------------
/**
*/
IoInterface::~IoInterface()
{
    __DestructInterfaceSingleton;
}

//------------------------------------------------------------------------------
/**
*/
void
IoInterface::Open()
{
    // настроить объект потока обработчика сообщений
    Ptr<BlockingHandlerThread> handlerThread = BlockingHandlerThread::Create();
    handlerThread->SetName("Поток IoInterface");
    handlerThread->SetThreadAffinity(System::Cpu::Core3);
    handlerThread->AttachHandler(IoInterfaceHandler::Create());
    this->SetHandlerThread(handlerThread.cast<HandlerThreadBase>());

    InterfaceBase::Open();
}

//------------------------------------------------------------------------------
/**
*/
void
IoInterface::Close()
{
    InterfaceBase::Close();
}

} // пространство имен ввода-вывода
