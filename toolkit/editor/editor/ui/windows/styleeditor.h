#pragma once
#include "core/refcounted.h"
#include "editor/ui/window.h"
namespace Presentation
{
class StyleEditor : public BaseWindow
{
    __DeclareClass(StyleEditor)
public:
    StyleEditor();
    ~StyleEditor();
    void Run(SaveMode save) override;
private:
};
__RegisterClass(StyleEditor)
}