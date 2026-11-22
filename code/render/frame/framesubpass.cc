//------------------------------------------------------------------------------
// framesubpass.cc
// (C) 2016–2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "framesubpass.h"
#include "coregraphics/graphicsdevice.h"

using namespace CoreGraphics;
namespace Frame
{

//------------------------------------------------------------------------------
/**
*/
FrameSubpass::FrameSubpass()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
FrameSubpass::~FrameSubpass()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
FrameSubpass::Discard()
{
    FrameOp::Discard();

    IndexT i;
    for (i = 0; i < this->children.Size(); i++)
    {
        this->children[i]->Discard();
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
FrameSubpass::OnWindowResized()
{

    IndexT i;
    for (i = 0; i < this->children.Size(); i++)
    {
        this->children[i]->OnWindowResized();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
FrameSubpass::CompiledImpl::Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex)
{
    N_CMD_SCOPE(cmdBuf, NEBULA_MARKER_GREEN, this->name.Value());

    // Запуск операций
    IndexT i;
    for (i = 0; i < this->ops.Size(); i++)
    {
        this->ops[i]->Run(cmdBuf, frameIndex, bufferIndex);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
FrameSubpass::CompiledImpl::Discard()
{
    for (IndexT i = 0; i < this->ops.Size(); i++)
        this->ops[i]->Discard();
}

//------------------------------------------------------------------------------
/**
*/
FrameOp::Compiled*
FrameSubpass::AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator)
{
    CompiledImpl* ret = allocator.Alloc<CompiledImpl>();
    ret->ops = {};
    ret->viewports = {};
    ret->scissors = {};
#if NEBULA_GRAPHICS_DEBUG
    ret->name = this->name;
#endif
    // не устанавливайте здесь операции, мы должны сделать это, когда будем строить
    return ret;
}

//------------------------------------------------------------------------------
/**
*/
void 
FrameSubpass::Build(const BuildContext& ctx)
{
    // if not enable, abort early
    if (!this->enabled)
        return;

    CompiledImpl* myCompiled = (CompiledImpl*)this->AllocCompiled(ctx.allocator);

    // Создайте новый контекст с помощью операций этого подграфа.
    BuildContext newCtx = { ctx.currentPass, ctx.subpass, ctx.allocator, myCompiled->ops, ctx.events, ctx.barriers, ctx.buffers, ctx.textures };

    for (IndexT i = 0; i < this->children.Size(); i++)
    {
        this->children[i]->Build(newCtx);
    }

    // Возьмите барьеры у детей
    for (Frame::FrameOp::Compiled* child : myCompiled->ops)
    {
        myCompiled->barriers.AppendArray(child->barriers);
        child->barriers.Clear();
    }
    this->compiled = myCompiled;
    ctx.compiledOps.Append(myCompiled);
}

} // пространство имен Frame2
