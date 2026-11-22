#pragma once
#include "ids/id.h"
#include "util/array.h"
#include "coregraphics/texture.h"
#include "coregraphics/buffer.h"
#include "coregraphics/accelerationstructure.h"
#include "coregraphics/config.h"
#include <tuple>
namespace CoreGraphics
{
ID_24_8_TYPE(BarrierId);
struct CmdBufferId;
struct TextureSubresourceInfo
{
    ImageBits bits;
    uint mip, mipCount, layer, layerCount;
    TextureSubresourceInfo() :
        bits(CoreGraphics::ImageBits::ColorBits),
        mip(0),
        mipCount(1),
        layer(0),
        layerCount(1)
    {}
    TextureSubresourceInfo(CoreGraphics::ImageBits bits, uint mip, uint mipCount, uint layer, uint layerCount) :
        bits(bits),
        mip(mip),
        mipCount(mipCount),
        layer(layer),
        layerCount(layerCount)
    {}
    static TextureSubresourceInfo Texture(CoreGraphics::ImageBits bits, TextureId tex)
    {
        return TextureSubresourceInfo(bits, 0, NEBULA_ALL_MIPS, 0, NEBULA_ALL_LAYERS);
    }
    static TextureSubresourceInfo Color(TextureId tex)
    {
        return TextureSubresourceInfo(ImageBits::ColorBits, 0, NEBULA_ALL_MIPS, 0, NEBULA_ALL_LAYERS);
    }
    static TextureSubresourceInfo DepthStencil(TextureId tex)
    {
        return TextureSubresourceInfo(ImageBits::DepthBits | ImageBits::StencilBits, 0, NEBULA_ALL_MIPS, 0, NEBULA_ALL_LAYERS);
    }
    static TextureSubresourceInfo ColorNoMipNoLayer()
    {
        return TextureSubresourceInfo(ImageBits::ColorBits, 0, 1, 0, 1);
    }
    static TextureSubresourceInfo ColorNoMip(uint layerCount)
    {
        return TextureSubresourceInfo(ImageBits::ColorBits, 0, 1, 0, layerCount);
    }
    static TextureSubresourceInfo ColorNoLayer(uint mipCount)
    {
        return TextureSubresourceInfo(ImageBits::ColorBits, 0, mipCount, 0, 1);
    }
    static TextureSubresourceInfo DepthStencilNoMipNoLayer()
    {
        return TextureSubresourceInfo(ImageBits::DepthBits | ImageBits::StencilBits, 0, 1, 0, 1);
    }
    static TextureSubresourceInfo DepthStencilNoMip(uint layerCount)
    {
        return TextureSubresourceInfo(ImageBits::DepthBits | ImageBits::StencilBits, 0, 1, 0, layerCount);
    }
    static TextureSubresourceInfo DepthStencilNoLayer(uint mipCount)
    {
        return TextureSubresourceInfo(CoreGraphics::ImageBits::DepthBits | CoreGraphics::ImageBits::StencilBits, 0, mipCount, 0, 1);
    }
    const bool Overlaps(const TextureSubresourceInfo& rhs) const
    {
        return ((this->bits & rhs.bits) != 0) && (this->mip <= rhs.mip && this->mip + this->mipCount >= rhs.mip) && (this->layer <= rhs.layer && this->layer + this->layerCount >= rhs.layer);
    }
};
struct BufferSubresourceInfo
{
    uint64_t offset, size;
    BufferSubresourceInfo() :
        offset(0),
        size(NEBULA_WHOLE_BUFFER_SIZE)
    {}
    BufferSubresourceInfo(uint64_t offset, uint64_t size) :
        offset(offset),
        size(size)
    {}
    const bool Overlaps(const BufferSubresourceInfo& rhs) const
    {
        return (this->offset <= rhs.offset && this->offset + this->size >= rhs.offset);
    }
};
struct TextureBarrier
{
    TextureId tex;
    TextureSubresourceInfo subres;
    CoreGraphics::PipelineStage fromStage;
    CoreGraphics::PipelineStage toStage;
};
struct BufferBarrier
{
    BufferId buf;
    CoreGraphics::PipelineStage fromStage;
    CoreGraphics::PipelineStage toStage;
    IndexT offset;
    SizeT size;
};
struct TextureBarrierInfo
{
    TextureId tex;
    TextureSubresourceInfo subres;
};
struct BufferBarrierInfo
{
    BufferId buf;
    BufferSubresourceInfo subres;
};
struct AccelerationStructureBarrierInfo
{
    BlasId blas;
    TlasId tlas;
    enum
    {
        BlasBarrier,
        TlasBarrier,
        None
    } type;
    ~AccelerationStructureBarrierInfo() {};
};
struct BarrierScope
{
    ~BarrierScope()
    {
        n_assert(this->textures.IsEmpty());
        n_assert(this->buffers.IsEmpty());
    }
    void Init(const char* name, CmdBufferId cmdBuf)
    {
        this->name = name;
        this->cmdBuf = cmdBuf;
        this->fromStage = CoreGraphics::PipelineStage::InvalidStage;
        this->toStage = CoreGraphics::PipelineStage::InvalidStage;
    }
    void AddTexture(ImageBits bits, TextureId tex, CoreGraphics::PipelineStage fromStage, CoreGraphics::PipelineStage toStage)
    {
        if (this->fromStage != fromStage || this->toStage != toStage)
        {
            this->Flush();
            this->fromStage = fromStage;
            this->toStage = toStage;
        }
        TextureBarrierInfo info = { .tex = tex, .subres = TextureSubresourceInfo::Texture(bits, tex) };
        textures.Append(info);
    }
    void AddBuffer(BufferId buf, CoreGraphics::PipelineStage fromStage, CoreGraphics::PipelineStage toStage)
    {
        if (this->fromStage != fromStage || this->toStage != toStage)
        {
            this->Flush();
            this->fromStage = fromStage;
            this->toStage = toStage;
        }
        BufferBarrierInfo info = { .buf = buf, .subres = CoreGraphics::BufferSubresourceInfo() };
        buffers.Append(info);
    }
    void Flush()
    {
        if (!this->textures.IsEmpty())
            CoreGraphics::CmdBarrier(this->cmdBuf, this->fromStage, this->toStage, CoreGraphics::BarrierDomain::Global, std::move(this->textures), InvalidIndex, InvalidIndex, this->name);
        if (!this->buffers.IsEmpty())
            CoreGraphics::CmdBarrier(this->cmdBuf, this->fromStage, this->toStage, CoreGraphics::BarrierDomain::Global, std::move(this->buffers), InvalidIndex, InvalidIndex, this->name);
        this->buffers.Clear();
        this->textures.Clear();
    }
    const char* name;
    CmdBufferId cmdBuf;
    CoreGraphics::PipelineStage fromStage, toStage;
    Util::Array<TextureBarrierInfo> textures;
    Util::Array<BufferBarrierInfo> buffers;
private:
};
struct BarrierCreateInfo
{
    Util::StringAtom name;
    BarrierDomain domain;
    CoreGraphics::PipelineStage fromStage;
    CoreGraphics::PipelineStage toStage;
    CoreGraphics::QueueType fromQueue;
    CoreGraphics::QueueType toQueue;
    Util::Array<TextureBarrierInfo> textures;
    Util::Array<BufferBarrierInfo> buffers;
};
BarrierId CreateBarrier(const BarrierCreateInfo& info);
void DestroyBarrier(const BarrierId id);
void BarrierReset(const BarrierId id);
void BarrierPush(
    const CoreGraphics::CmdBufferId buf,
    CoreGraphics::PipelineStage fromStage,
    CoreGraphics::PipelineStage toStage,
    CoreGraphics::BarrierDomain domain,
    const Util::FixedArray<TextureBarrierInfo, true>& textures,
    const Util::FixedArray<BufferBarrierInfo, true>& buffers);
void BarrierPush(
    const CoreGraphics::CmdBufferId buf,
    CoreGraphics::PipelineStage fromStage,
    CoreGraphics::PipelineStage toStage,
    CoreGraphics::BarrierDomain domain,
    const Util::FixedArray<TextureBarrierInfo, true>& textures);
void BarrierPush(
    const CoreGraphics::CmdBufferId buf,
    CoreGraphics::PipelineStage fromStage,
    CoreGraphics::PipelineStage toStage,
    CoreGraphics::BarrierDomain domain,
    const Util::FixedArray<BufferBarrierInfo, true>& buffers);
void BarrierPop(const CoreGraphics::CmdBufferId buf);
void BarrierRepeat(const CoreGraphics::CmdBufferId buf);
inline CoreGraphics::ImageBits
ImageBitsFromString(const Util::String& str)
{
    Util::Array<Util::String> comps = str.Tokenize("|");
    CoreGraphics::ImageBits bits = CoreGraphics::ImageBits(0x0);
    for (IndexT i = 0; i < comps.Size(); i++)
    {
        if (comps[i] == "Auto")             { bits = CoreGraphics::ImageBits::Auto; break; }
        else if (comps[i] == "Color")       bits |= CoreGraphics::ImageBits::ColorBits;
        else if (comps[i] == "Depth")       bits |= CoreGraphics::ImageBits::DepthBits;
        else if (comps[i] == "Stencil")     bits |= CoreGraphics::ImageBits::StencilBits;
        else if (comps[i] == "Metadata")    bits |= CoreGraphics::ImageBits::MetaBits;
        else if (comps[i] == "Plane0")      bits |= CoreGraphics::ImageBits::Plane0Bits;
        else if (comps[i] == "Plane1")      bits |= CoreGraphics::ImageBits::Plane1Bits;
        else if (comps[i] == "Plane2")      bits |= CoreGraphics::ImageBits::Plane2Bits;
        else
        {
            n_error("Неверная строка доступа '%s'\n", comps[i].AsCharPtr());
            return CoreGraphics::ImageBits::ColorBits;
        }
    }
    return bits;
}
inline CoreGraphics::ImageLayout
ImageLayoutFromString(const Util::String& str)
{
    if (str == "Undefined")                 return CoreGraphics::ImageLayout::Undefined;
    else if (str == "General")              return CoreGraphics::ImageLayout::General;
    else if (str == "ColorRenderTexture")   return CoreGraphics::ImageLayout::ColorRenderTexture;
    else if (str == "DepthRenderTexture")   return CoreGraphics::ImageLayout::DepthStencilRenderTexture;
    else if (str == "DepthStencilRead")     return CoreGraphics::ImageLayout::DepthStencilRead;
    else if (str == "ShaderRead")           return CoreGraphics::ImageLayout::ShaderRead;
    else if (str == "TransferSource")       return CoreGraphics::ImageLayout::TransferSource;
    else if (str == "TransferDestination")  return CoreGraphics::ImageLayout::TransferDestination;
    else if (str == "Preinitialized")       return CoreGraphics::ImageLayout::Preinitialized;
    else if (str == "Present")              return CoreGraphics::ImageLayout::Present;
    return CoreGraphics::ImageLayout::Undefined;
}
}