#pragma once
#include "core/refcounted.h"
#include "editor/ui/window.h"
namespace Presentation
{
class AssetEditor;
class AssetBrowser : public BaseWindow
{
    __DeclareClass(AssetBrowser)
public:
    AssetBrowser();
    ~AssetBrowser();
    void Update();
    void Run(SaveMode save) override;
private:
    void DisplayFileTree();
};
__RegisterClass(AssetBrowser)
}