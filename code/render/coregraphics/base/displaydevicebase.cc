//------------------------------------------------------------------------------
//  displaydevicebase.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coregraphics/base/displaydevicebase.h"
#include "frame/frameserver.h"

namespace Base
{
__ImplementClass(Base::DisplayDeviceBase, 'DSDB', Core::RefCounted);
__ImplementSingleton(Base::DisplayDeviceBase);

using namespace CoreGraphics;

//------------------------------------------------------------------------------
/**
*/
DisplayDeviceBase::DisplayDeviceBase() :
    adapter(Adapter::Primary),
    verticalSync(false),
    isOpen(false)
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
DisplayDeviceBase::~DisplayDeviceBase()
{
    n_assert(!this->IsOpen());
    __DestructSingleton;
}

//------------------------------------------------------------------------------
/**
    Откройте дисплей.
*/
bool
DisplayDeviceBase::Open()
{
    n_assert(!this->IsOpen());
    this->isOpen = true;
    return true;
}

//------------------------------------------------------------------------------
/**
    Закройте дисплей.
*/
void
DisplayDeviceBase::Close()
{
    n_assert(this->IsOpen());
    this->isOpen = false;

    IndexT i;
    for (i = 0; i < this->windows.Size(); i++)
    {
        CoreGraphics::DestroyWindow(this->windows[i]);
    }
    this->windows.Clear();
}

//------------------------------------------------------------------------------
/**
*/
void 
DisplayDeviceBase::Reopen()
{
    n_assert(this->IsOpen());

    // уведомить все обработчики событий
    DisplayEvent closeEvent(DisplayEvent::WindowReopen);
    this->NotifyEventHandlers(closeEvent);
}

//------------------------------------------------------------------------------
/**
    Прикрепите обработчик событий к устройству отображения.
*/
void
DisplayDeviceBase::AttachEventHandler(const Ptr<DisplayEventHandler>& handler)
{
    n_assert(handler.isvalid());
    n_assert(InvalidIndex == this->eventHandlers.FindIndex(handler));
    this->eventHandlers.Append(handler);
    handler->OnAttach();
}

//------------------------------------------------------------------------------
/**
    Удалите обработчик событий с устройства отображения.
*/
void
DisplayDeviceBase::RemoveEventHandler(const Ptr<DisplayEventHandler>& handler)
{
    n_assert(handler.isvalid());
    IndexT index = this->eventHandlers.FindIndex(handler);
    n_assert(InvalidIndex != index);
    this->eventHandlers.EraseIndex(index);
    handler->OnRemove();
}

//------------------------------------------------------------------------------
/**
    Уведомить все обработчики событий о событии.
*/
bool
DisplayDeviceBase::NotifyEventHandlers(const DisplayEvent& event)
{
    bool handled = false;
    IndexT i;
    for (i = 0; i < this->eventHandlers.Size(); i++)
    {
        handled |= this->eventHandlers[i]->PutEvent(event);
    }
    return handled;
}

//------------------------------------------------------------------------------
/**
    Обрабатывать сообщения оконной системы. Переопределить этот метод в подклассе.
*/
void
DisplayDeviceBase::ProcessWindowMessages()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Возвращает режимы отображения на данном адаптере в заданном формате пикселей.
*/
Util::Array<DisplayMode>
DisplayDeviceBase::GetAvailableDisplayModes(Adapter::Code adapter, PixelFormat::Code pixelFormat)
{
    // переопределить этот метод в подклассе, чтобы сделать что-нибудь полезное
    Util::Array<DisplayMode> emptyArray;
    return emptyArray;
}

//------------------------------------------------------------------------------
/**
    This method checks the available display modes on the given adapter
    against the requested display modes and returns true if the display mode
    exists.
*/
bool
DisplayDeviceBase::SupportsDisplayMode(Adapter::Code adapter, const DisplayMode& requestedMode)
{
    return false;
}

//------------------------------------------------------------------------------
/**
    Этот метод возвращает текущий режим отображения адаптера. Его можно использовать
    чтобы получить текущий режим отображения рабочего стола.
*/
DisplayMode
DisplayDeviceBase::GetCurrentAdapterDisplayMode(Adapter::Code adapter)
{
    DisplayMode emptyMode;
    return emptyMode;
}

//------------------------------------------------------------------------------
/**
    Проверяет, существует ли данный адаптер.
*/
bool
DisplayDeviceBase::AdapterExists(Adapter::Code adapter)
{
    return false;
}

//------------------------------------------------------------------------------
/**
    Возвращает информацию о предоставленном адаптере.
*/
AdapterInfo
DisplayDeviceBase::GetAdapterInfo(Adapter::Code adapter)
{
    AdapterInfo emptyAdapterInfo;
    return emptyAdapterInfo;
}

//------------------------------------------------------------------------------
/**
*/
CoreGraphics::WindowId
DisplayDeviceBase::SetupWindow(const Util::String& title, const Util::String& icon, const CoreGraphics::DisplayMode& displayMode, const CoreGraphics::AntiAliasQuality::Code aa)
{
    WindowCreateInfo info;
    info.aa = aa;
    info.mode = displayMode;
    info.title = title;
    info.icon = icon;
    CoreGraphics::WindowId wnd = CoreGraphics::CreateWindow(info);

    // добавить в список и установить текущий, если это первый
    if (this->windows.IsEmpty()) this->currentWindow = wnd;
    this->windows.Append(wnd);

    return wnd;
}

//------------------------------------------------------------------------------
/**
*/
CoreGraphics::WindowId
DisplayDeviceBase::EmbedWindow(const Util::Blob& windowData)
{
    CoreGraphics::WindowId wnd = CoreGraphics::EmbedWindow(windowData);

    // добавить в список и установить текущий, если это первый
    if (this->windows.IsEmpty()) this->currentWindow = wnd;
    this->windows.Append(wnd);
    return wnd;
}

//------------------------------------------------------------------------------
/**
*/
void
DisplayDeviceBase::MakeWindowCurrent(const CoreGraphics::WindowId id)
{
    CoreGraphics::WindowMakeCurrent(id);
    this->currentWindow = id;
}

} // пространство имен DisplayDevice
