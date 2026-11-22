//------------------------------------------------------------------------------
//  inputserverbase.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "input/base/inputserverbase.h"
#include "input/inputhandler.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "input/gamepad.h"

namespace Base
{
__ImplementClass(Base::InputServerBase, 'IPSB', Core::RefCounted);

using namespace Input;
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
InputServerBase::InputServerBase() :
    isOpen(false),
    inBeginFrame(false),
    isQuitRequested(false),
    hasFocus(false),
    inputHandlersLockCount(0),
    maxNumLocalPlayers(4),
    defaultGamePad(GamePad::GetMaxNumControllers()),
    creatorThreadId(Threading::Thread::GetMyThreadId())
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
InputServerBase::~InputServerBase()
{
    if (this->IsOpen())
    {
        this->Close();
    }
}

//------------------------------------------------------------------------------
/**
    Setup the maximum number of local players for this application. The
    default number is 1. This defines the number of game pad objects
    created and queried.
*/
void
InputServerBase::SetMaxNumLocalPlayers(SizeT num)
{
    n_assert(!this->isOpen);
    n_assert(num <= GamePad::GetMaxNumControllers());
    this->maxNumLocalPlayers = num;
}

//------------------------------------------------------------------------------
/**
*/
void
InputServerBase::Open()
{
    n_assert(!this->isOpen);
    n_assert(!this->inBeginFrame);
    this->isOpen = true;
    this->hasFocus = true; // Предположим, у нас есть фокус при открытии
}

//------------------------------------------------------------------------------
/**
*/
void
InputServerBase::Close()
{
    n_assert(this->IsOpen());

    // удалить все обработчики ввода
    IndexT i;
    for (i = 0; i < this->inputHandlers.Size(); i++)
    {
        this->inputHandlers[i].Value()->OnRemove();
    }
    this->inputHandlers.Clear();
    this->keyboardCaptureHandler = nullptr;
    this->mouseCaptureHandler = nullptr;

    // освободить обработчики ввода по умолчанию
    this->defaultKeyboard = nullptr;
    this->defaultMouse = nullptr;
    this->defaultGamePad.Fill(nullptr);

    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
*/
void
InputServerBase::Reset()
{
    InputEvent resetEvent;
    resetEvent.SetType(InputEvent::Reset);
    this->PutEvent(resetEvent);
    if (this->mouseCaptureHandler.isvalid())
    {
        this->ReleaseMouseCapture(this->mouseCaptureHandler);
    }
    if (this->keyboardCaptureHandler.isvalid())
    {
        this->ReleaseKeyboardCapture(this->keyboardCaptureHandler);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
InputServerBase::AttachInputHandler(InputPriority::Code pri, const Ptr<InputHandler>& inputHandler)
{
    n_assert(0 == this->inputHandlersLockCount);

    // убедитесь, что обработчик ввода еще не подключен
    IndexT i;
    for (i = 0; i < this->inputHandlers.Size(); i++)
    {
        n_assert(inputHandler != this->inputHandlers[i].Value());
    }    

    // добавить новую запись в отсортированный массив обработчиков ввода
    KeyValuePair<InputPriority::Code,Ptr<InputHandler> > kvp(pri, inputHandler);
    this->inputHandlers.InsertSorted(kvp);
    inputHandler->OnAttach();
}

//------------------------------------------------------------------------------
/**
*/
void
InputServerBase::RemoveInputHandler(const Ptr<InputHandler>& inputHandler)
{
    n_assert(0 == this->inputHandlersLockCount);

    // проверьте, захватывает ли обработчик ввода в данный момент ввод
    if (this->mouseCaptureHandler.isvalid() && (this->mouseCaptureHandler == inputHandler))
    {
        this->ReleaseMouseCapture(inputHandler);
    }
    if (this->keyboardCaptureHandler.isvalid() && (this->keyboardCaptureHandler == inputHandler))
    {
        this->ReleaseKeyboardCapture(inputHandler);
    }

    // удалить обработчик ввода
    IndexT i;
    for (i = 0; i < this->inputHandlers.Size(); i++)
    {
        if (inputHandler == this->inputHandlers[i].Value())
        {
            this->inputHandlers[i].Value()->OnRemove();
            this->inputHandlers.EraseIndex(i);
            return;
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void
InputServerBase::BeginFrame()
{
    n_assert(!this->inBeginFrame);
    this->inBeginFrame = true;

    // уведомить обработчики ввода
    this->inputHandlersLockCount++;
    IndexT i;
    for (i = 0; i < this->inputHandlers.Size(); i++)
    {
        this->inputHandlers[i].Value()->OnBeginFrame();
    }
    this->inputHandlersLockCount--;
}

//------------------------------------------------------------------------------
/**
*/
void
InputServerBase::OnFrame()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Вызовите это где-нибудь в конце кадра, когда это будет гарантировано.
    что данные больше никому не нужны.
*/
void
InputServerBase::EndFrame()
{
    n_assert(this->inBeginFrame);

    // уведомить обработчики ввода
    this->inputHandlersLockCount++;
    IndexT i;
    for (i = 0; i < this->inputHandlers.Size(); i++)
    {
        this->inputHandlers[i].Value()->OnEndFrame();
    }
    this->inputHandlersLockCount--;
    this->inBeginFrame = false;
}

//------------------------------------------------------------------------------
/**
    ПРИМЕЧАНИЕ. События MouseMove и RawMouseMove будут распространяться на все
    обработчики ввода независимо от состояния захвата мыши!
*/
void
InputServerBase::PutEvent(const InputEvent& inputEvent)
{
    n_assert2(creatorThreadId == Threading::Thread::GetMyThreadId(), "PutEvent не может быть вызван ни из какого потока, кроме потока создателя!");
    
    // проверить захват мыши
    if (this->mouseCaptureHandler.isvalid())
    {
        // маршрутизировать события мыши исключительно в обработчик захвата мыши
        switch (inputEvent.GetType())
        {
            // не маршрутизируйте эти события исключительно
            case InputEvent::BeginMouseCapture:
            case InputEvent::EndMouseCapture:
                break;

            case InputEvent::AppLoseFocus:
            case InputEvent::AppClose:
            case InputEvent::Reset:
                this->ReleaseMouseCapture(this->mouseCaptureHandler);
                break;

            case InputEvent::MouseButtonDown:
            case InputEvent::MouseButtonUp:
            case InputEvent::MouseButtonDoubleClick:
            case InputEvent::MouseWheelForward:
            case InputEvent::MouseWheelBackward:
                this->mouseCaptureHandler->OnEvent(inputEvent);
                return;
            default:
                break;
        }
    }

    // проверить захват клавиатуры
    if (this->keyboardCaptureHandler.isvalid())
    {
        // маршрутизировать события клавиатуры исключительно в обработчик захвата клавиатуры
        switch (inputEvent.GetType())
        {
            // не маршрутизируйте эти события исключительно
            case InputEvent::BeginKeyboardCapture:
            case InputEvent::EndKeyboardCapture:
                break;

            case InputEvent::AppLoseFocus:
            case InputEvent::AppClose:
            case InputEvent::Reset:
                this->ReleaseKeyboardCapture(this->keyboardCaptureHandler);
                break;

            case InputEvent::KeyDown:
            case InputEvent::KeyUp:
            case InputEvent::Character:
                this->keyboardCaptureHandler->OnEvent(inputEvent);
                return;

            default:
                break;
        }
    }

    switch (inputEvent.GetType())
    {
        case InputEvent::AppLoseFocus:
            this->hasFocus = false;
            break;

        case InputEvent::AppObtainFocus:
            this->hasFocus = true;
            break;

        case InputEvent::AppClose:
            this->SetQuitRequested(true);
            break;
            
        default: break;
    }

    // обычная обработка событий ввода
    this->inputHandlersLockCount++;
    //bool обработан = ложь;
    IndexT i;
    for (i = 0; i < this->inputHandlers.Size(); i++)
    {
        if (this->inputHandlers[i].Value()->OnEvent(inputEvent))
        {
            //обработано = правда;
            break;
        }
    }
    //n_assert (обработано);
    this->inputHandlersLockCount--;
}

//------------------------------------------------------------------------------
/**
    Получите захват мыши. Весь ввод с помощью мыши будет осуществляться исключительно
    захватывать обработчик ввода до вызова ReleaseMouseCapture().
*/
void
InputServerBase::ObtainMouseCapture(const Ptr<InputHandler>& inputHandler)
{
    if (this->mouseCaptureHandler.isvalid())
    {
        this->mouseCaptureHandler->OnReleaseCapture();
        this->mouseCaptureHandler = nullptr;
    }
    this->mouseCaptureHandler = inputHandler;
    this->mouseCaptureHandler->OnObtainCapture();
}

//------------------------------------------------------------------------------
/**
    Отпустите захват мыши. Предоставленный указатель должен соответствовать текущему
    захват обработчика ввода.
*/
void
InputServerBase::ReleaseMouseCapture(const Ptr<InputHandler>& inputHandler)
{
    if (this->mouseCaptureHandler.isvalid() && (this->mouseCaptureHandler == inputHandler))
    {
        this->mouseCaptureHandler->OnReleaseCapture();
        this->mouseCaptureHandler = nullptr;
    }
}

//------------------------------------------------------------------------------
/**
*/
const Ptr<InputHandler>&
InputServerBase::GetMouseCaptureHandler() const
{
    return this->mouseCaptureHandler;
}

//------------------------------------------------------------------------------
/**
    This clears the currently set mouse capture (if exists).
*/
void
InputServerBase::ClearMouseCapture()
{
    if (this->mouseCaptureHandler.isvalid())
    {
        this->mouseCaptureHandler->OnReleaseCapture();
        this->mouseCaptureHandler = nullptr;
    }
}

//------------------------------------------------------------------------------
/**
    This clears the currently set keyboard capture (if exists).
*/
void
InputServerBase::ClearKeyboardCapture()
{
    if (this->keyboardCaptureHandler.isvalid())
    {
        this->keyboardCaptureHandler->OnReleaseCapture();
        this->keyboardCaptureHandler = nullptr;
    }
}

//------------------------------------------------------------------------------
/**
    This clears the mouse and keyboards captures, if set.
*/
void
InputServerBase::ClearCapture()
{
    this->ClearMouseCapture();
    this->ClearKeyboardCapture();
}

//------------------------------------------------------------------------------
/**
    Получите захват клавиатуры. Весь ввод с клавиатуры будет осуществляться исключительно
    захватывать обработчик ввода до тех пор, пока не будет вызван ReleaseKeyboardCapture().
*/
void
InputServerBase::ObtainKeyboardCapture(const Ptr<InputHandler>& inputHandler)
{
    if (this->keyboardCaptureHandler.isvalid())
    {
        this->keyboardCaptureHandler->OnReleaseCapture();
        this->keyboardCaptureHandler = nullptr;
    }
    this->keyboardCaptureHandler = inputHandler;
    this->keyboardCaptureHandler->OnObtainCapture();
}

//------------------------------------------------------------------------------
/**
    Отпустите захват мыши. Предоставленный указатель должен соответствовать текущему
    захват обработчика ввода.
*/
void
InputServerBase::ReleaseKeyboardCapture(const Ptr<InputHandler>& inputHandler)
{
    if (this->keyboardCaptureHandler.isvalid() && (this->keyboardCaptureHandler == inputHandler))
    {
        this->keyboardCaptureHandler->OnReleaseCapture();
        this->keyboardCaptureHandler = nullptr;
    }
}

//------------------------------------------------------------------------------
/**
*/
const Ptr<InputHandler>&
InputServerBase::GetKeyboardCaptureHandler() const
{
    return this->keyboardCaptureHandler;
}
    
//------------------------------------------------------------------------------
/**
*/
Ptr<Input::GamePad>
InputServerBase::GetDefaultGamePad(IndexT playerIndex) const
{
    this->critSect.Enter();
    n_assert(playerIndex < this->maxNumLocalPlayers);
    Ptr<Input::GamePad> result = this->defaultGamePad[playerIndex];
    this->critSect.Leave();
    return result;
}

//------------------------------------------------------------------------------
/**
*/
void 
InputServerBase::AddGamePad(const Ptr<Input::GamePad> pad, IndexT playerIndex)
{
    n_assert(playerIndex < this->maxNumLocalPlayers);
    this->critSect.Enter();
    n_assert(!this->defaultGamePad[playerIndex].isvalid());
    this->defaultGamePad[playerIndex] = pad;
    this->critSect.Leave();
    
}

} // База пространства имен
