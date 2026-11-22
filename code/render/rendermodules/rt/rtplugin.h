#pragma once
#include "core/refcounted.h"
#include "timing/time.h"
#include "util/stringatom.h"
namespace Graphics
{
class Stage;
class View;
class GraphicsEntity;
};
namespace RenderModules
{
class RTPlugin : public Core::RefCounted
{
    __DeclareClass(RTPlugin);
public:
    RTPlugin();
    virtual ~RTPlugin();
    virtual void OnRegister();
    virtual void OnUnregister();
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
};
}