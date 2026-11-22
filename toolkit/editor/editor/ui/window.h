#pragma once
#include "util/string.h"
#include "core/refcounted.h"
#include "imgui.h"
namespace Presentation
{
class BaseWindow : public Core::RefCounted
{
    __DeclareClass(BaseWindow)
public:
    enum class SaveMode
    {
        None,
        SaveActive,
        SaveAll
    };
    BaseWindow();
    BaseWindow(Util::String name);
    virtual ~BaseWindow();
    const Util::String GetName() const;
    void SetName(const char* name);
    const Util::String& GetCategory() const;
    void SetCategory(const char* category);
    bool& Open();
    virtual void Run(SaveMode save);
    virtual void Update();
    const ImGuiWindowFlags_ GetAdditionalFlags() const;
    Math::vec2 GetWindowPadding() const;
    void SetWindowPadding(const Math::vec2& padding);
    Math::vec2 GetPosition() const;
    Math::vec2 GetSize() const;
    void SetPosition(const Math::vec2& pos);
    void SetSize(const Math::vec2& size);
    void Edit();
    void Unedit(int count = 1);
    void Save();
    static Util::String FormatName(const Util::String& name, int editCounter);
protected:
    friend class WindowServer;
    ImGuiWindowFlags_ additionalFlags;
    Util::String name;
    Util::String category;
    bool open;
    int editCounter;
private:
    Math::vec2 windowPadding;
    bool usesCustomWindowPadding = false;
};
}