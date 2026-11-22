#pragma once
#include "coregraphics/gpubuffertypes.h"
#include "coregraphics/pixelformat.h"
#include "coregraphics/config.h"
#include "coregraphics/window.h"
#include "coregraphics/memory.h"
#include "math/rectangle.h"
#include "ids/idallocator.h"
namespace CoreGraphics
{
struct CmdBufferId;
struct TextureSubresourceInfo;
RESOURCE_ID_TYPE(TextureId);
_DECL_ACQUIRE_RELEASE(TextureId);
enum TextureType
{
    InvalidTextureType,
    Texture1D,
    Texture2D,
    Texture3D,
    TextureCube,
    Texture1DArray,
    Texture2DArray,
    TextureCubeArray,
};
enum TextureCubeFace
{
    PosX = 0,
    NegX,
    PosY,
    NegY,
    PosZ,
    NegZ,
};
enum TextureUsage
{
    InvalidTextureUsage         = 0x0,
    SampleTexture               = 0x1,
    RenderTexture               = 0x2,
    ReadWriteTexture            = 0x4,
    TransferSourceTexture       = 0x8,
    TransferDestinationTexture  = 0x10,
    DeviceExclusive             = 0x20
};
__ImplementEnumBitOperators(CoreGraphics::TextureUsage);
struct TextureDimensions
{
    SizeT width, height, depth;
    bool operator==(const TextureDimensions& dims) const
    {
        return this->width == dims.width && this->height == dims.height && this->depth == dims.depth;
    }
};
struct TextureRelativeDimensions
{
    float width, height, depth;
};
enum class TextureChannelMapping
{
    None,
    Red,
    Green,
    Blue,
    Alpha,
    Zero,
    One
};
struct TextureSwizzle
{
    TextureChannelMapping red, green, blue, alpha;
};
static const ubyte TextureAutoMips = 0xFF;
struct DepthStencilClear
{
    float depth;
    uint stencil;
};
struct TextureCreateInfo
{
    TextureCreateInfo()
        : name(""_atm)
        , usage(CoreGraphics::TextureUsage::SampleTexture)
        , tag(""_atm)
        , data(nullptr)
        , dataSize(0)
        , type(Texture2D)
        , format(CoreGraphics::PixelFormat::R8G8B8A8)
        , width(1)
        , height(1)
        , depth(1)
        , mips(1)
        , minMip(0)
        , layers(1)
        , samples(1)
        , clear(false)
        , clearColorF4{0,0,0,0}
        , windowRelative(false)
        , bindless(true)
        , sparse(false)
        , allowCast(false)
        , alias(CoreGraphics::InvalidTextureId)
        , defaultLayout(CoreGraphics::ImageLayout::ShaderRead)
        , swizzle({ TextureChannelMapping::None, TextureChannelMapping::None, TextureChannelMapping::None, TextureChannelMapping::None })
    {};
    Resources::ResourceName name;
    CoreGraphics::TextureUsage usage;
    Util::StringAtom tag;
    const void* data;
    SizeT dataSize;
    CoreGraphics::TextureType type;
    CoreGraphics::PixelFormat::Code format;
    float width, height, depth;
    uint mips, minMip, layers;
    uint samples;
    bool clear;
    union
    {
        Math::float4 clearColorF4;
        Math::uint4 clearColorU4;
        Math::int4 clearColorI4;
        DepthStencilClear clearDepthStencil;
    };
    bool windowRelative : 1;
    bool bindless : 1;
    bool sparse : 1;
    bool allowCast : 1;
    CoreGraphics::TextureId alias;
    CoreGraphics::ImageLayout defaultLayout;
    CoreGraphics::TextureSwizzle swizzle;
};
struct TextureCreateInfoAdjusted : TextureCreateInfo
{
    float widthScale, heightScale, depthScale;
    CoreGraphics::WindowId window;
};
struct TextureSparsePageSize
{
    uint width, height, depth;
};
struct TextureSparsePageOffset
{
    uint x, y, z;
};
struct TextureSparsePage
{
    TextureSparsePageOffset offset;
    TextureSparsePageSize extent;
    CoreGraphics::Alloc alloc;
};
const TextureId CreateTexture(const TextureCreateInfo& info);
void DestroyTexture(const TextureId id);
Util::StringAtom TextureGetName(const TextureId id);
TextureDimensions TextureGetDimensions(const TextureId id);
TextureRelativeDimensions TextureGetRelativeDimensions(const TextureId id);
CoreGraphics::PixelFormat::Code TextureGetPixelFormat(const TextureId id);
TextureType TextureGetType(const TextureId id);
SizeT TextureGetNumMips(const TextureId id);
SizeT TextureGetNumLayers(const TextureId id);
SizeT TextureGetNumSamples(const TextureId id);
const CoreGraphics::TextureId TextureGetAlias(const TextureId id);
const CoreGraphics::TextureUsage TextureGetUsage(const TextureId id);
const CoreGraphics::ImageLayout TextureGetDefaultLayout(const TextureId id);
uint TextureGetBindlessHandle(const TextureId id);
uint TextureGetStencilBindlessHandle(const TextureId id);
void TextureWindowResized(const TextureId id);
void TextureGenerateMipmaps(const CoreGraphics::CmdBufferId cmdBuf, const TextureId id);
TextureSparsePageSize TextureSparseGetPageSize(const CoreGraphics::TextureId id);
IndexT TextureSparseGetPageIndex(const CoreGraphics::TextureId id, IndexT layer, IndexT mip, IndexT x, IndexT y, IndexT z);
const TextureSparsePage& TextureSparseGetPage(const CoreGraphics::TextureId id, IndexT layer, IndexT mip, IndexT pageIndex);
SizeT TextureSparseGetNumPages(const CoreGraphics::TextureId id, IndexT layer, IndexT mip);
IndexT TextureSparseGetMaxMip(const CoreGraphics::TextureId id);
void TextureSparseEvict(const CoreGraphics::TextureId id, IndexT layer, IndexT mip, IndexT pageIndex);
void TextureSparseMakeResident(const CoreGraphics::TextureId id, IndexT layer, IndexT mip, IndexT pageIndex);
void TextureSparseEvictMip(const CoreGraphics::TextureId id, IndexT layer, IndexT mip);
void TextureSparseMakeMipResident(const CoreGraphics::TextureId id, IndexT layer, IndexT mip);
void TextureSparseCommitChanges(const CoreGraphics::TextureId id);
bool TextureUpdate(const CoreGraphics::CmdBufferId cmd, CoreGraphics::TextureId tex, const SizeT width, SizeT height, SizeT mip, SizeT layer, const void* data, SizeT dataSize);
void TextureClearColor(const CoreGraphics::CmdBufferId cmd, const CoreGraphics::TextureId id, Math::vec4 color, const CoreGraphics::ImageLayout layout, const CoreGraphics::TextureSubresourceInfo& subres);
void TextureClearDepthStencil(const CoreGraphics::CmdBufferId cmd, const CoreGraphics::TextureId id, float depth, uint stencil, const CoreGraphics::ImageLayout layout, const CoreGraphics::TextureSubresourceInfo& subres);
TextureCreateInfoAdjusted TextureGetAdjustedInfo(const TextureCreateInfo& info);
void TextureSetHighestLod(const CoreGraphics::TextureId id, uint lod);
#ifdef WITH_NEBULA_EDITOR
const Util::Array<CoreGraphics::TextureId>& TextureGetCreated();
extern Util::Array<CoreGraphics::TextureId> TrackedTextures;
#endif
inline TextureType
TextureTypeFromString(const Util::String& string)
{
    if      (string == "Texture1D") return Texture1D;
    else if (string == "Texture2D") return Texture2D;
    else if (string == "Texture3D") return Texture3D;
    else if (string == "TextureCube") return TextureCube;
    else if (string == "Texture1DArray") return Texture1DArray;
    else if (string == "Texture2DArray") return Texture2DArray;
    else if (string == "TextureCubeArray") return TextureCubeArray;
    else
    {
        n_error("Неизвестный тип текстуры «%s»", string.AsCharPtr());
        return Texture1D;
    }
}
inline TextureUsage
TextureUsageFromString(const Util::String& string)
{
    Util::Array<Util::String> comps = string.Tokenize("|");
    TextureUsage usage = TextureUsage(0x0);
    for (IndexT i = 0; i < comps.Size(); i++)
    {
        if      (comps[i] == "Sample") usage |= SampleTexture;
        else if (comps[i] == "Render") usage |= RenderTexture;
        else if (comps[i] == "ReadWrite") usage |= ReadWriteTexture;
        else if (comps[i] == "TransferSource") usage |= TransferSourceTexture;
        else if (comps[i] == "TransferDestination") usage |= TransferDestinationTexture;
    }
    return usage;
}
extern TextureId White1D;
extern TextureId Black2D;
extern TextureId White2D;
extern TextureId WhiteCube;
extern TextureId White3D;
extern TextureId White1DArray;
extern TextureId White2DArray;
extern TextureId WhiteCubeArray;
extern TextureId Red2D;
extern TextureId Green2D;
extern TextureId Blue2D;
}