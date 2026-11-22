#pragma once
#include "core/refcounted.h"
#include "editor/ui/window.h"
#include "editor/editor.h"
namespace Presentation
{
class Settings : public BaseWindow
{
    __DeclareClass(Settings)
public:
    Settings();
    ~Settings();
    void Update();
    void Run(SaveMode save) override;
private:
};
__RegisterClass(Settings)
}