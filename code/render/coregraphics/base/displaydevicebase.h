#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "coregraphics/antialiasquality.h"
#include "coregraphics/adapter.h"
#include "coregraphics/displayeventhandler.h"
#include "coregraphics/displaymode.h"
#include "coregraphics/adapterinfo.h"
#include "util/blob.h"
#include "coregraphics/window.h"
namespace Base
{
class DisplayDeviceBase : public Core::RefCounted
{
    __DeclareClass(DisplayDeviceBase);
    __DeclareSingleton(DisplayDeviceBase);
public:
    DisplayDeviceBase();
    virtual ~DisplayDeviceBase();
    bool Open();
    void Close();
    bool IsOpen() const;
    void ProcessWindowMessages();
    void Reopen();
    void SetVerticalSyncEnabled(bool b);
    const bool IsVerticalSyncEnabled() const;
    bool AdapterExists(CoreGraphics::Adapter::Code adapter);
    Util::Array<CoreGraphics::DisplayMode> GetAvailableDisplayModes(CoreGraphics::Adapter::Code adapter, CoreGraphics::PixelFormat::Code pixelFormat);
    bool SupportsDisplayMode(CoreGraphics::Adapter::Code adapter, const CoreGraphics::DisplayMode& requestedMode);
    CoreGraphics::DisplayMode GetCurrentAdapterDisplayMode(CoreGraphics::Adapter::Code adapter);
    CoreGraphics::AdapterInfo GetAdapterInfo(CoreGraphics::Adapter::Code adapter);
    void SetAdapter(CoreGraphics::Adapter::Code a);
    CoreGraphics::Adapter::Code GetAdapter() const;
    const bool IsFullscreen() const;
    void AttachEventHandler(const Ptr<CoreGraphics::DisplayEventHandler>& h);
    void RemoveEventHandler(const Ptr<CoreGraphics::DisplayEventHandler>& h);
    CoreGraphics::WindowId SetupWindow(const Util::String& title, const Util::String& icon, const CoreGraphics::DisplayMode& displayMode, const CoreGraphics::AntiAliasQuality::Code aa = CoreGraphics::AntiAliasQuality::None);
    CoreGraphics::WindowId EmbedWindow(const Util::Blob& windowData);
    CoreGraphics::WindowId GetMainWindow() const;
    const CoreGraphics::WindowId GetCurrentWindow() const;
    const CoreGraphics::WindowId GetWindow(IndexT index) const;
    const Util::Array<CoreGraphics::WindowId>& GetWindows() const;
    void MakeWindowCurrent(const CoreGraphics::WindowId id);
protected:
    bool NotifyEventHandlers(const CoreGraphics::DisplayEvent& e);
    CoreGraphics::Adapter::Code adapter;
    bool verticalSync;
    bool isOpen;
    bool isFullscreen;
    Util::Array<Ptr<CoreGraphics::DisplayEventHandler> > eventHandlers;
    CoreGraphics::WindowId currentWindow;
    Util::Array<CoreGraphics::WindowId> windows;
};
inline bool
DisplayDeviceBase::IsOpen() const
{
    return this->isOpen;
}
inline void
DisplayDeviceBase::SetVerticalSyncEnabled(bool b)
{
    this->verticalSync = b;
}
inline const bool
DisplayDeviceBase::IsVerticalSyncEnabled() const
{
    return this->verticalSync;
}
inline void
DisplayDeviceBase::SetAdapter(CoreGraphics::Adapter::Code a)
{
    this->adapter = a;
}
inline CoreGraphics::Adapter::Code
DisplayDeviceBase::GetAdapter() const
{
    return this->adapter;
}
inline const bool
DisplayDeviceBase::IsFullscreen() const
{
    return this->isFullscreen;
}
inline CoreGraphics::WindowId
DisplayDeviceBase::GetMainWindow() const
{
    if (this->windows.Size() >= 1) return this->windows[0];
    else                          return NULL;
}
inline const CoreGraphics::WindowId
DisplayDeviceBase::GetCurrentWindow() const
{
    return this->currentWindow;
}
inline const CoreGraphics::WindowId
DisplayDeviceBase::GetWindow(IndexT index) const
{
    n_assert(this->windows.Size() > index && index != InvalidIndex);
    return this->windows[index];
}
inline const Util::Array<CoreGraphics::WindowId>&
DisplayDeviceBase::GetWindows() const
{
    return this->windows;
}
}