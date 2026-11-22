//------------------------------------------------------------------------------
//  localstringatomtable.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "util/localstringatomtable.h"

namespace Util
{
__ImplementSingleton(Util::LocalStringAtomTable);

//------------------------------------------------------------------------------
/**
*/
LocalStringAtomTable::LocalStringAtomTable()
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
LocalStringAtomTable::~LocalStringAtomTable()
{
    __DestructSingleton;
}

//------------------------------------------------------------------------------
/**
    NOTE: the string added here must already be located in the global string
    buffer (see GlobalStringAtomTable for details).
*/
void
LocalStringAtomTable::Add(const char* str)
{
    StaticString sstr;
    sstr.ptr = (char*)str;
    this->table.InsertSorted(sstr);
}

} // пространство имен Util
