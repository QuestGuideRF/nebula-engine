//------------------------------------------------------------------------------
//  attrexithandler.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "attrexithandler.h"
#include "attributedefinitionbase.h"

namespace Attr
{

//------------------------------------------------------------------------------
/**
*/
void
AttrExitHandler::OnExit() const
{
    // очистка динамических атрибутов
    AttributeDefinitionBase::ClearDynamicAttributes();
}

} // Атрибут пространства имен
