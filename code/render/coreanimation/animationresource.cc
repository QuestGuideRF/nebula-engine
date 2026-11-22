//------------------------------------------------------------------------------
//  @file анимацияresource.cc
//  @copyright (C) 2022 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "animationresource.h"
namespace CoreAnimation
{

AnimationResourceAllocator animationResourceAllocator;

//------------------------------------------------------------------------------
/**
*/
const AnimationId
AnimationResourceGetAnimation(const AnimationResourceId id, IndexT index)
{
    return animationResourceAllocator.Get<0>(id.id)[index];
}

//------------------------------------------------------------------------------
/**
*/
void
DestroyAnimationResource(const AnimationResourceId id)
{
    auto animations = animationResourceAllocator.Get<0>(id.id);
    for (IndexT i = 0; i < animations.Size(); i++)
    {
        DestroyAnimation(animations[i]);
    }
    animationResourceAllocator.Dealloc(id.id);
}

} // пространство имен CoreAnimation
