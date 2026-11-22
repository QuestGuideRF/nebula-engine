#pragma once
#include "core/refcounted.h"
#include "imguiinputhandler.h"
namespace Dynui
{
class ImguiAddon : public Core::RefCounted
{
    __DeclareClass(ImguiAddon);
public:
    ImguiAddon();
    virtual ~ImguiAddon();
    void Setup();
    void Discard();
private:
    Ptr<ImguiInputHandler> inputHandler;
};
}