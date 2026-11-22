//------------------------------------------------------------------------------
//  displayeventhandler.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coregraphics/displayeventhandler.h"

namespace CoreGraphics
{
__ImplementClass(CoreGraphics::DisplayEventHandler, 'DSEH', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
DisplayEventHandler::DisplayEventHandler()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
DisplayEventHandler::~DisplayEventHandler()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
DisplayEventHandler::OnAttach()
{
    // пусто, при необходимости переопределите в подклассе
}

//------------------------------------------------------------------------------
/**
*/
void
DisplayEventHandler::OnRemove()
{
    // пусто, при необходимости переопределите в подклассе
}

//------------------------------------------------------------------------------
/**
    This method is called by the DisplayDevice when an event happens.
    The default behaviour of this class is to call the HandleEvent() method
    directly. Subclasses of DisplayEventHandler may choose to implement
    a different behaviour.
*/
bool
DisplayEventHandler::PutEvent(const DisplayEvent& e)
{
    return this->HandleEvent(e);
}

//------------------------------------------------------------------------------
/**
    Обработка события отображения. Этот метод обычно вызывается PutEvent(), но
    подклассы DisplayEventHandler могут реализовать другой
    поведение. Переопределите этот метод в своем подклассе для обработки входящих 
    событие.
*/
bool
DisplayEventHandler::HandleEvent(const DisplayEvent& e)
{
    // пусто, при необходимости переопределите в подклассе
    return false;
}

} // пространство имен CoreGraphics

