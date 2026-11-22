#pragma once
#include "core/refcounted.h"
#include "editor/ui/window.h"
namespace Presentation
{
class Outline : public BaseWindow
{
    __DeclareClass(Outline)
public:
    Outline();
    ~Outline();
    void Run(SaveMode save) override;
private:
};
__RegisterClass(Outline)
}