//------------------------------------------------------------------------------
//  refcountedlist.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "core/refcountedlist.h"
#include "core/refcounted.h"
#include "core/sysfunc.h"

namespace Core
{
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
void
RefCountedList::DumpLeaks()
{
    if (!this->IsEmpty())
    {
        SysFunc::DebugOut("\n\n\n******** ОБНАРУЖЕНЫ УТЕЧКИ ПЕРЕСЧЕТА ТУМАННОСТИ ********\n\n");
        RefCountedList::Iterator iter;
        for (iter = this->Begin(); iter != this->End(); iter++)
        {
            if (this->refcountedDebugNames.Contains(*iter))
            {
                String msg;
                msg.Format("УТЕЧКА REFCOUNT: объект класса «%s» с идентификатором отладки «%s» по адресу «0x%p», счетчик ссылок равен «%d»\n",
                    (*iter)->GetClassName().AsCharPtr(),
                    this->refcountedDebugNames[*iter].AsCharPtr(),
                    (void*)(*iter),
                    (*iter)->GetRefCount());
                SysFunc::DebugOut(msg.AsCharPtr());
            }
            else
            {
                String msg;
                msg.Format("УТЕЧКА REFCOUNT: объект класса '%s' по адресу '0x%p', счетчик ссылок равен '%d'\n",
                    (*iter)->GetClassName().AsCharPtr(),
                    (void*)(*iter),
                    (*iter)->GetRefCount());
                SysFunc::DebugOut(msg.AsCharPtr());
            }            
        }
        SysFunc::DebugOut("\n******** КОНЕЦ ОТЧЕТА ОБ УТЕЧЕКЕ ТУМАННОСТИ ********\n\n\n");
        this->refcountedDebugNames.Clear();
    }
    else
    {
        SysFunc::DebugOut("\n>>> УРА, УТЕЧКИ ТУМАННОСТИ НЕТ!!!\n\n\n");
    }
}

//------------------------------------------------------------------------------
/**
*/
void
RefCountedList::SetDebugName(RefCounted* ptr, const Util::String& name)
{
    n_assert(!this->refcountedDebugNames.Contains(ptr));
    this->refcountedDebugNames.Add(ptr, name);
}

} // пространство имен Core
