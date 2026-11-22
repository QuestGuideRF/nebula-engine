//------------------------------------------------------------------------------
//  message/dispatcher.cc
//  (C) 2005 г. РадонЛабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "messaging/dispatcher.h"
#include "messaging/port.h"

namespace Messaging
{
__ImplementClass(Messaging::Dispatcher, 'MDIS', Messaging::Port);

//------------------------------------------------------------------------------
/**
*/
Dispatcher::Dispatcher() :
    idPorts(64, 64)
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Присоедините новый порт сообщения.

    @param указатель порта на объект порта сообщения
*/
void
Dispatcher::AttachPort(const Ptr<Port>& port)
{
    n_assert(port);
    n_assert(!this->HasPort(port));

    // добавить в массив портов
    this->portArray.Append(port);

    // получить массив принятых сообщений из порта и добавить каждое сообщение
    // к нашим собственным принятым сообщениям и создать сопоставление идентификаторов сообщений с портами
    const Util::Array<const Id*>& idArray = port->GetAcceptedMessages();
    IndexT i;
    for (i = 0; i < idArray.Size(); i++)
    {
        const Id* msgIdPtr = idArray[i];
        this->RegisterMessage(*msgIdPtr);
        if (!this->idPortMap.Contains(msgIdPtr))
        {
            // нужно добавить новую пустую запись
            Util::Array<Ptr<Port> > emptyArray;
            this->idPorts.Append(emptyArray);
            this->idPortMap.Add(msgIdPtr, this->idPorts.Size() - 1);
        }
        this->idPorts[this->idPortMap[msgIdPtr]].Append(port);
    }
}

//------------------------------------------------------------------------------
/**
    Удалить объект порта сообщения.

    Указатель обработчика @param на объект порта сообщения, который необходимо удалить
*/
void
Dispatcher::RemovePort(const Ptr<Port>& port)
{
    n_assert(0 != port);
    n_assert(this->HasPort(port));

    // удалить порт из карты идентификаторов/портов
    const Util::Array<const Id*>& idArray = port->GetAcceptedMessages();
    IndexT i;
    for (i = 0; i < idArray.Size(); i++)
    {
        const Id* msgIdPtr = idArray[i];
        if (this->idPortMap.Contains(msgIdPtr))
        {            
            Util::Array<Ptr<Port> >& ports = this->idPorts[this->idPortMap[msgIdPtr]];
            IndexT portIndex = ports.FindIndex(port);
            n_assert(InvalidIndex != portIndex);
            ports.EraseIndex(portIndex);
        }        
    }

    // ПРИМЕЧАНИЕ. Удалить сообщение из принятых сообщений невозможно.
    // так это не баг!

    // наконец удалите порт из массива портов
    IndexT index = this->portArray.FindIndex(port);
    n_assert(InvalidIndex != index);
    this->portArray.EraseIndex(index);
}

//------------------------------------------------------------------------------
/**
    Верните true, если порт уже подключен.
*/
bool
Dispatcher::HasPort(const Ptr<Port>& port) const
{
    n_assert(0 != port);
    return (InvalidIndex != this->portArray.FindIndex(port));
}

//------------------------------------------------------------------------------
/**
    Обработать сообщение. Сообщение будет рассылаться только по портам
    которые принимают сообщение.
*/
void
Dispatcher::HandleMessage(const Ptr<Message>& msg)
{
    const Id* msgIdPtr = &(msg->GetId());
    IndexT mapIndex = this->idPortMap.FindIndex(msgIdPtr);
    if (InvalidIndex != mapIndex)
    {
        const Util::Array<Ptr<Port> >& portArray = this->idPorts[this->idPortMap.ValueAtIndex(mapIndex)];
        IndexT portIndex;
        for (portIndex = 0; portIndex < portArray.Size(); portIndex++)
        {
            portArray[portIndex]->HandleMessage(msg);
        }
    }
}

} // пространство имен