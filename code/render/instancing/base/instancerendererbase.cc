//------------------------------------------------------------------------------
//  instancerendererbase.cc
//  (C) 2012 Густав Стербрант
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "instancerendererbase.h"
#include "coregraphics/transformdevice.h"

using namespace CoreGraphics;
using namespace Math;
namespace Base
{
__ImplementClass(Base::InstanceRendererBase, 'INRB', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
InstanceRendererBase::InstanceRendererBase() :
    isOpen(false),
    inBeginUpdate(false),
    shader(0)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
InstanceRendererBase::~InstanceRendererBase()
{
    this->shader = 0;
    this->modelTransforms.Clear();
    this->modelViewTransforms.Clear();
    this->modelViewProjectionTransforms.Clear();
    this->objectIds.Clear();
}

//------------------------------------------------------------------------------
/**
*/
void
InstanceRendererBase::Setup()
{
    n_assert(!this->isOpen);
    this->isOpen = true;
}

//------------------------------------------------------------------------------
/**
*/
void
InstanceRendererBase::Close()
{
    n_assert(this->isOpen);
    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
*/
void 
InstanceRendererBase::BeginUpdate(SizeT amount)
{
    n_assert(!this->inBeginUpdate);
    this->modelTransforms.Clear();
    this->modelViewTransforms.Clear();
    this->modelViewProjectionTransforms.Clear();
    this->objectIds.Clear();
    this->modelTransforms.Reserve(amount);
    this->modelViewTransforms.Reserve(amount);
    this->modelViewProjectionTransforms.Reserve(amount);
    this->objectIds.Reserve(amount);
    this->inBeginUpdate = true;
}

//------------------------------------------------------------------------------
/**
*/
void 
InstanceRendererBase::AddTransform( const matrix44& matrix )
{
    n_assert(this->inBeginUpdate);
    this->modelTransforms.Append(matrix);
}

//------------------------------------------------------------------------------
/**
*/
void 
InstanceRendererBase::AddId( const int id )
{
    n_assert(this->inBeginUpdate);
    this->objectIds.Append(id);
}

//------------------------------------------------------------------------------
/**
    Предполагается, что все преобразования установлены. 
    Вычислите оставшиеся преобразования.
*/
void 
InstanceRendererBase::EndUpdate()
{
    n_assert(this->inBeginUpdate);
    this->inBeginUpdate = false;

    // получить преобразования вида и проекции
    const Ptr<TransformDevice>& transDev = TransformDevice::Instance();
    const matrix44& view = transDev->GetViewTransform();
    const matrix44& viewProj = transDev->GetViewProjTransform();

    // вычислить остаток преобразований
    IndexT i;
    for (i = 0; i < this->modelTransforms.Size(); i++)
    {
        // получить базовое преобразование
        const matrix44& trans = this->modelTransforms[i];

        // добавить преобразования
        this->modelViewTransforms.Append(matrix44::multiply(trans, view));
        this->modelViewProjectionTransforms.Append(matrix44::multiply(trans, viewProj));
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
InstanceRendererBase::Render(const SizeT multiplier)
{
    // переопределить в подклассе
    n_error("Вызывается InstanceRendererBase::Render()!\n");
}


} // пространство имен