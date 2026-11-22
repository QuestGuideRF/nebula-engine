#pragma once
#include "core/refcounted.h"
#include "editor/ui/window.h"
namespace Presentation
{
class History : public BaseWindow
{
    __DeclareClass(History)
public:
    History();
    ~History();
    void Update();
    void Run(SaveMode save) override;
};
__RegisterClass(History)
}