//------------------------------------------------------------------------------
// frameop.cc
// (C) 2016–2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "framescript.h"
#include "frameop.h"

namespace Frame
{

//------------------------------------------------------------------------------
/**
*/
FrameOp::FrameOp() :
    domain(CoreGraphics::BarrierDomain::Global),
    queue(CoreGraphics::QueueType::GraphicsQueueType),
    enabled(true)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
FrameOp::~FrameOp()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
FrameOp::Discard()
{
}

//------------------------------------------------------------------------------
/**
*/
void
FrameOp::OnWindowResized()
{
    for (IndexT i = 0; i < this->children.Size(); i++)
        this->children[i]->OnWindowResized();
}

//------------------------------------------------------------------------------
/**
*/
void
FrameOp::Build(const BuildContext& ctx)
{
    // if not enable, abort early
    if (!this->enabled)
        return;

    // создайте скомпилированную версию этой операции, FramePass и FrameSubpass реализуют это иначе, чем обычные операции
    this->compiled = this->AllocCompiled(ctx.allocator);
    ctx.compiledOps.Append(this->compiled);

    this->SetupSynchronization(ctx.allocator, ctx.events, ctx.barriers, ctx.buffers, ctx.textures);
}

//------------------------------------------------------------------------------
/**
    Проанализируйте и при необходимости установите барьеры
*/
void
FrameOp::AnalyzeAndSetupTextureBarriers(
    struct FrameOp::Compiled* op,
    CoreGraphics::TextureId tex,
    const Util::StringAtom& textureName,
    DependencyIntent readOrWrite,
    CoreGraphics::PipelineStage stage,
    CoreGraphics::BarrierDomain domain,
    const CoreGraphics::TextureSubresourceInfo& subres,
    CoreGraphics::QueueType toQueue,
    Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, CoreGraphics::BarrierCreateInfo>& barriers,
    Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, CoreGraphics::EventCreateInfo>& waitEvents,
    Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, struct FrameOp::Compiled*>& signalEvents,
    Util::Array<FrameOp::TextureDependency>& textureDependencies)
{
    Util::Array<CoreGraphics::TextureSubresourceInfo> subresources{ subres };

    // пройти назад по списку зависимостей
    for (IndexT j = textureDependencies.Size() - 1; j >= 0 && subresources.Size() > 0; j--)
    {
        FrameOp::TextureDependency& dep = textureDependencies[j];
        const CoreGraphics::TextureSubresourceInfo& currentSubres = subresources.Front();
        const CoreGraphics::TextureSubresourceInfo& depSubres = dep.subres;

        // check if the dependency touches the same subresource (the framescript guarantees we will have at least one dependency which overlaps)
        if (currentSubres.Overlaps(depSubres))
        {
            // удалить подресурс, который перекрывается
            subresources.EraseFront();

            // Если очереди не совпадают, добавьте новую зависимость к новой очереди.
            if (dep.queue != toQueue)
            {
                // Тодо, нам следует узнать о других материалах и бросить
                // утверждение, если мы не дождемся этого представления здесь
                FrameOp::TextureDependency newDep;
                newDep.stage = stage;
                newDep.intent = readOrWrite;
                newDep.subres = subres;
                newDep.queue = toQueue;
                textureDependencies.Append(newDep);
                continue;
            }

            // if these criteria are met, we need no barrier
            if (dep.intent == DependencyIntent::Read            // предыдущий вызов просто читал
                && readOrWrite == DependencyIntent::Read        // мы просто читаем
                && dep.stage == stage)                          // предыдущий этап был консервативным
            {
                continue;
            }
            else
            {
                // построить пару между операторами
                const Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage> tuple = Util::MakeTuple(stage, dep.stage);
                CoreGraphics::TextureBarrierInfo barrier{ tex, subres };

                CoreGraphics::BarrierCreateInfo& info = barriers.Emplace(tuple);
                info.name = info.name.IsValid() ? info.name.AsString() + " + " + textureName.AsString() : textureName.AsString();
                info.domain = domain;
                info.fromStage = dep.stage;
                info.toStage = stage;
                info.fromQueue = CoreGraphics::QueueType::InvalidQueueType;
                info.toQueue = CoreGraphics::QueueType::InvalidQueueType;
                info.textures.Append(barrier);

                // разделить барьеры, если у нас на mips частично перекрывающийся подресурс
                if (currentSubres.mip < depSubres.mip || currentSubres.mipCount > depSubres.mipCount)
                {
                    uint leftStart = currentSubres.mip;
                    uint leftEnd = leftStart - depSubres.mip;
                    if (leftStart < leftEnd)
                    {
                        CoreGraphics::TextureSubresourceInfo leftRes = currentSubres;
                        leftRes.mipCount = leftEnd - leftStart;
                        subresources.Append(leftRes);
                    }

                    uint rightStart = depSubres.mip + depSubres.mipCount;
                    uint rightEnd = currentSubres.mip + currentSubres.mipCount;
                    if (rightStart < rightEnd)
                    {
                        CoreGraphics::TextureSubresourceInfo rightRes = depSubres;
                        rightRes.mip = rightStart;
                        rightRes.mipCount = rightEnd - rightStart;
                        subresources.Append(rightRes);
                    }
                }

                // разделить барьеры, если у нас есть частично перекрывающиеся подресурсы на слоях
                if (currentSubres.layer < depSubres.layer || currentSubres.layerCount > depSubres.layerCount)
                {
                    uint leftStart = currentSubres.layer;
                    uint leftEnd = leftStart - depSubres.layer;
                    if (leftStart < leftEnd)
                    {
                        CoreGraphics::TextureSubresourceInfo leftRes = currentSubres;
                        leftRes.layerCount = leftEnd - leftStart;
                        subresources.Append(leftRes);
                    }

                    uint rightStart = depSubres.layer + depSubres.layerCount;
                    uint rightEnd = currentSubres.layer + currentSubres.layerCount;
                    if (rightStart < rightEnd)
                    {
                        CoreGraphics::TextureSubresourceInfo rightRes = depSubres;
                        rightRes.layer = rightStart;
                        rightRes.layerCount = rightEnd - rightStart;
                        subresources.Append(rightRes);
                    }
                }

                // добавить новую зависимость в список
                FrameOp::TextureDependency newDep;
                newDep.stage = stage;
                newDep.intent = readOrWrite;
                newDep.subres = subres;
                newDep.queue = toQueue;
                textureDependencies.Append(newDep);
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void
FrameOp::AnalyzeAndSetupBufferBarriers(
    struct FrameOp::Compiled* op,
    CoreGraphics::BufferId buf,
    const Util::StringAtom& bufferName,
    DependencyIntent readOrWrite,
    CoreGraphics::PipelineStage stage,
    CoreGraphics::BarrierDomain domain,
    const CoreGraphics::BufferSubresourceInfo& subres,
    CoreGraphics::QueueType toQueue,
    Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, CoreGraphics::BarrierCreateInfo>& barriers,
    Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, CoreGraphics::EventCreateInfo>& waitEvents,
    Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, FrameOp::Compiled*>& signalEvents,
    Util::Array<FrameOp::BufferDependency>& bufferDependencies)
{
    Util::Array<CoreGraphics::BufferSubresourceInfo> subresources{ subres };

    // левый набор зависимостей
    for (IndexT j = bufferDependencies.Size() - 1; j >= 0 && subresources.Size() > 0; j--)
    {
        FrameOp::BufferDependency& dep = bufferDependencies[j];
        const CoreGraphics::BufferSubresourceInfo& currentSubres = subresources.Front();
        const CoreGraphics::BufferSubresourceInfo& depSubres = dep.subres;

        // check if the dependency touches the same subresource (the framescript guarnatees we will have at least one dependency which overlaps)
        if (currentSubres.Overlaps(depSubres))
        {
            subresources.EraseFront();

            // Если очереди не совпадают, добавьте новую зависимость к новой очереди.
            if (dep.queue != toQueue)
            {
                // Тодо, нам следует узнать о других материалах и бросить
                // утверждение, если мы не дождемся этого представления здесь
                FrameOp::BufferDependency newDep;
                newDep.stage = stage;
                newDep.intent = readOrWrite;
                newDep.subres = subres;
                newDep.queue = toQueue;
                bufferDependencies.Append(newDep);
                continue;
            }

            // if these criteria are met, we need no barrier
            if (dep.intent == DependencyIntent::Read
                && readOrWrite == DependencyIntent::Read
                && dep.stage == stage)                       // проверьте, был ли предыдущий этап достаточно консервативным
            {
                continue;
            }
            else
            {
                // построить пару между операторами
                const Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage> tuple = Util::MakeTuple(stage, dep.stage);
                CoreGraphics::BufferBarrierInfo barrier{ buf, { subres.offset, subres.size } };

                CoreGraphics::BarrierCreateInfo& info = barriers.Emplace(tuple);
                info.name = info.name.IsValid() ? info.name.AsString() + " + " + bufferName.AsString() : bufferName.AsString();
                info.domain = domain;
                info.fromStage = dep.stage;
                info.toStage = stage;
                info.fromQueue = CoreGraphics::QueueType::InvalidQueueType;
                info.toQueue = CoreGraphics::QueueType::InvalidQueueType;
                info.buffers.Append(barrier);
            }

            // разделить барьеры, если у нас есть частично перекрывающиеся подресурсы на слоях
            if (currentSubres.offset < depSubres.offset || currentSubres.size > depSubres.size)
            {
                uint leftStart = currentSubres.offset;
                uint leftEnd = leftStart - depSubres.offset;
                if (leftStart < leftEnd)
                {
                    CoreGraphics::BufferSubresourceInfo leftRes = currentSubres;
                    leftRes.size = leftEnd - leftStart;
                    subresources.Append(leftRes);
                }

                uint rightStart = depSubres.offset + depSubres.size;
                uint rightEnd = currentSubres.offset + currentSubres.size;
                if (rightStart < rightEnd)
                {
                    CoreGraphics::BufferSubresourceInfo rightRes = depSubres;
                    rightRes.size = rightEnd - rightStart;
                    subresources.Append(rightRes);
                }
            }

            // добавить новую зависимость буфера
            FrameOp::BufferDependency newDep;
            newDep.stage = stage;
            newDep.intent = readOrWrite;
            newDep.subres = subres;
            newDep.queue = toQueue;
            bufferDependencies.Append(newDep);
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void
FrameOp::SetupSynchronization(
    Memory::ArenaAllocator<BIG_CHUNK>& allocator,
    Util::Array<CoreGraphics::EventId>& events,
    Util::Array<CoreGraphics::BarrierId>& barriers,
    Util::Dictionary<CoreGraphics::BufferId, Util::Array<BufferDependency>>& buffers,
    Util::Dictionary<CoreGraphics::TextureId, Util::Array<TextureDependency>>& textures)
{
    n_assert(this->compiled != nullptr);
    IndexT i;

    if (!this->textureDeps.IsEmpty() || !this->bufferDeps.IsEmpty())
    {
        Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, CoreGraphics::EventCreateInfo> waitEvents;
        Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, CoreGraphics::BarrierCreateInfo> barriers;
        Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, FrameOp::Compiled*> signalEvents;

        Util::Dictionary<CoreGraphics::TextureId, Util::Tuple<Util::StringAtom, CoreGraphics::PipelineStage, CoreGraphics::TextureSubresourceInfo>> texDeps = this->textureDeps;
        Util::Dictionary<CoreGraphics::BufferId, Util::Tuple<Util::StringAtom, CoreGraphics::PipelineStage, CoreGraphics::BufferSubresourceInfo>> bufDeps = this->bufferDeps;

        // Просмотрите все ссылки на текстуры и буферы, чтобы установить зависимости.
        for (i = 0; i < this->textureDepRefs.Size(); i++)
            texDeps.Add(*this->textureDepRefs.KeyAtIndex(i), this->textureDepRefs.ValueAtIndex(i));
        for (i = 0; i < this->bufferDepRefs.Size(); i++)
            bufDeps.Add(*this->bufferDepRefs.KeyAtIndex(i), this->bufferDepRefs.ValueAtIndex(i));

        // просмотреть зависимости текстур
        for (i = 0; i < texDeps.Size(); i++)
        {
            const CoreGraphics::TextureId& tex = texDeps.KeyAtIndex(i);
            const CoreGraphics::TextureId& alias = TextureGetAlias(tex);

            // правильный набор зависимостей
            const Util::StringAtom& name = Util::Get<0>(texDeps.ValueAtIndex(i));
            const CoreGraphics::PipelineStage& stage = Util::Get<1>(texDeps.ValueAtIndex(i));
            const CoreGraphics::TextureSubresourceInfo& subres = Util::Get<2>(texDeps.ValueAtIndex(i));

            DependencyIntent readOrWrite = DependencyIntent::Read;
            switch (stage)
            {
                case CoreGraphics::PipelineStage::VertexShaderWrite:
                case CoreGraphics::PipelineStage::HullShaderWrite:
                case CoreGraphics::PipelineStage::DomainShaderWrite:
                case CoreGraphics::PipelineStage::GeometryShaderWrite:
                case CoreGraphics::PipelineStage::PixelShaderWrite:
                case CoreGraphics::PipelineStage::ComputeShaderWrite:
                case CoreGraphics::PipelineStage::AllShadersWrite:
                case CoreGraphics::PipelineStage::ColorWrite:
                case CoreGraphics::PipelineStage::DepthStencilWrite:
                case CoreGraphics::PipelineStage::HostWrite:
                case CoreGraphics::PipelineStage::MemoryWrite:
                case CoreGraphics::PipelineStage::TransferWrite:
                    readOrWrite = DependencyIntent::Write;
                    break;
                default: break;
            }

            // зависимости на данный момент от текстуры
            IndexT idx = textures.FindIndex(tex);

            // Если это первое появление этой текстуры, добавьте неявную зависимость чтения шейдера.
            if (idx == InvalidIndex)
            {
                CoreGraphics::PipelineStage stage = this->queue == CoreGraphics::QueueType::ComputeQueueType ? CoreGraphics::PipelineStage::ComputeShaderRead : CoreGraphics::PipelineStage::AllShadersRead;
                idx = textures.Add(tex, { FrameOp::TextureDependency{ stage, DependencyIntent::Read, subres } });
            }
            Util::Array<TextureDependency>& deps = textures.ValueAtIndex(idx);

            // проанализировать, требуется ли синхронизация, и настроить соответствующие барьеры и/или события
            AnalyzeAndSetupTextureBarriers(
                this->compiled, tex, name, readOrWrite, stage, this->domain, subres, this->queue, barriers, waitEvents, signalEvents, deps);

            // if alias, also make sure to visit the alias
            if (alias != CoreGraphics::InvalidTextureId)
            {
                IndexT idx = textures.FindIndex(alias);
                Util::Array<TextureDependency>& deps = textures.ValueAtIndex(idx);
                AnalyzeAndSetupTextureBarriers(
                    this->compiled, alias, name, readOrWrite, stage, this->domain, subres, this->queue, barriers, waitEvents, signalEvents, deps);
            }

        }

        // пройти через зависимости буфера
        for (i = 0; i < bufDeps.Size(); i++)
        {
            const CoreGraphics::BufferId& buf = bufDeps.KeyAtIndex(i);

            // правильный набор зависимостей
            const Util::StringAtom& name = Util::Get<0>(bufDeps.ValueAtIndex(i));
            const CoreGraphics::PipelineStage& stage = Util::Get<1>(bufDeps.ValueAtIndex(i));
            const CoreGraphics::BufferSubresourceInfo& subres = Util::Get<2>(bufDeps.ValueAtIndex(i));

            DependencyIntent readOrWrite = DependencyIntent::Read;
            switch (stage)
            {
                case CoreGraphics::PipelineStage::VertexShaderWrite:
                case CoreGraphics::PipelineStage::HullShaderWrite:
                case CoreGraphics::PipelineStage::DomainShaderWrite:
                case CoreGraphics::PipelineStage::GeometryShaderWrite:
                case CoreGraphics::PipelineStage::PixelShaderWrite:
                case CoreGraphics::PipelineStage::ComputeShaderWrite:
                case CoreGraphics::PipelineStage::ColorWrite:
                case CoreGraphics::PipelineStage::DepthStencilWrite:
                case CoreGraphics::PipelineStage::HostWrite:
                case CoreGraphics::PipelineStage::MemoryWrite:
                case CoreGraphics::PipelineStage::TransferWrite:
                    readOrWrite = DependencyIntent::Write;
                    break;
                default: break;
            }

            // зависимости на данный момент от текстуры
            IndexT idx = buffers.FindIndex(buf);
            if (idx == InvalidIndex)
            {
                CoreGraphics::PipelineStage stage = this->queue == CoreGraphics::QueueType::ComputeQueueType ? CoreGraphics::PipelineStage::ComputeShaderRead : CoreGraphics::PipelineStage::AllShadersRead;
                idx = buffers.Add(buf, { BufferDependency{ stage, DependencyIntent::Read } });
            }
            Util::Array<BufferDependency>& deps = buffers.ValueAtIndex(idx);

            AnalyzeAndSetupBufferBarriers(
                this->compiled, buf, name, readOrWrite, stage, this->domain, subres, this->queue, barriers, waitEvents, signalEvents, deps);
        }

#pragma push_macro("CreateEvent")
#undef CreateEvent

        // allocate inputs, which is what we wait for or if we immediately trigger a barrier _before_ we execute the command
        this->compiled->barriers.Resize(barriers.Size());

        for (i = 0; i < barriers.Size(); i++)
        {
            barriers.ValueAtIndex(i).name = barriers.ValueAtIndex(i).name.AsString() + " <Рамочный барьер>";
            CoreGraphics::BarrierId bar = CreateBarrier(barriers.ValueAtIndex(i));
            this->compiled->barriers[i] = bar;
        }

#pragma pop_macro("CreateEvent")
    }
}

//------------------------------------------------------------------------------
/**
*/
void
FrameOp::Compiled::Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex)
{
    n_assert(cmdBuf != CoreGraphics::InvalidCmdBufferId);
}

//------------------------------------------------------------------------------
/**
*/
void
FrameOp::Compiled::Discard()
{
    for (IndexT i = 0; i < this->barriers.Size(); i++)
        CoreGraphics::DestroyBarrier(this->barriers[i]);
    this->barriers.Clear();
}

//------------------------------------------------------------------------------
/**
*/
void
FrameOp::Compiled::SetupConstants(const IndexT bufferIndex)
{
    // Ничего не делайте, скрипт отвечает за настройку констант при необходимости
}

//------------------------------------------------------------------------------
/**
*/
void
FrameOp::Compiled::QueuePreSync(const CoreGraphics::CmdBufferId cmdBuf)
{
    for (const CoreGraphics::BarrierId barrier : this->barriers)
    {
        // Это действительно уродливый способ борьбы с буферами подкачки, которые меняют изображение ВК внутри каждого кадра.
        CoreGraphics::BarrierReset(barrier);
        CoreGraphics::CmdBarrier(cmdBuf, barrier);
    }
}

} // пространство имен Frame2
