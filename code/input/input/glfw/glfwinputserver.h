#pragma once
#include "core/singleton.h"
#include "input/base/inputserverbase.h"
namespace GLFW
{
class GLFWInputServer : public Base::InputServerBase
{
    __DeclareClass(GLFWInputServer);
    __DeclareSingleton(GLFWInputServer);
public:
    GLFWInputServer();
    virtual ~GLFWInputServer();
    void Open();
    void Close();
    void OnFrame();
};
}