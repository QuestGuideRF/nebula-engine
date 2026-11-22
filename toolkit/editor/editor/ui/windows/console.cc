//------------------------------------------------------------------------------
//  console.cc
//  (C) 2018 г. Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "console.h"
#include "imgui.h"
#include "graphics/graphicsserver.h"
#include "frame/frameserver.h"
#include "coregraphics/texture.h"
#include "input/inputserver.h"
#include "input/keyboard.h"

namespace Presentation
{
__ImplementClass(Presentation::Console, 'cosl', Presentation::BaseWindow)

//------------------------------------------------------------------------------
/**
*/
Console::Console()
{
    // По умолчанию значение false, пока мы не реализуем профили пользователей.
    this->open = false;

    if (!Dynui::ImguiConsole::HasInstance())
        this->console = Dynui::ImguiConsole::Create();
    else
        this->console = Dynui::ImguiConsole::Instance();

    this->consoleHandler = Dynui::ImguiConsoleHandler::Create();
    this->console->Setup();
    this->consoleHandler->Setup();
}

//------------------------------------------------------------------------------
/**
*/
Console::~Console()
{
    if (this->consoleHandler.isvalid())
    {
        this->consoleHandler->Discard();
        this->consoleHandler = nullptr;
        this->console = nullptr;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
Console::Run(SaveMode save)
{
    this->console->RenderContent();
}

//------------------------------------------------------------------------------
/**
*/
void
Console::Update()
{
    // Это следует переместить в какое-то централизованное место для всех сочетаний клавиш.
    ImGuiIO& io = ImGui::GetIO();
    if (Input::InputServer::Instance()->GetDefaultKeyboard()->KeyDown(Input::Key::F8) || io.KeysData[Input::Key::F8].DownDuration == 0.0f)
    {
        // Переключить окно
        this->open = !open;
    }
}

} // Презентация пространства имен


