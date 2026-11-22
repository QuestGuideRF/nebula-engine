//------------------------------------------------------------------------------
//  gamecontentserverbase.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/base/gamecontentserverbase.h"

namespace Base
{
__ImplementClass(Base::GameContentServerBase, 'GCSB', Core::RefCounted);
__ImplementInterfaceSingleton(Base::GameContentServerBase);

//------------------------------------------------------------------------------
/**
*/
GameContentServerBase::GameContentServerBase() :
    isValid(false)
{
    __ConstructInterfaceSingleton;
}

//------------------------------------------------------------------------------
/**
*/
GameContentServerBase::~GameContentServerBase()
{
    n_assert(!this->IsValid());
    __DestructInterfaceSingleton;
}

//------------------------------------------------------------------------------
/**
*/
void
GameContentServerBase::Setup()
{
    n_assert(!this->IsValid());
    n_assert(this->title.IsValid());
    n_assert(this->titleId.IsValid());
    n_assert(this->version.IsValid());
    this->isValid = true;
}

//------------------------------------------------------------------------------
/**
*/
void
GameContentServerBase::Discard()
{
    n_assert(this->isValid);
    this->isValid = false;
}

} // пространство имен ввода-вывода