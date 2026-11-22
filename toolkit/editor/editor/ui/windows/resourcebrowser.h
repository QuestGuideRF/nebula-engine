#pragma once
#include "editor/ui/window.h"
namespace Presentation
{
class ResourceBrowser : public BaseWindow
{
    __DeclareClass(ResourceBrowser)
public:
    ResourceBrowser();
    ~ResourceBrowser();
    void Run(SaveMode save) override;
private:
};
__RegisterClass(ResourceBrowser)
}