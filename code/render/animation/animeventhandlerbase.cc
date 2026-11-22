//------------------------------------------------------------------------------
//  animeventhandlerbase.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "animation/animeventhandlerbase.h"

namespace Animation
{
__ImplementClass(Animation::AnimEventHandlerBase, 'AEHB', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
AnimEventHandlerBase::AnimEventHandlerBase():
    isOpen(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
AnimEventHandlerBase::~AnimEventHandlerBase()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool
AnimEventHandlerBase::HandleEvent(const Animation::AnimEventInfo& event)
{
    n_assert(this->isOpen);

    // провал: сообщение было обработано
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
AnimEventHandlerBase::OnFrame(Timing::Time time)
{
    // перезаписать в подклассе
}
} // пространство имен Анимация
