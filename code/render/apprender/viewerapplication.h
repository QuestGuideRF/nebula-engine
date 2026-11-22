#pragma once
#include "apprender/renderapplication.h"
#include "graphics/graphicsserver.h"
#include "graphics/stage.h"
#include "graphics/view.h"
#include "graphics/cameraentity.h"
#include "renderutil/mayacamerautil.h"
#include "renderutil/freecamerautil.h"
#include "debugrender/debugshaperenderer.h"
#include "debugrender/debugtextrenderer.h"
namespace App
{
class ViewerApplication : public RenderApplication
{
public:
    ViewerApplication();
    virtual ~ViewerApplication();
    virtual bool Open();
    virtual void Close();
protected:
    virtual void OnProcessInput();
    virtual void OnUpdateFrame();
    bool useResolveRect;
    Math::rectangle<int> resolveRect;
    Ptr<Graphics::GraphicsServer> graphicsServer;
    Ptr<Debug::DebugShapeRenderer> debugShapeRenderer;
    Ptr<Debug::DebugTextRenderer> debugTextRenderer;
    Ptr<Graphics::Stage> stage;
    Ptr<Graphics::View> view;
    Ptr<Graphics::CameraEntity> camera;
#ifndef FREECAM
    RenderUtil::MayaCameraUtil mayaCameraUtil;
#else
    RenderUtil::FreeCameraUtil freeCameraUtil;
#endif
};
}