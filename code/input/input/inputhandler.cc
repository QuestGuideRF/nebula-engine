//------------------------------------------------------------------------------
//  inputhandler.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "input/inputhandler.h"

namespace Input
{
__ImplementClass(Input::InputHandler, 'INPH', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
InputHandler::InputHandler() :
    isAttached(false),
    isCapturing(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
InputHandler::~InputHandler()
{
    n_assert(!this->IsAttached());
}

//------------------------------------------------------------------------------
/**
    Начните захват ввода в этот обработчик ввода. Этот метод должен быть
    переопределено в подклассе, производный метод должен вызвать 
    ObtainMouseCapture(), ObtainKeyboardCapture() или оба, в зависимости от
    от того, какие события ввода типа вы хотите захватить. Обработчик ввода
    который захватывает входные данные, получает исключительно все входные события данного типа.
*/
void
InputHandler::BeginCapture()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Завершить захват ввода в этот обработчик ввода. Переопределить этот метод
    в подклассе и освободить захваты, полученные в BeginCapture().
*/
void
InputHandler::EndCapture()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
InputHandler::OnAttach()
{
    n_assert(!this->IsAttached());
    this->isAttached = true;
}

//------------------------------------------------------------------------------
/**
*/
void
InputHandler::OnRemove()
{
    n_assert(this->IsAttached());
    this->isAttached = false;
}

//------------------------------------------------------------------------------
/**
*/
void
InputHandler::OnBeginFrame()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
InputHandler::OnEndFrame()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
InputHandler::OnObtainCapture()
{
    this->isCapturing = true;
}

//------------------------------------------------------------------------------
/**
*/
void
InputHandler::OnReleaseCapture()
{
    this->isCapturing = false;
}

//------------------------------------------------------------------------------
/**
*/
bool
InputHandler::OnEvent(const InputEvent& inputEvent)
{
    switch (inputEvent.GetType())
    {
#ifndef _DEBUG
        case InputEvent::AppObtainFocus:
        case InputEvent::AppLoseFocus:
#endif
        case InputEvent::Reset:
            this->OnReset();
            break;

        default:
            break;
    }        
    return true;
}

//------------------------------------------------------------------------------
/**
    OnReset is called when the app loses or gains focus (amongst other
    occasions). The input handler should reset its  state
    to prevent keys from sticking down, etc...
*/
void
InputHandler::OnReset()
{
    // empty
}

} // пространство имен Ввод


