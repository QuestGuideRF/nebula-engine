//------------------------------------------------------------------------------
//  stringatom.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "util/stringatom.h"
#include "util/localstringatomtable.h"
#include "util/globalstringatomtable.h"

#include <string.h>

//------------------------------------------------------------------------------
/**
    Строка формы буквального конструктора, для использования "foobar"_atm автоматически создаст StringAtom.
*/
Util::StringAtom
operator ""_atm(const char* c, std::size_t s)
{
    return Util::StringAtom(c);
}

namespace Util
{

//------------------------------------------------------------------------------
/**
*/
void
StringAtom::Setup(const char* str)
{
    #if NEBULA_ENABLE_THREADLOCAL_STRINGATOM_TABLES
        // сначала проверьте нашу локальную таблицу атомов строк, является ли строка
        // там уже зарегистрирован, для этого не требуется никакого треда
        // synchronisation
        LocalStringAtomTable* localTable = LocalStringAtomTable::Instance();
        this->content = localTable->Find(str);
        if (0 != this->content)
        {
            // да, строка в локальной таблице, мы закончили
            return;
        }
    #endif

    // the string wasn't in the local table (or thread-local tables are disabled), 
    // поэтому нам нужно проверить глобальную таблицу, это включает в себя синхронизацию потоков
    GlobalStringAtomTable* globalTable = GlobalStringAtomTable::Instance();
    globalTable->Lock();
    this->content = globalTable->Find(str);
    if (0 == this->content)
    {
        // hrmpf, строки еще нет в глобальной таблице, поэтому добавьте ее...
        this->content = globalTable->Add(str);
    }
    globalTable->Unlock();

    #if NEBULA_ENABLE_THREADLOCAL_STRINGATOM_TABLES
        // наконец, добавьте новую строку и в нашу локальную таблицу, чтобы
        // следующий поиск этой строки в нашем потоке будет быстрее
        localTable->Add(this->content);
    #endif
}

//------------------------------------------------------------------------------
/**
    Сравните с необработанной строкой. Осторожно, медленно!
*/
bool
StringAtom::operator==(const char* rhs) const
{
    if (nullptr == this->content)
    {
        return false;
    }
    else
    {
        return (0 == strcmp(this->content, rhs));
    }
}

//------------------------------------------------------------------------------
/**
*/
bool 
StringAtom::operator==(std::nullptr_t) const
{
    return this->content == nullptr;
}

//------------------------------------------------------------------------------
/**
    Сравните с необработанной строкой. Осторожно, медленно!
*/
bool
StringAtom::operator!=(const char* rhs) const
{
    if (nullptr == this->content)
    {
        return false;
    }
    else
    {
        return (0 == strcmp(this->content, rhs));
    }
}

//------------------------------------------------------------------------------
/**
*/
uint32_t
StringAtom::StringHashCode() const
{
    uint32_t hash = 0;
    const char* ptr = this->content;
    size_t len = strlen(ptr);
    uint32_t i;
    for (i = 0; i < len; i++)
    {
        hash += ptr[i];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

//------------------------------------------------------------------------------
/**
*/
Util::StringAtom
operator""_atm(const char* c)
{
    return Util::StringAtom(c);
}

} // пространство имен Util
