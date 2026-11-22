//------------------------------------------------------------------------------
//  globalstringatomtable.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "util/globalstringatomtable.h"

namespace Util
{
__ImplementInterfaceSingleton(Util::GlobalStringAtomTable);

//------------------------------------------------------------------------------
/**
*/
GlobalStringAtomTable::GlobalStringAtomTable()
{
    __ConstructInterfaceSingleton;
    
    // настроить глобальный строковый буфер
    this->stringBuffer.Setup(NEBULA_GLOBAL_STRINGBUFFER_CHUNKSIZE);
}

//------------------------------------------------------------------------------
/**
*/
GlobalStringAtomTable::~GlobalStringAtomTable()
{
    this->critSect.Enter();
    this->stringBuffer.Discard();
    __DestructInterfaceSingleton;
    this->critSect.Leave();
}

//------------------------------------------------------------------------------
/**
*/
void
GlobalStringAtomTable::Lock()
{
    this->critSect.Enter();
}

//------------------------------------------------------------------------------
/**
*/
void
GlobalStringAtomTable::Unlock()
{
    this->critSect.Leave();
}

//------------------------------------------------------------------------------
/**
    This adds a new string to the atom table and the global string buffer,
    and returns the pointer to the string in the string buffer.

    NOTE: you MUST call this method within Lock()/Unlock()!
*/
const char*
GlobalStringAtomTable::Add(const char* str)
{
    StaticString sstr;
    sstr.ptr = (char*)this->stringBuffer.AddString(str);
    this->table.InsertSorted(sstr);
    return sstr.ptr;
}

//------------------------------------------------------------------------------
/**
    Метод отладки: получить массив со всеми строками в таблице.
*/
GlobalStringAtomTable::DebugInfo
GlobalStringAtomTable::GetDebugInfo() const
{
    this->critSect.Enter();
    DebugInfo debugInfo;
    debugInfo.strings.Reserve(this->table.Size());
    debugInfo.chunkSize = NEBULA_GLOBAL_STRINGBUFFER_CHUNKSIZE;
    debugInfo.numChunks = this->stringBuffer.GetNumChunks();
    debugInfo.allocSize = debugInfo.chunkSize * debugInfo.numChunks;
    debugInfo.usedSize  = 0;
    debugInfo.growthEnabled = NEBULA_ENABLE_GLOBAL_STRINGBUFFER_GROWTH;

    IndexT i;
    for (i = 0; i < table.Size(); i++)
    {        
        const char* str = this->table[i].ptr;
        debugInfo.strings.Append(this->table[i].ptr);
        debugInfo.usedSize += strlen(str) + 1;
    }
    this->critSect.Leave();
    return debugInfo;
}

} // пространство имен Util