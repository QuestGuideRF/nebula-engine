#pragma once
#include "core/refcounted.h"
#include "editor/ui/window.h"
namespace Presentation
{
class Toolbar : public BaseWindow
{
    __DeclareClass(Toolbar)
public:
    Toolbar();
    ~Toolbar();
    void Run(SaveMode save) override;
private:
};
__RegisterClass(Toolbar)
}