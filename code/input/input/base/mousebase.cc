//------------------------------------------------------------------------------
//  mousebase.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "input/base/mousebase.h"
#include "input/inputserver.h"

namespace Base
{
__ImplementClass(Base::MouseBase, 'MSBS', Input::InputHandler);

using namespace Input;

//------------------------------------------------------------------------------
/**
*/
MouseBase::MouseBase() :
    buttonStates(MouseButton::NumMouseButtons),
    wheelForward(false),
    wheelBackward(false),
    initialMouseMovement(false),
    mouseLocked(false),
    mouseWasLocked(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
MouseBase::~MouseBase()
{
    n_assert(!this->IsAttached());
}

//------------------------------------------------------------------------------
/**
*/
void
MouseBase::BeginCapture()
{
    InputHandler::BeginCapture();
    InputServer::Instance()->ObtainMouseCapture(this);
}

//------------------------------------------------------------------------------
/**
*/
void
MouseBase::EndCapture()
{
    InputHandler::EndCapture();
    InputServer::Instance()->ReleaseMouseCapture(this);
}

//------------------------------------------------------------------------------
/**
*/
void
MouseBase::OnAttach()
{
    InputHandler::OnAttach();

    ButtonState initialButtonState;
    this->buttonStates.Fill(initialButtonState);
    this->beginFramePixelPosition.set(0.0f, 0.0f);
    this->beginFrameScreenPosition.set(0.0f, 0.0f);
    this->pixelPosition.set(0.0f, 0.0f);
    this->screenPosition.set(0.0f, 0.0f);
    this->movement.set(0.0f, 0.0f);
    this->wheelForward = false;
    this->wheelBackward = false;
    this->initialMouseMovement = true;
}

//------------------------------------------------------------------------------
/**
*/
void
MouseBase::OnBeginFrame()
{
    InputHandler::OnBeginFrame();

    // сбросить флаги кнопки вверх/вниз/двойного щелчка, но сохранить нажатое состояние
    IndexT i;
    for (i = 0; i < this->buttonStates.Size(); i++)
    {
        ButtonState& state = this->buttonStates[i];

        // ПРИМЕЧАНИЕ. Если кнопка была отпущена в предыдущем кадре,
        // очистить нажатое состояние (см. обработку ButtonUp
        // код в OnEvent, чтобы узнать, почему это хорошо)
        if (state.up)
        {
            state.pressed = false;
        }
        state.down = false;
        state.up = false;
        state.doubleClicked = false;
    }
    this->wheelForward = false;
    this->wheelBackward = false;
    this->beginFramePixelPosition = this->pixelPosition;
    this->beginFrameScreenPosition = this->screenPosition;
    this->movement.set(0.0f, 0.0f);
}

//------------------------------------------------------------------------------
/**
*/
bool
MouseBase::OnEvent(const InputEvent& inputEvent)
{
    switch (inputEvent.GetType())
    {
        // сбросить обработчик ввода, если другой начинает захватывать
        case InputEvent::BeginMouseCapture:
            if (!this->IsCapturing())
            {
                this->OnReset();
            }
            return true;

        case InputEvent::MouseMove:
            this->UpdateMousePositions(inputEvent.GetAbsMousePos(), inputEvent.GetNormMousePos());
            return true;

        case InputEvent::RawMouseMove:
            this->movement = inputEvent.GetAbsMousePos();
            return true;

        case InputEvent::MouseButtonDown:
            {
                MouseButton::Code btn = inputEvent.GetMouseButton();
                if (MouseButton::InvalidMouseButton != btn)
                {
                    this->buttonStates[btn].down = true;
                    this->buttonStates[btn].pressed = true;
                    this->UpdateMousePositions(inputEvent.GetAbsMousePos(), inputEvent.GetNormMousePos());
                }
            }
            return true;

        case InputEvent::MouseButtonUp:
            {
                MouseButton::Code btn = inputEvent.GetMouseButton();
                if (MouseButton::InvalidMouseButton != btn)
                {
                    this->buttonStates[btn].up = true;
                    // ПРИМЕЧАНИЕ. Здесь мы не снимаем нажатый флаг, потому что
                    // может случиться так, что кнопка была нажата кратковременно
                    // внутри кадра, и в этом случае мы все еще хотим, чтобы
                    // нажатый флаг, который будет установлен для текущего кадра (в противном случае приложение
                    // может пропустить нажатое состояние) вместо этого нажатое состояние будет
                    // очищаться в начале следующего кадра
                    // когда был установлен флаг кнопки вверх
                    this->UpdateMousePositions(inputEvent.GetAbsMousePos(), inputEvent.GetNormMousePos());
                }
            }
            return true;
       
        case InputEvent::MouseButtonDoubleClick:
            {
                MouseButton::Code btn = inputEvent.GetMouseButton();
                if (MouseButton::InvalidMouseButton != btn)
                {
                    this->buttonStates[btn].doubleClicked = true;
                    this->UpdateMousePositions(inputEvent.GetAbsMousePos(), inputEvent.GetNormMousePos());
                }
            }
            return true;

        case InputEvent::MouseWheelForward:
            this->wheelForward = true;
            this->UpdateMousePositions(inputEvent.GetAbsMousePos(), inputEvent.GetNormMousePos());
            return true;

        case InputEvent::MouseWheelBackward:
            this->wheelBackward = true;
            this->UpdateMousePositions(inputEvent.GetAbsMousePos(), inputEvent.GetNormMousePos());
            return true;

        case InputEvent::AppLoseFocus:
            {
                if (this->IsLocked())
                {
                    this->LockToPosition(false, mouseLockedPosition);
                    mouseWasLocked = true;
                }
                break;
            }

        case InputEvent::AppObtainFocus:
            {
                if (mouseWasLocked)
                {
                    this->LockToPosition(true, mouseLockedPosition);
                    mouseWasLocked = false;
                }
                break;
            }

        default:
            break;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
MouseBase::OnObtainCapture()
{
    InputHandler::OnObtainCapture();
    
    // положить начало события захвата мыши
    InputEvent beginCaptureEvent;
    beginCaptureEvent.SetType(InputEvent::BeginMouseCapture);
    InputServer::Instance()->PutEvent(beginCaptureEvent);
}

//------------------------------------------------------------------------------
/**
*/
void
MouseBase::OnReleaseCapture()
{
    // положить конец событию захвата мыши
    InputEvent endCaptureEvent;
    endCaptureEvent.SetType(InputEvent::EndMouseCapture);
    InputServer::Instance()->PutEvent(endCaptureEvent);

    InputHandler::OnReleaseCapture();
}

//------------------------------------------------------------------------------
/**
*/
void
MouseBase::UpdateMousePositions(const Math::vec2& pixelPos, const Math::vec2& screenPos)
{    
    // предотвратить прыжок мыши в начале
    if (this->initialMouseMovement)
    {
        this->beginFramePixelPosition = pixelPos;
        this->beginFrameScreenPosition = screenPos;
        this->initialMouseMovement = false;
    }
    this->pixelPosition = pixelPos;
    this->screenPosition = screenPos;
    this->movement = this->beginFramePixelPosition - this->pixelPosition;
}

//------------------------------------------------------------------------------
/**
*/
void
MouseBase::OnReset()
{
    // кнопки сброса
    IndexT i;
    for (i = 0; i < MouseButton::NumMouseButtons; i++)
    {
        ButtonState& state = this->buttonStates[i];
        if (state.pressed)
        {
            state.up = true;
        }
        else
        {
            state.up = false;
        }
        state.down = false;
        state.pressed = false;
    }
    this->pixelPosition.set(0.0f, 0.0f);
    this->screenPosition.set(0.0f, 0.0f);
    this->movement.set(0.0f, 0.0f);
    this->wheelForward = false;
    this->wheelBackward = false;
    this->initialMouseMovement = true;
}

} // База пространства имен