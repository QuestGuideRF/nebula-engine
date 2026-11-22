//------------------------------------------------------------------------------
// framesubpasssortedbatch.cc
// (C) 2016–2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "framesubpassorderedbatch.h"

namespace Frame
{

//------------------------------------------------------------------------------
/**
*/
FrameSubpassOrderedBatch::FrameSubpassOrderedBatch()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
FrameSubpassOrderedBatch::~FrameSubpassOrderedBatch()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
FrameOp::Compiled*
FrameSubpassOrderedBatch::AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator)
{
    CompiledImpl* ret = allocator.Alloc<CompiledImpl>();
    ret->batch = this->batch;
    return ret;
}

//------------------------------------------------------------------------------
/**
*/
void
FrameSubpassOrderedBatch::CompiledImpl::Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex)
{

}

} // пространство имен Frame2
