//------------------------------------------------------------------------------
//  inputserver.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "input/inputserver.h"

namespace Input
{
__ImplementClass(Input::InputServer, 'INPS', GLFW::GLFWInputServer);
__ImplementInterfaceSingleton(Input::InputServer);

//------------------------------------------------------------------------------
/**
*/
InputServer::InputServer()
{
    __ConstructInterfaceSingleton;
}

//------------------------------------------------------------------------------
/**
*/
InputServer::~InputServer()
{
    __DestructInterfaceSingleton;
}

} // пространство имен Ввод
