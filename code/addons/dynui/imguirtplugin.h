#pragma once
#include "rendermodules/rt/rtplugin.h"
#include "imguirenderer.h"
namespace Dynui
{
class ImguiRTPlugin : public RenderModules::RTPlugin
{
    __DeclareClass(ImguiRTPlugin);
public:
    ImguiRTPlugin();
    virtual ~ImguiRTPlugin();
    virtual void OnRegister();
    virtual void OnUnregister();
    virtual void OnRenderFrameBatch(const Ptr<Frame::FrameBatch>& frameBatch);
    virtual void OnWindowResized(IndexT windowId, SizeT width, SizeT height);
private:
    Ptr<ImguiRenderer> renderer;
};
}