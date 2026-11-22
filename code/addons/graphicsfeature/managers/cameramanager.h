#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "game/manager.h"
#include "ids/idgenerationpool.h"
#include "graphics/graphicsentity.h"
#include "math/vec3.h"
#include "graphicsfeature/components/camera.h"
namespace Graphics
{
    class View;
}
namespace GraphicsFeature
{
typedef uint ViewHandle;
class CameraManager : public Game::Manager
{
    __DeclareClass(CameraManager)
    __DeclareSingleton(CameraManager)
public:
    CameraManager();
    virtual ~CameraManager();
    void OnActivate() override;
    void OnDeactivate() override;
    static ViewHandle RegisterView(Ptr<Graphics::View> const& view);
    static bool IsViewHandleValid(ViewHandle handle);
    static Math::mat4 GetProjection(ViewHandle handle);
    static Math::mat4 GetLocalTransform(ViewHandle handle);
private:
    void InitUpdateCameraProcessor();
    struct ViewData
    {
        Graphics::GraphicsEntityId gid;
        Ptr<Graphics::View> view;
        Camera currentSettings;
    };
    Util::Array<ViewData> viewHandleMap;
    Ids::IdGenerationPool viewHandlePool;
};
inline bool
CameraManager::IsViewHandleValid(ViewHandle handle)
{
    n_assert(CameraManager::HasInstance());
    return CameraManager::Singleton->viewHandlePool.IsValid(handle);
}
}