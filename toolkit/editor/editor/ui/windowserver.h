#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "window.h"
#include "util/string.h"
#include "io/uri.h"
#include "util/delegate.h"
#include "input/key.h"
namespace Presentation
{
class WindowServer : public Core::RefCounted
{
    __DeclareClass(WindowServer);
    __DeclareSingleton(WindowServer);
public:
    WindowServer();
    ~WindowServer();
    void RunAll();
    void Update();
    void BroadcastSave(BaseWindow::SaveMode mode);
    void RegisterWindow(const Util::String& className, const char* label, const char* category = NULL);
    void RegisterWindow(const Util::FourCC fourcc, const char* label, const char* category = NULL);
    void RegisterWindow(const Ptr<BaseWindow>& base);
    void RegisterWindowScript(const char* script, const char* label);
    void RegisterCommand(Util::Delegate<void()> func, Util::String const& label, Util::String const& shortcut, const char* menu = NULL, const char* category = NULL);
    Ptr<BaseWindow> GetWindow(const Util::String& name);
private:
    void AddCategory(const Util::String& category);
    Util::HashTable<Util::String, Ptr<BaseWindow>> windowByName;
    Util::Array<Util::String> categories;
    Util::Array<Ptr<BaseWindow>> windows;
    BaseWindow::SaveMode save;
    struct CommandInfo
    {
        Util::Delegate<void()> func;
        Util::String label;
        Util::String shortcut;
        Util::FixedArray<Input::Key::Code> keys;
        Util::String menu;
        Util::String category;
    };
    Util::Dictionary<Util::String, CommandInfo> commands;
};
}