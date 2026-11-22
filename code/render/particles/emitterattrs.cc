//------------------------------------------------------------------------------
//  emitterattrs.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "particles/emitterattrs.h"

namespace Particles
{

//------------------------------------------------------------------------------
/**
*/
EmitterAttrs::EmitterAttrs()
{
    Memory::Clear(this->floatValues, sizeof(this->floatValues));
    Memory::Clear(&this->intAttributes, sizeof(this->intAttributes));
    Memory::Clear(&this->boolAttributes, sizeof(this->boolAttributes));
    Memory::Clear(this->vec4Values, sizeof(this->vec4Values));
}


} // Частицы пространства имен