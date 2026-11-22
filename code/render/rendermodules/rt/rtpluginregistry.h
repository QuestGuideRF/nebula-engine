#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "rendermodules/rt/rtplugin.h"
namespace RenderModules
{
class RTPluginRegistry : public Core::RefCounted
{
    __DeclareClass(RTPluginRegistry);
    __DeclareSingleton(RTPluginRegistry);
public:
    RTPluginRegistry();
    virtual ~RTPluginRegistry();
    void Setup();
    void Discard();
    bool IsValid() const;
    void RegisterRTPlugin(const Core::Rtti* rtti);
    void UnregisterRTPlugin(const Core::Rtti* rtti);
    const Util::Array<Ptr<RTPlugin> >& GetRTPlugins() const;
    virtual void OnStageCreated(const Ptr<Graphics::Stage>& stage);
    virtual void OnDiscardStage(const Ptr<Graphics::Stage>& stage);
    virtual void OnViewCreated(const Ptr<Graphics::View>& view);
    virtual void OnDiscardView(const Ptr<Graphics::View>& view);
    virtual void OnAttachEntity(const Ptr<Graphics::GraphicsEntity>& entity);
    virtual void OnRemoveEntity(const Ptr<Graphics::GraphicsEntity>& entity);
    virtual void OnUpdateBefore(IndexT frameId, Timing::Time time);
    virtual void OnUpdateAfter(IndexT frameId, Timing::Time time);
    virtual void OnRenderBefore(IndexT frameId, Timing::Time time);
    virtual void OnRenderAfter(IndexT frameId, Timing::Time time);
    virtual void OnRender(const Util::StringAtom& filter);
    virtual void OnFrameBefore(IndexT frameId, Timing::Time time);
    virtual void OnFrameAfter(IndexT frameId, Timing::Time time);
    virtual void OnRenderFrame();
    virtual void OnRenderWithoutView(IndexT frameId, Timing::Time time);
    virtual void OnWindowResized(IndexT windowId, SizeT width, SizeT height);
    IndexT FindPlugin(const Core::Rtti* rtti) const;
    template<typename T> const Ptr<T>& GetPluginByRTTI(const Core::Rtti* rtti) const;
private:
    Util::Array<Ptr<RTPlugin> > plugins;
    bool isValid;
};
inline bool
RTPluginRegistry::IsValid() const
{
    return this->isValid;
}
inline const Util::Array<Ptr<RTPlugin> >&
RTPluginRegistry::GetRTPlugins() const
{
    return this->plugins;
}
template<typename T> const Ptr<T>&
RenderModules::RTPluginRegistry::GetPluginByRTTI(const Core::Rtti* rtti) const
{
    n_assert(rtti->IsDerivedFrom(RTPlugin::RTTI));
    IndexT index = this->FindPlugin(rtti);
    return this->plugins[index].downcast<T>();
}
}