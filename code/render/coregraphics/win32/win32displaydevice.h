#pragma once
#include "coregraphics/base/displaydevicebase.h"
namespace Win32
{
class Win32DisplayDevice : public Base::DisplayDeviceBase
{
    __DeclareClass(Win32DisplayDevice);
    __DeclareSingleton(Win32DisplayDevice);
public:
    Win32DisplayDevice();
    virtual ~Win32DisplayDevice();
    virtual bool Open();
    virtual void Close();
    virtual void Reopen();
    virtual void ProcessWindowMessages();
    virtual void PostEvent(const Win32::Win32Event& winEvent);
    HWND GetHwnd() const;
protected:
    static LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual bool WindowOpen();
    virtual void CloseWindow();
    virtual bool EmbedWindow();
    virtual void OnMinimized();
    virtual void OnRestored();
    virtual bool OnSetCursor();
    virtual void OnPaint();
    virtual void OnSetFocus();
    virtual void OnKillFocus();
    virtual void OnCloseRequested();
    virtual void OnToggleFullscreenWindowed();
    virtual void OnResize(WORD width, WORD height);
    virtual void OnKeyDown(LPARAM lParam, WPARAM wParam);
    virtual void OnKeyUp(LPARAM lParam, WPARAM wParam);
    virtual void OnChar(WPARAM wParam);
    virtual void OnMouseButton(UINT uMsg, LPARAM lParam);
    virtual void OnMouseMove(LPARAM lParam);
    virtual void OnMouseWheel(WPARAM wParam);
    Input::Key::Code TranslateKeyCode(LPARAM lParam, WPARAM wParam) const;
    virtual CoreGraphics::DisplayMode ComputeAdjustedWindowRect();
    Math::float2 ComputeAbsMousePos(LPARAM lParam) const;
    Math::float2 ComputeNormMousePos(const Math::float2& absMousePos) const;
    HINSTANCE hInst;
    HWND hWnd;
    HWND parentHwnd;
    HACCEL hAccel;
    DWORD windowedStyle;
    DWORD fullscreenStyle;
    DWORD childWindowStyle;
    enum
    {
        AccelToggleFullscreen = 1001,
    };
};
inline HWND
Win32DisplayDevice::GetHwnd() const
{
    return this->hWnd;
}
}