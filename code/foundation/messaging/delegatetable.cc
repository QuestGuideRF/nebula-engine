//------------------------------------------------------------------------------
//  delegatetable.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "messaging/delegatetable.h"

namespace Messaging
{
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
void
DelegateTable::AddDelegate(const Id& msgId, const Delegate<void(const Ptr<Message>&)>& del)
{
    IndexT index = this->idIndexMap.FindIndex(&msgId);
    if (InvalidIndex == index)
    {
        // это первый делегат для этого идентификатора сообщения
        Array<Delegate<void(const Ptr<Message>&)> > emptyArray;
        this->delegateArray.Append(emptyArray);
        index = this->delegateArray.Size() - 1;
        this->idIndexMap.Add(&msgId, index);
    }

    // добавить делегата в таблицу поиска
    this->delegateArray[index].Append(del);
}

//------------------------------------------------------------------------------
/**
*/
bool
DelegateTable::Invoke(const Ptr<Message>& msg)
{
    // проверьте, были ли привязаны к сообщению какие-либо делегаты
    IndexT index = this->idIndexMap.FindIndex(&msg->GetId());
    if (InvalidIndex != index)    
    {
        // позвоните делегатам для этого сообщения
        const Array<Delegate<void(const Ptr<Message>&)> > delegates = this->delegateArray[index];
        IndexT delegateIndex;
        for (delegateIndex = 0; delegateIndex < delegates.Size(); delegateIndex++)
        {
            delegates[delegateIndex](msg);
        }
        return true;
    }
    else
    {
        // нет делегатов для этого сообщения
        return false;
    }
}

} // пространство имен
