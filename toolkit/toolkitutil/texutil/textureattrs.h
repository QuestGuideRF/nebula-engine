#pragma once
#include "util/string.h"
#include "io/filetime.h"
namespace ToolkitUtil
{
class TextureAttrs
{
public:
    enum PixelFormat
    {
        DXT1C,
        DXT1A,
        DXT3,
        DXT5,
        DXT5NM,
        U1555,
        U4444,
        U565,
        U8888,
        U888,
        BC4,
        BC5,
        BC6H,
        BC7,
        R8,
        R16F,
        R16G16F,
        R16,
        R16G16,
        DXGI,
    };
    enum Filter
    {
        Point,
        Box,
        Triangle,
        Quadrat,
        Cubic,
        Kaiser,
    };
    enum Quality
    {
        Low,
        Normal,
        High,
    };
    enum ColorSpace
    {
        Linear,
        sRGB
    };
    TextureAttrs();
    void SetMaxWidth(SizeT w);
    SizeT GetMaxWidth() const;
    void SetMaxHeight(SizeT h);
    SizeT GetMaxHeight() const;
    void SetGenMipMaps(bool b);
    bool GetGenMipMaps() const;
    void SetPixelFormat(PixelFormat p);
    PixelFormat GetPixelFormat() const;
    void SetMipMapFilter(Filter f);
    Filter GetMipMapFilter() const;
    void SetScaleFilter(Filter f);
    Filter GetScaleFilter() const;
    void SetQuality(Quality q);
    Quality GetQuality() const;
    void SetColorSpace(ColorSpace c);
    ColorSpace GetColorSpace() const;
    void SetTime(const IO::FileTime & ft);
    const IO::FileTime & GetTime() const;
    const bool GetFlipNormalY() const;
    void SetFlipNormalY(bool const val);
    void SetDxgi(const Util::String& str);
    Util::String GetDxgi() const;
    static Util::String PixelFormatToString(PixelFormat p);
    static PixelFormat StringToPixelFormat(const Util::String& str);
    static Util::String FilterToString(Filter f);
    static Filter StringToFilter(const Util::String& str);
    static Util::String QualityToString(Quality q);
    static Quality StringToQuality(const Util::String& str);
    static Util::String ColorSpaceToString(ColorSpace c);
    static ColorSpace StringToColorSpace(const Util::String& str);
private:
    SizeT maxWidth;
    SizeT maxHeight;
    bool genMipMaps;
    PixelFormat pixelFormat;
    Filter mipMapFilter;
    Filter scaleFilter;
    Quality quality;
    ColorSpace colorSpace;
    IO::FileTime attrTime;
    bool flipNormalY;
    Util::String dxgiRaw;
};
inline void
TextureAttrs::SetMaxWidth(SizeT w)
{
    this->maxWidth = w;
}
inline SizeT
TextureAttrs::GetMaxWidth() const
{
    return this->maxWidth;
}
inline void
TextureAttrs::SetMaxHeight(SizeT h)
{
    this->maxHeight = h;
}
inline SizeT
TextureAttrs::GetMaxHeight() const
{
    return this->maxHeight;
}
inline void
TextureAttrs::SetGenMipMaps(bool b)
{
    this->genMipMaps = b;
}
inline bool
TextureAttrs::GetGenMipMaps() const
{
    return this->genMipMaps;
}
inline void
TextureAttrs::SetPixelFormat(PixelFormat p)
{
    this->pixelFormat = p;
}
inline TextureAttrs::PixelFormat
TextureAttrs::GetPixelFormat() const
{
    return this->pixelFormat;
}
inline void
TextureAttrs::SetMipMapFilter(Filter f)
{
    this->mipMapFilter = f;
}
inline TextureAttrs::Filter
TextureAttrs::GetMipMapFilter() const
{
    return this->mipMapFilter;
}
inline void
TextureAttrs::SetScaleFilter(Filter f)
{
    this->scaleFilter = f;
}
inline TextureAttrs::Filter
TextureAttrs::GetScaleFilter() const
{
    return this->scaleFilter;
}
inline void
TextureAttrs::SetQuality(Quality q)
{
    this->quality = q;
}
inline TextureAttrs::Quality
TextureAttrs::GetQuality() const
{
    return this->quality;
}
inline void
TextureAttrs::SetColorSpace(ColorSpace c)
{
    this->colorSpace = c;
}
inline TextureAttrs::ColorSpace
TextureAttrs::GetColorSpace() const
{
    return this->colorSpace;
}
inline const IO::FileTime &
TextureAttrs::GetTime() const
{
    return this->attrTime;
}
inline const bool
TextureAttrs::GetFlipNormalY() const
{
    return this->flipNormalY;
}
inline void
TextureAttrs::SetFlipNormalY(bool const val)
{
    this->flipNormalY = val;
}
inline void
TextureAttrs::SetTime(const IO::FileTime & t)
{
    this->attrTime = t;
}
inline void
TextureAttrs::SetDxgi(const Util::String& str)
{
    this->dxgiRaw = str;
}
inline Util::String
TextureAttrs::GetDxgi() const
{
    return this->dxgiRaw;
}
}