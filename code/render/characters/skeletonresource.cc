//------------------------------------------------------------------------------
//  @fileскелетресурс.cc
//  @copyright (C) 2022 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "skeletonresource.h"
namespace Characters
{

SkeletonResourceAllocator skeletonResourceAllocator;


//------------------------------------------------------------------------------
/**
    Получить скелет из загруженного ресурса
*/
const SkeletonId
SkeletonResourceGetSkeleton(const SkeletonResourceId id, IndexT index)
{
    return skeletonResourceAllocator.Get<0>(id.id)[index];
}

//------------------------------------------------------------------------------
/**
    Откажитесь от ресурса скелета и скелетов, которые он содержит.
*/
void
DestroySkeletonResource(const SkeletonResourceId id)
{
    auto skeletons = skeletonResourceAllocator.Get<0>(id.id);
    for (IndexT i = 0; i < skeletons.Size(); i++)
    {
        DestroySkeleton(skeletons[i]);
    }
    skeletonResourceAllocator.Dealloc(id.id);
}

} // Символы пространства имен
