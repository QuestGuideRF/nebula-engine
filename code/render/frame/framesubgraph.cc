//------------------------------------------------------------------------------
//  @fileframesubgraph.cc
//  @copyright (C) 2021 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "framesubgraph.h"
namespace Frame
{

Util::Dictionary<Util::StringAtom, Util::Array<FrameOp*>> nameToSubgraph;
//------------------------------------------------------------------------------
/**
*/
FrameSubgraph::FrameSubgraph()
{
}

//------------------------------------------------------------------------------
/**
*/
FrameSubgraph::~FrameSubgraph()
{
}

//------------------------------------------------------------------------------
/**
*/
void
FrameSubgraph::OnWindowResized()
{
    // Просто распространите на узлы графа
    Util::Array<Frame::FrameOp*> ops = Frame::GetSubgraph(this->name);
    for (Frame::FrameOp* op : ops)
        op->OnWindowResized();
}

//------------------------------------------------------------------------------
/**
*/
void
FrameSubgraph::CompiledImpl::Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex)
{
    N_CMD_SCOPE(cmdBuf, NEBULA_MARKER_ORANGE, this->name.Value());

    for (Frame::FrameOp::Compiled* op : this->subgraphOps)
    {
        op->QueuePreSync(cmdBuf);
        op->Run(cmdBuf, frameIndex, bufferIndex);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
FrameSubgraph::CompiledImpl::Discard()
{
    FrameOp::Compiled::Discard();
    for (Frame::FrameOp::Compiled* op : this->subgraphOps)
    {
        op->Discard();
    }
}

//------------------------------------------------------------------------------
/**
*/
FrameOp::Compiled*
FrameSubgraph::AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator)
{
    CompiledImpl* ret = allocator.Alloc<CompiledImpl>();

#if NEBULA_GRAPHICS_DEBUG
    ret->name = this->name;
#endif
    return ret;
}

//------------------------------------------------------------------------------
/**
*/
void
FrameSubgraph::Build(const BuildContext& ctx)
{
    // Если не включить, прервать досрочно
    if (!this->enabled)
        return;

    Util::Array<Frame::FrameOp*> ops = Frame::GetSubgraph(this->name);
    if (ops.IsEmpty())
        return;

    CompiledImpl* myCompiled = (CompiledImpl*)this->AllocCompiled(ctx.allocator);

    // Просто извлеките подграф и постройте его так, как если бы он всегда был частью сценария фрейма.
    BuildContext newCtx = { ctx.currentPass, ctx.subpass, ctx.allocator, myCompiled->subgraphOps, ctx.events, ctx.barriers, ctx.buffers, ctx.textures };
    for (Frame::FrameOp* op : ops)
    {
        op->Build(newCtx);
    }

    // Возьмите барьеры у детей
    if (this->domain == CoreGraphics::BarrierDomain::Pass)
    {
        for (Frame::FrameOp::Compiled* child : myCompiled->subgraphOps)
        {
            myCompiled->barriers.AppendArray(child->barriers);
            child->barriers.Clear();
        }
    }

    this->compiled = myCompiled;
    ctx.compiledOps.Append(myCompiled);
}

//------------------------------------------------------------------------------
/**
*/
void
AddSubgraph(const Util::StringAtom name, const Util::Array<FrameOp*> ops)
{
    n_assert(!nameToSubgraph.Contains(name));
    nameToSubgraph.Add(name, ops);
}

//------------------------------------------------------------------------------
/**
*/
const Util::Array<FrameOp*>
GetSubgraph(const Util::StringAtom name)
{
    if (nameToSubgraph.Contains(name))
        return nameToSubgraph[name];
    else
    {
        n_printf("[FrameScript] — подграф «%s» не найден\n", name.Value());
        return {};
    }
}

} // пространство имен Фрейм
