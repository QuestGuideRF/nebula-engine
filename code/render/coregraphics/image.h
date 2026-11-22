#pragma once
#include "ids/id.h"
#include "ids/idallocator.h"
#include "pixelformat.h"
#include "io/uri.h"
namespace CoreGraphics
{
ID_24_8_TYPE(ImageId);
struct ImageCreateInfoFile
{
    IO::URI path;
    bool convertTo32Bit;
};
struct ImageCreateInfoData
{
    CoreGraphics::PixelFormat format;
    SizeT width, height, depth;
    void* data;
};
ImageId CreateImage(const ImageCreateInfoFile& info);
ImageId CreateImage(const ImageCreateInfoData& info);
void DestroyImage(const ImageId id);
struct ImageDimensions
{
    SizeT width, height, depth;
};
ImageDimensions ImageGetDimensions(const ImageId id);
const byte* ImageGetBuffer(const ImageId id);
const byte* ImageGetRedPtr(const ImageId id);
const byte* ImageGetGreenPtr(const ImageId id);
const byte* ImageGetBluePtr(const ImageId id);
const byte* ImageGetAlphaPtr(const ImageId id);
const SizeT ImageGetPixelStride(const ImageId id);
enum ImageChannelPrimitive
{
    Bit8Uint,
    Bit16Uint,
    Bit16Float,
    Bit32Uint,
    Bit32Float
};
ImageChannelPrimitive ImageGetChannelPrimitive(const ImageId id);
enum ImageContainer
{
    PNG,
    JPEG,
    DDS
};
struct ImageLoadInfo
{
    SizeT width, height, depth;
    ImageChannelPrimitive primitive;
    PixelFormat::Code format;
    ImageContainer container;
    uint8_t redOffset, greenOffset, blueOffset, alphaOffset;
    byte* buffer;
};
typedef Ids::IdAllocator<
    ImageLoadInfo
> ImageAllocator;
extern ImageAllocator imageAllocator;
}