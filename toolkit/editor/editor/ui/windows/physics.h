#pragma once
#include "core/refcounted.h"
#include "editor/ui/window.h"
namespace Presentation
{
class Physics: public BaseWindow
{
    __DeclareClass(Physics)
public:
    Physics();
    ~Physics();
    void Run(SaveMode save) override;
private:
    Graphics::GraphicsEntityId defaultCamera;
};
__RegisterClass(Physics)
}