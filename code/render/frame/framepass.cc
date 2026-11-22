//------------------------------------------------------------------------------
// framepass.cc
// (C) 2016–2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "framepass.h"
#include "coregraphics/graphicsdevice.h"

using namespace CoreGraphics;

namespace Frame
{

//------------------------------------------------------------------------------
/**
*/
FramePass::FramePass()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
FramePass::~FramePass()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
FramePass::Discard()
{
    FrameOp::Discard();

    DestroyPass(this->pass);
    this->pass = InvalidPassId;

    IndexT i;
    for (i = 0; i < this->children.Size(); i++)
        this->children[i]->Discard();
    this->children.Clear();
}

//------------------------------------------------------------------------------
/**
*/
void
FramePass::CompiledImpl::Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex)
{
    N_CMD_SCOPE(cmdBuf, NEBULA_MARKER_GREEN, this->name.Value());

    // начать проход
    CoreGraphics::CmdBeginPass(cmdBuf, this->pass); 
    IndexT bufferedIndex = CoreGraphics::GetBufferedFrameIndex();
    
    // запускать подпроходы
    IndexT i;
    for (i = 0; i < this->subpasses.Size(); i++)
    {
        // перейти к следующему подпроходу, если не на первой итерации
        if (i > 0)
            CoreGraphics::CmdNextSubpass(cmdBuf);

        // выполнить содержимое этого подпрохода и синхронизировать
        // обратите внимание, что мы перегружаем синхронизацию между очередями, поэтому делаем это вне прохода рендеринга
        this->subpasses[i]->QueuePreSync(cmdBuf);
        this->subpasses[i]->Run(cmdBuf, frameIndex, bufferedIndex);
    }

    // конец прохода
    CoreGraphics::CmdEndPass(cmdBuf);
}

//------------------------------------------------------------------------------
/**
*/
void
FramePass::CompiledImpl::Discard()
{
    for (IndexT i = 0; i < this->subpasses.Size(); i++)
    {
        this->subpasses[i]->Discard();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
FramePass::OnWindowResized()
{
    // изменить размер прохода
    PassWindowResizeCallback(this->pass);

    IndexT i;
    for (i = 0; i < this->children.Size(); i++)
    {
        this->children[i]->OnWindowResized();
    }
}

//------------------------------------------------------------------------------
/**
*/
FrameOp::Compiled* 
FramePass::AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator)
{
    CompiledImpl* ret = allocator.Alloc<CompiledImpl>();
    ret->subpasses = {};
    ret->pass = this->pass;
    return ret;
}

//------------------------------------------------------------------------------
/**
*/
void 
FramePass::Build(const BuildContext& ctx)
{
    // if not enable, abort early
    if (!this->enabled)
        return;

    CompiledImpl* myCompiled = (CompiledImpl*)this->AllocCompiled(ctx.allocator);

#if NEBULA_GRAPHICS_DEBUG
    myCompiled->name = this->name;
#endif

    for (IndexT i = 0; i < this->children.Size(); i++)
    {
        // Создайте новый контекст сборки для каждого подпрохода.
        BuildContext newCtx = { myCompiled->pass, (uint)i, ctx.allocator, myCompiled->subpasses, ctx.events, ctx.barriers, ctx.buffers, ctx.textures };
        this->children[i]->Build(newCtx);
    }

    this->compiled = myCompiled;
    this->SetupSynchronization(ctx.allocator, ctx.events, ctx.barriers, ctx.buffers, ctx.textures);

    // Возьмите у детей барьеры, это должно удержать все барьеры после постройки.
    for (Frame::FrameOp::Compiled* child : myCompiled->subpasses)
    {
        myCompiled->barriers.AppendArray(child->barriers);
        child->barriers.Clear();
    }

    ctx.compiledOps.Append(myCompiled);
}

} // пространство имен Frame2
