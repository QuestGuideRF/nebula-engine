#pragma once
#include "core/refcounted.h"
#include "editor/ui/window.h"
#include "io/uri.h"
namespace nanobind
{
class object;
}
namespace Presentation
{
class ScriptedWindow : public BaseWindow
{
    __DeclareClass(ScriptedWindow)
public:
    ScriptedWindow();
    ~ScriptedWindow();
    void Run(SaveMode save) override;
    bool LoadModule(Util::String const& modulePath);
private:
    nanobind::object* script;
    Util::String modulePath;
};
}