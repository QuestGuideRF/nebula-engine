//------------------------------------------------------------------------------
//  imguifeatureunit.cc
//  (C) 2012-2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "render/stdneb.h"
#include "imguiaddon.h"
#include "input/inputserver.h"
#include "imguirenderer.h"

namespace Dynui
{
__ImplementClass(Dynui::ImguiAddon, 'IMFU', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
ImguiAddon::ImguiAddon()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ImguiAddon::~ImguiAddon()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
ImguiAddon::Setup()
{
    // зарегистрировать контекст
    Dynui::ImguiContext::Create();

    // создать и зарегистрировать обработчик ввода
    this->inputHandler = ImguiInputHandler::Create();
    Input::InputServer::Instance()->AttachInputHandler(Input::InputPriority::DynUi, this->inputHandler.upcast<Input::InputHandler>());
}

//------------------------------------------------------------------------------
/**
*/
void
ImguiAddon::Discard()
{
    // удалить обработчик ввода
    Input::InputServer::Instance()->RemoveInputHandler(this->inputHandler.upcast<Input::InputHandler>());
    this->inputHandler = nullptr;

    Dynui::ImguiContext::Destroy();
}

} // пространство имен Dynui