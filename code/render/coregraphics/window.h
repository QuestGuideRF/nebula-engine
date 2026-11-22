#pragma once
#include "ids/id.h"
#include "ids/idpool.h"
#include "coregraphics/displaymode.h"
#include "coregraphics/antialiasquality.h"
#include "coregraphics/adapter.h"
#include "util/stringatom.h"
#include "util/blob.h"
#if defined(CreateWindow)
#undef CreateWindow
#endif
namespace CoreGraphics
{
ID_24_8_TYPE(WindowId);
struct TextureId;
struct SwapchainId;
extern WindowId CurrentWindow;
struct WindowCreateInfo
{
    CoreGraphics::DisplayMode mode;
    Util::StringAtom title;
    Util::StringAtom icon;
    CoreGraphics::AntiAliasQuality::Code aa;
    bool resizable : 1;
    bool decorated : 1;
    bool fullscreen : 1;
    bool vsync : 1;
};
const WindowId CreateWindow(const WindowCreateInfo& info);
const WindowId EmbedWindow(const Util::Blob& windowData);
void DestroyWindow(const WindowId id);
void WindowResize(const WindowId id, SizeT newWidth, SizeT newHeight);
void WindowSetTitle(const WindowId id, const Util::String& title);
void WindowMakeCurrent(const WindowId id);
void WindowPresent(const WindowId id, const IndexT frameIndex);
void WindowPollEvents();
void WindowNewFrame(const WindowId id);
void WindowApplyFullscreen(const WindowId id, Adapter::Code monitor, bool b);
void WindowSetCursorVisible(const WindowId id, bool b);
void WindowSetCursorLocked(const WindowId id, bool b);
const CoreGraphics::DisplayMode WindowGetDisplayMode(const WindowId id);
const CoreGraphics::AntiAliasQuality::Code WindowGetAAQuality(const WindowId id);
const bool WindowIsFullscreen(const WindowId id);
const bool WindowIsDecorated(const WindowId id);
const bool WindowIsResizable(const WindowId id);
const Util::StringAtom& WindowGetTitle(const WindowId id);
const Util::StringAtom& WindowGetIcon(const WindowId id);
Math::vec2 WindowGetContentScale(const WindowId id);
const CoreGraphics::SwapchainId WindowGetSwapchain(const WindowId id);
}