#pragma once
#include "core/types.h"
#include "util/string.h"
#include "io/mediatype.h"
namespace CoreGraphics
{
class ImageFileFormat
{
public:
    enum Code
    {
        BMP,
        JPG,
        PNG,
        DDS,
        TGA,
        InvalidImageFileFormat,
    };
    static Code FromString(const Util::String& str);
    static Util::String ToString(Code c);
    static Code FromMediaType(const IO::MediaType& mediaType);
    static IO::MediaType ToMediaType(Code c);
};
}