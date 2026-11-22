//------------------------------------------------------------------------------
//  gamecontentserver.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/gamecontentserver.h"

namespace IO
{
#if (__WIN32__ || __OSX__ || __linux__)
__ImplementClass(IO::GameContentServer, 'IGCS', Base::GameContentServerBase);
#else
#error "IO::GameContentServer не реализован на этой платформе!"
#endif
__ImplementInterfaceSingleton(IO::GameContentServer);

//------------------------------------------------------------------------------
/**
*/
GameContentServer::GameContentServer()
{
    __ConstructInterfaceSingleton;
}

//------------------------------------------------------------------------------
/**
*/
GameContentServer::~GameContentServer()
{
    __DestructInterfaceSingleton;
}

} // пространство имен ввода-вывода
