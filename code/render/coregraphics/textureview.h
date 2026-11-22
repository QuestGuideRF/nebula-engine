#pragma once
#include "coregraphics/config.h"
#include "coregraphics/texture.h"
namespace CoreGraphics
{
ID_24_8_TYPE(TextureViewId);
struct TextureViewCreateInfo
{
    Util::StringAtom name = "";
    TextureId tex = InvalidTextureId;
    IndexT startMip = 0;
    SizeT numMips = 1;
    IndexT startLayer = 0;
    SizeT numLayers = 1;
    PixelFormat::Code format = PixelFormat::InvalidPixelFormat;
    ImageBits bits = ImageBits::ColorBits;
    CoreGraphics::TextureUsage usage = CoreGraphics::TextureUsage::InvalidTextureUsage;
    CoreGraphics::TextureSwizzle swizzle = { TextureChannelMapping::None, TextureChannelMapping::None, TextureChannelMapping::None, TextureChannelMapping::None };
};
TextureViewId CreateTextureView(const TextureViewCreateInfo& info);
void DestroyTextureView(const TextureViewId id);
void TextureViewReload(const TextureViewId id);
TextureId TextureViewGetTexture(const TextureViewId id);
}