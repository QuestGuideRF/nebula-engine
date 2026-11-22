#pragma once
#include "GLFW/glfw3.h"
#include "coregraphics/base/displaydevicebase.h"
#include "util/array.h"
#include "threading/thread.h"
#include "input/glfw/glfwinputdisplayeventhandler.h"
#include "graphics/glfw/glfwgraphicsdisplayeventhandler.h"
namespace GLFW
{
class GLFWDisplayDevice : public Base::DisplayDeviceBase
{
    __DeclareClass(GLFWDisplayDevice);
    __DeclareSingleton(GLFWDisplayDevice);
public:
    GLFWDisplayDevice();
    virtual ~GLFWDisplayDevice();
    bool Open();
    void Close();
    static void ProcessWindowMessages();
    bool AdapterExists(CoreGraphics::Adapter::Code adapter);
    Util::Array<CoreGraphics::DisplayMode> GetAvailableDisplayModes(CoreGraphics::Adapter::Code adapter, CoreGraphics::PixelFormat::Code pixelFormat);
    bool SupportsDisplayMode(CoreGraphics::Adapter::Code adapter, const CoreGraphics::DisplayMode& requestedMode);
    CoreGraphics::DisplayMode GetCurrentAdapterDisplayMode(CoreGraphics::Adapter::Code adapter);
    CoreGraphics::AdapterInfo GetAdapterInfo(CoreGraphics::Adapter::Code adapter);
    static Input::Key::Code TranslateKeyCode(int inkey);
protected:
    Ptr<GLFWInputDisplayEventHandler> inputEventHandler;
    Ptr<GLFWGraphicsDisplayEventHandler> graphicsEventHandler;
    friend class GLFWWindow;
    friend class OGL4RenderDevice;
    friend class GLFWInputServer;
    friend void CoreGraphics::DestroyWindow(const CoreGraphics::WindowId id);
    friend void KeyFunc(const CoreGraphics::WindowId& id, int key, int scancode, int action, int mods);
    friend void CharFunc(const CoreGraphics::WindowId& id, unsigned int key);
    friend void MouseButtonFunc(const CoreGraphics::WindowId& id, int button, int action, int mods);
    friend void MouseFunc(const CoreGraphics::WindowId& id, double xpos, double ypos);
    friend void ScrollFunc(const CoreGraphics::WindowId& id, double xs, double ys);
    friend void CloseFunc(const CoreGraphics::WindowId& id);
    friend void FocusFunc(const CoreGraphics::WindowId& id, int focus);
    friend void ResizeFunc(const CoreGraphics::WindowId& id, int width, int height);
    friend void CoreGraphics::WindowNewFrame(const CoreGraphics::WindowId id);
    friend const CoreGraphics::WindowId InternalSetupFunction(const CoreGraphics::WindowCreateInfo& info, const Util::Blob& windowData, bool embed);
    GLFWmonitor* GetMonitor(int index);
};
}