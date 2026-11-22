//------------------------------------------------------------------------------
//  instanceserver.cc
//  (C) 2012 Густав Стербрант
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "instancing/instanceserver.h"
namespace Instancing
{
#if __VULKAN__
__ImplementClass(Instancing::InstanceServer, 'INSR', Vulkan::VkInstanceServer);
#else
#error "Класс InstanceServer не реализован на этой платформе!"
#endif

__ImplementSingleton(Instancing::InstanceServer);

//------------------------------------------------------------------------------
/**
*/
InstanceServer::InstanceServer()
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
InstanceServer::~InstanceServer()
{
    __DestructSingleton;
}

} // пространство имен
