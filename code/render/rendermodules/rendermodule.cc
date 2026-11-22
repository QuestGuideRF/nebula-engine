//------------------------------------------------------------------------------
//  rendermodule.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "rendermodules/rendermodule.h"
#include "graphics/graphicsserver.h"

namespace RenderModules
{
__ImplementClass(RenderModules::RenderModule, 'RMDL', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
RenderModule::RenderModule() :
    isValid(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
RenderModule::~RenderModule()
{
    n_assert(!this->IsValid());
}

//------------------------------------------------------------------------------
/**
*/
void
RenderModule::Setup()
{
    n_assert(!this->IsValid());
    this->isValid = true;

    // register with the graphics server (for per-frame callbacks)
    Graphics::GraphicsServer::Instance()->RegisterRenderModule(this);
}

//------------------------------------------------------------------------------
/**
*/
void
RenderModule::Discard()
{
    n_assert(this->IsValid());
    this->isValid = false;

    // отменить регистрацию на графическом сервере
    Graphics::GraphicsServer::Instance()->UnregisterRenderModule(this);
}

//------------------------------------------------------------------------------
/**
*/
void
RenderModule::OnFrame()
{
    // пусто, при необходимости переопределить в подклассе
}
    
} // пространство имен Графика
