//------------------------------------------------------------------------------
//  rendereventhandler.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coregraphics/rendereventhandler.h"

namespace CoreGraphics
{
__ImplementClass(CoreGraphics::RenderEventHandler, 'RDEH', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
RenderEventHandler::RenderEventHandler()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
RenderEventHandler::~RenderEventHandler()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
RenderEventHandler::OnAttach()
{
    // пусто, при необходимости переопределите в подклассе
}

//------------------------------------------------------------------------------
/**
*/
void
RenderEventHandler::OnRemove()
{
    // пусто, при необходимости переопределите в подклассе
}

//------------------------------------------------------------------------------
/**
    This method is called by the RenderDevice when an event happens.
    The default behaviour of this class is to call the HandleEvent() method
    directly. Subclasses of RenderEventHandler may choose to implement
    a different behaviour.
*/
bool
RenderEventHandler::PutEvent(const RenderEvent& e)
{
    return this->HandleEvent(e);
}

//------------------------------------------------------------------------------
/**
    Обработка события рендеринга. Этот метод обычно вызывается PutEvent(), но
    подклассы RenderEventHandler могут реализовать другой
    поведение. Переопределите этот метод в своем подклассе для обработки входящих 
    событие.
*/
bool
RenderEventHandler::HandleEvent(const RenderEvent& e)
{
    // пусто, при необходимости переопределите в подклассе
    return false;
}

} // пространство имен CoreGraphics