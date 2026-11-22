//------------------------------------------------------------------------------
//  @fileentitypool.cc
//  @copyright (C) 2021 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "entitypool.h"
#include "gameserver.h"
#include "basegamefeature/managers/blueprintmanager.h"

namespace Game
{

//------------------------------------------------------------------------------
/**
*/
EntityPool::EntityPool()
{
    this->generations.Reserve(1024);
    this->freeIds.Reserve(2048);
}

//------------------------------------------------------------------------------
/**
*/
bool
EntityPool::Allocate(Entity& e)
{
    memset(&e, 0, sizeof(Entity));
    // убедитесь, что у нас не иссякают поколения слишком быстро
    // всегда освобождая как минимум n объектов перед переработкой
    if (this->freeIds.Size() < 1024)
    {
        this->generations.Append(0);
        e.index = this->generations.Size() - 1;
        n_assert2(e.index < 0x003FFFFF, "переполнение индекса");
        e.generation = 0;
        return true;
    }
    else
    {
        uint32_t const index = this->freeIds.Dequeue();
        e.index = index;
        e.generation = this->generations[index];
        return false;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
EntityPool::Deallocate(Entity e)
{
    n_assert2(this->IsValid(e), "Попытка освободить недействительный/уничтоженный объект!");
    this->freeIds.Enqueue(e.index);
#if NEBULA_DEBUG
    // if you get this warning, you might want to consider reserving more bits for the generation.
    n_warn2(this->generations[e.index] <= 0x3FF, "Переполнение генерации сущности!");
#endif
    this->generations[e.index]++;

}

//------------------------------------------------------------------------------
/**
*/
bool
EntityPool::IsValid(Entity e) const
{
    return e.index < (uint32_t)this->generations.Size() &&
        e.generation == this->generations[e.index];
}

} // пространство имен Игра
