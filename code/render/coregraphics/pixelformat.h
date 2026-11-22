#pragma once
#include "core/types.h"
#include "util/string.h"
#include "coregraphics/config.h"
namespace CoreGraphics
{
class PixelFormat
{
public:
    enum Code
    {
        R8G8B8X8 = 0,
        R8G8B8,
        R8G8B8A8,
        R5G6B5,
        R5G5B5A1,
        R4G4B4A4,
        DXT1,
        DXT1A,
        DXT3,
        DXT5,
        DXT1sRGB,
        DXT1AsRGB,
        DXT3sRGB,
        DXT5sRGB,
        BC4,
        BC5,
        BC7,
        BC7sRGB,
        R8,
        R16F,
        R16,
        R16G16F,
        R16G16,
        R16G16B16A16F,
        R16G16B16A16,
        R32F,
        R32,
        R32G32F,
        R32G32,
        R32G32B32A32F,
        R32G32B32A32,
        R32G32B32F,
        R32G32B32,
        R11G11B10F,
        R9G9B9E5F,
        SRGBA8,
        R10G10B10X2,
        R10G10B10A2,
        D24X8,
        D24S8,
        D32S8,
        D16S8,
        D32,
        S8,
        B8G8R8,
        B8G8R8A8,
        B5G6R5,
        B5G6R5A1,
        B4G4R4A4,
        NumPixelFormats,
        BC1 = DXT1,
        BC3 = DXT5,
        InvalidPixelFormat = 0xFFFFFFFF,
    };
    static Code FromString(const Util::String& str);
    static Util::String ToString(Code code);
    static uint ToSize(Code code);
    static uint ToChannels(Code code);
    static bool ToCompressed(Code code);
    static SizeT ToTexelSize(Code code);
    static SizeT ToBlockSize(Code code);
    static bool IsDepthFormat(Code code);
    static bool IsStencilFormat(Code code);
    static bool IsDepthStencilFormat(Code code);
    static CoreGraphics::ImageBits ToImageBits(Code code);
};
}