//------------------------------------------------------------------------------
//  interfacebase.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "interface/interfacebase.h"
#include "interface/interfacehandlerbase.h"
#include "core/coreserver.h"

namespace Interface
{
__ImplementClass(Interface::InterfaceBase, 'INBS', Messaging::AsyncPort);

using namespace Core;

//------------------------------------------------------------------------------
/**
*/
InterfaceBase::InterfaceBase() :
    rootDirectory("home:")
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
InterfaceBase::~InterfaceBase()        
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
InterfaceBase::Open()
{
    n_assert(!this->IsOpen());
    
    // установить атрибуты главного сервера
    this->SetCompanyName(CoreServer::Instance()->GetCompanyName());
    this->SetAppName(CoreServer::Instance()->GetAppName());
    this->SetRootDirectory(CoreServer::Instance()->GetRootDirectory());

    Messaging::AsyncPort::Open();
}

//------------------------------------------------------------------------------
/**
*/
void
InterfaceBase::Close()
{
    n_assert(this->IsOpen());

    Messaging::AsyncPort::Close();
}

//------------------------------------------------------------------------------
/**
*/
void
InterfaceBase::AttachHandler(const Ptr<Messaging::Handler>& handler)
{
    // установить атрибуты приложения в обработчике
    const Ptr<InterfaceHandlerBase>& interfaceHandler = handler.downcast<InterfaceHandlerBase>();
    interfaceHandler->SetCompanyName(this->companyName);
    interfaceHandler->SetAppName(this->appName);

    // вызвать родительский класс
    Messaging::AsyncPort::AttachHandler(handler);
}

} // Интерфейс пространства имен
