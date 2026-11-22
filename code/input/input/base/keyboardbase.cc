//------------------------------------------------------------------------------
//  keyboardbase.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "input/base/keyboardbase.h"
#include "input/inputserver.h"

namespace Base
{
__ImplementClass(Base::KeyboardBase, 'KBBS', Input::InputHandler);

using namespace Input;

//------------------------------------------------------------------------------
/**
*/
KeyboardBase::KeyboardBase() :
    keyStates(Key::NumKeyCodes),
    nextKeyStates(Key::NumKeyCodes)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
KeyboardBase::~KeyboardBase()
{
    n_assert(!this->IsAttached());
}

//------------------------------------------------------------------------------
/**
*/
void
KeyboardBase::BeginCapture()
{
    InputHandler::BeginCapture();
    InputServer::Instance()->ObtainKeyboardCapture(this);
}

//------------------------------------------------------------------------------
/**
*/
void
KeyboardBase::EndCapture()
{
    InputHandler::EndCapture();
    InputServer::Instance()->ReleaseKeyboardCapture(this);
}

//------------------------------------------------------------------------------
/**
*/
void
KeyboardBase::OnAttach()
{
    InputHandler::OnAttach();
    KeyState initialKeyState;
    this->keyStates.Fill(initialKeyState);
    this->nextKeyStates.Fill(initialKeyState);
    this->charInput.Clear();
}

//------------------------------------------------------------------------------
/**
*/
void
KeyboardBase::OnBeginFrame()
{
    InputHandler::OnBeginFrame();

    // сбросить состояние вверх и вниз всех клавиш
    IndexT i;
    for (i = 0; i < this->keyStates.Size(); i++)
    {
        KeyState& keyState = this->keyStates[i];
        //KeyState& nextKeyState = this->nextKeyStates[i];

        // ПРИМЕЧАНИЕ: если клавиша была отпущена в предыдущем кадре,
        // очистить нажатое состояние (см. KeyUp-обработку
        // код в OnEvent, чтобы узнать, почему это хорошо)
        if (keyState.up)
        {
            keyState.pressed = false;
        }
        //keyState.pressed = nextKeyState.pressed;
        keyState.down = false;
        keyState.up = false;
        //nextKeyState.down = ложь;
        //nextKeyState.up = ложь;
    }

    // очистить ввод символов
    this->charInput.Clear();
}

//------------------------------------------------------------------------------
/**
*/
bool
KeyboardBase::OnEvent(const InputEvent& inputEvent)
{
    switch (inputEvent.GetType())
    {
        // сбросить обработчик ввода, если другой начинает захватывать
        case InputEvent::BeginKeyboardCapture:
            if (!this->IsCapturing())
            {
                this->OnReset();
            }
            return true;

        case InputEvent::KeyDown:
            // примечание: если мы уже нажали, мы не записываем
            // нажатие клавиши, это происходит, когда начинается повторение клавиши
            {
                KeyState& keyState = this->keyStates[inputEvent.GetKey()];
                if (!keyState.pressed)
                {
                    keyState.down = true;
                    keyState.pressed = true;
                }
            }
            return true;


        case InputEvent::KeyUp:
            {
                KeyState& keyState = this->keyStates[inputEvent.GetKey()];
                keyState.up = true;
                // ПРИМЕЧАНИЕ. Здесь мы не снимаем нажатый флаг, потому что
                // может случиться так, что клавиша была нажата кратковременно
                // внутри кадра, и в этом случае мы все еще хотим, чтобы
                // нажатый флаг, который будет установлен для текущего кадра (в противном случае приложение
                // может пропустить нажатое состояние) вместо этого нажатое состояние будет
                // очищаться в начале следующего кадра
                // когда был установлен флаг нажатия клавиши
            }
            return true;

        case InputEvent::Character:
            {
                uchar chr = inputEvent.GetChar();
                this->charInput.AppendRange((const char*)&chr, 1);
            }
            return true;

        default:
            break;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
KeyboardBase::OnObtainCapture()
{
    InputHandler::OnObtainCapture();

    // положить начало события захвата клавиатуры
    InputEvent beginCaptureEvent;
    beginCaptureEvent.SetType(InputEvent::BeginKeyboardCapture);
    InputServer::Instance()->PutEvent(beginCaptureEvent);
}

//------------------------------------------------------------------------------
/**
*/
void
KeyboardBase::OnReleaseCapture()
{
    // положить конец событию захвата клавиатуры
    InputEvent endCaptureEvent;
    endCaptureEvent.SetType(InputEvent::EndKeyboardCapture);
    InputServer::Instance()->PutEvent(endCaptureEvent);

    InputHandler::OnReleaseCapture();
}

//------------------------------------------------------------------------------
/**
*/
void
KeyboardBase::OnReset()
{
    IndexT i;
    for (i = 0; i < this->keyStates.Size(); i++)
    {
        KeyState& keyState = this->keyStates[i];
        KeyState& nextkeyState = this->nextKeyStates[i];
        if (keyState.pressed)
        {
            keyState.up = true;
            nextkeyState.up = true;
        }
        else
        {
            keyState.up = false;
            nextkeyState.up = false;
        }
        keyState.down = false;
        nextkeyState.down = false;
        keyState.pressed = false;
        nextkeyState.pressed = false;
    }
    this->charInput.Clear();
}

} // База пространства имен
