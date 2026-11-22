#pragma once
#include "core/refcounted.h"
#include "framesync/framesynctimer.h"
#include "util/stringatom.h"
#include "ids/idgenerationpool.h"
#include "stage.h"
#include "graphicsentity.h"
#include "coregraphics/graphicsdevice.h"
#include "coregraphics/displaydevice.h"
#include "coregraphics/shaderserver.h"
#include "coregraphics/shaperenderer.h"
#include "coregraphics/textrenderer.h"
#include "debug/debughandler.h"
namespace Graphics
{
struct FrameContext
{
    Timing::Time time;
    Timing::Tick ticks;
    Timing::Time frameTime;
    IndexT frameIndex;
    IndexT bufferIndex;
};
using ViewIndependentCall = void(*)(const Graphics::FrameContext& ctx);
using ViewDependentCall = void(*)(const Ptr<Graphics::View>& view, const Graphics::FrameContext& ctx);
class GraphicsContext;
struct GraphicsContextFunctionBundle;
struct GraphicsContextState;
class View;
class GraphicsServer : public Core::RefCounted
{
    __DeclareClass(GraphicsServer);
    __DeclareSingleton(GraphicsServer);
public:
    GraphicsServer();
    virtual ~GraphicsServer();
    void Open();
    void Close();
    GraphicsEntityId CreateGraphicsEntity();
    void DiscardGraphicsEntity(const GraphicsEntityId id);
    bool IsValidGraphicsEntity(const GraphicsEntityId id);
    Ptr<View> CreateView(const Util::StringAtom& name, void(*)(const Math::rectangle<int>&, IndexT, IndexT), const Math::rectangle<int>& viewport);
    Ptr<View> CreateView(const Util::StringAtom& name);
    const Ptr<View>& GetView(const Util::StringAtom& name);
    void DiscardView(const Ptr<View>& view);
    const Ptr<View>& GetCurrentView() const;
    void SetCurrentView(const Ptr<View>& view);
    using PreViewCallback = void(*)(IndexT, IndexT);
    using PostViewCallback = void(*)(IndexT, IndexT);
    void AddPreViewCall(PreViewCallback callback);
    void AddPostViewCall(PostViewCallback callback);
    void SetResizeCall(void(*)(const SizeT, const SizeT));
    struct SwapInfo
    {
        void (*syncFunc)(CoreGraphics::CmdBufferId) = nullptr;
        CoreGraphics::TextureId swapSource = CoreGraphics::InvalidTextureId;
        CoreGraphics::SubmissionWaitEvent submission;
    };
    void SetSwapInfo(const SwapInfo& info);
    Ptr<Stage> CreateStage(const Util::StringAtom& name, bool main);
    void DiscardStage(const Ptr<Stage>& stage);
    void SetupPreLogicCalls(const Util::Array<ViewIndependentCall>& calls);
    void SetupPostLogicCalls(const Util::Array<ViewIndependentCall>& calls);
    void SetupPreLogicViewCalls(const Util::Array<ViewDependentCall>& calls);
    void SetupPostLogicViewCalls(const Util::Array<ViewDependentCall>& calls);
    void RunPreLogic();
    void RunPostLogic();
    void Render();
    void EndFrame();
    void NewFrame();
    const Timing::Time GetTime() const;
    const Timing::Time GetFrameTime() const;
    const IndexT GetFrameIndex() const;
    void RenderDebug(uint32_t flags);
    void RegisterGraphicsContext(GraphicsContextFunctionBundle* context, GraphicsContextState* state);
    void UnregisterGraphicsContext(GraphicsContextFunctionBundle* context);
    void OnWindowResized(CoreGraphics::WindowId wndId);
private:
    Ids::IdGenerationPool entityPool;
    Ptr<FrameSync::FrameSyncTimer> timer;
    bool ownsTimer;
    FrameContext frameContext;
    Util::Array<GraphicsContextFunctionBundle*> contexts;
    Util::Array<GraphicsContextState*> states;
    Util::Array<Ptr<Stage>> stages;
    Util::Dictionary<Util::StringAtom, Ptr<View>> viewsByName;
    Util::Array<Ptr<View>> views;
    Ptr<View> currentView;
    Util::Array<PreViewCallback> preViewCallbacks;
    Util::Array<PostViewCallback> postViewCallbacks;
    void (*resizeCall) (const SizeT, const SizeT);
    Ptr<CoreGraphics::DisplayDevice> displayDevice;
    bool graphicsDevice;
    Ptr<CoreGraphics::ShaderServer> shaderServer;
    Ptr<CoreGraphics::ShapeRenderer> shapeRenderer;
    Ptr<CoreGraphics::TextRenderer> textRenderer;
    Util::Array<ViewIndependentCall> preLogicCalls, postLogicCalls;
    Util::Array<ViewDependentCall> preLogicViewCalls, postLogicViewCalls;
    SwapInfo swapInfo;
    bool isOpen;
};
static GraphicsEntityId
CreateEntity()
{
    return GraphicsServer::Instance()->CreateGraphicsEntity();
}
static void
DestroyEntity(const GraphicsEntityId id)
{
    GraphicsServer::Instance()->DiscardGraphicsEntity(id);
}
template<typename ... CONTEXTS>
static void
RegisterEntity(const GraphicsEntityId id)
{
    (CONTEXTS::RegisterEntity(id), ...);
}
template<typename ... CONTEXTS>
static void
DeregisterEntity(const GraphicsEntityId id)
{
    (CONTEXTS::DeregisterEntity(id), ...);
}
inline const Ptr<View>&
GraphicsServer::GetView(const Util::StringAtom& name)
{
    return this->viewsByName[name];
}
inline const Ptr<View>&
GraphicsServer::GetCurrentView() const
{
    return this->currentView;
}
inline void
GraphicsServer::AddPreViewCall(PreViewCallback callback)
{
    this->preViewCallbacks.Append(callback);
}
inline void
GraphicsServer::AddPostViewCall(PostViewCallback callback)
{
    this->postViewCallbacks.Append(callback);
}
inline void
GraphicsServer::SetResizeCall(void(*func)(const SizeT windowWidth, const SizeT windowHeight))
{
    this->resizeCall = func;
}
inline void
GraphicsServer::SetSwapInfo(const SwapInfo& info)
{
    this->swapInfo = info;
}
inline void
GraphicsServer::SetupPreLogicCalls(const Util::Array<ViewIndependentCall>& calls)
{
    this->preLogicCalls = calls;
}
inline void
GraphicsServer::SetupPostLogicCalls(const Util::Array<ViewIndependentCall>& calls)
{
    this->postLogicCalls = calls;
}
inline void
GraphicsServer::SetupPreLogicViewCalls(const Util::Array<ViewDependentCall>& calls)
{
    this->preLogicViewCalls = calls;
}
inline void
GraphicsServer::SetupPostLogicViewCalls(const Util::Array<ViewDependentCall>& calls)
{
    this->postLogicViewCalls = calls;
}
inline const Timing::Time
GraphicsServer::GetTime() const
{
    return this->frameContext.time;
}
inline const Timing::Time
GraphicsServer::GetFrameTime() const
{
    return this->frameContext.frameTime;
}
inline const IndexT
GraphicsServer::GetFrameIndex() const
{
    return this->frameContext.frameIndex;
}
}