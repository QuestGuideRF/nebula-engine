#pragma once
#include "core/singleton.h"
#include "input/glfw/glfwinputserver.h"
namespace Input
{
class InputServer : public GLFW::GLFWInputServer
{
    __DeclareClass(InputServer);
    __DeclareInterfaceSingleton(InputServer);
public:
    InputServer();
    virtual ~InputServer();
};
}