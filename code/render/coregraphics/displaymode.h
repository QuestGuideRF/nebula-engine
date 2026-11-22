#pragma once
#include "core/types.h"
#include "coregraphics/pixelformat.h"
namespace CoreGraphics
{
class DisplayMode
{
public:
    DisplayMode();
    DisplayMode(uint x, uint y, SizeT w, SizeT h);
    DisplayMode(SizeT w, SizeT h, PixelFormat::Code p);
    DisplayMode(uint x, uint y, SizeT w, SizeT h, PixelFormat::Code p);
    bool operator==(const DisplayMode& rhs) const;
    bool operator!=(const DisplayMode& rhs) const;
    void SetXPos(uint x);
    uint GetXPos() const;
    void SetYPos(uint y);
    uint GetYPos() const;
    void SetWidth(SizeT w);
    SizeT GetWidth() const;
    void SetHeight(SizeT h);
    SizeT GetHeight() const;
    void SetPixelFormat(PixelFormat::Code p);
    PixelFormat::Code GetPixelFormat() const;
    void SetAspectRatio(float a);
    float GetAspectRatio() const;
    void SetRefreshRate( uint refreshRate );
    uint GetRefreshRate() const;
    void SetContentScale(float s);
    float GetContentScale() const;
private:
    uint xPos;
    uint yPos;
    SizeT width;
    SizeT height;
    uint refreshRate;
    float aspectRatio;
    float contentScale;
    PixelFormat::Code pixelFormat;
};
inline
DisplayMode::DisplayMode() :
    xPos(0),
    yPos(0),
    width(1024),
    height(768),
    refreshRate(60),
    aspectRatio(4.0f / 3.0f),
    contentScale(1.0f),
    pixelFormat(PixelFormat::SRGBA8)
{
}
inline
DisplayMode::DisplayMode(uint x, uint y, SizeT w, SizeT h) :
    xPos(x),
    yPos(y),
    width(w),
    height(h),
    refreshRate(60),
    aspectRatio(float(w) / float(h)),
    contentScale(1.0f),
    pixelFormat(PixelFormat::SRGBA8)
{
}
inline
DisplayMode::DisplayMode(SizeT w, SizeT h, PixelFormat::Code p) :
    xPos(0),
    yPos(0),
    width(w),
    height(h),
    refreshRate(60),
    aspectRatio(float(w) / float(h)),
    contentScale(1.0f),
    pixelFormat(p)
{
}
inline
DisplayMode::DisplayMode(uint x, uint y, SizeT w, SizeT h, PixelFormat::Code p) :
    xPos(x),
    yPos(y),
    width(w),
    height(h),
    refreshRate(60),
    aspectRatio(float(w) / float(h)),
    contentScale(1.0f),
    pixelFormat(p)
{
}
inline bool
DisplayMode::operator==(const DisplayMode& rhs) const
{
    return ((this->xPos == rhs.xPos) &&
            (this->yPos == rhs.yPos) &&
            (this->width == rhs.width) &&
            (this->height == rhs.height) &&
            (this->refreshRate == rhs.refreshRate) &&
            (this->aspectRatio == rhs.aspectRatio) &&
            (this->contentScale == rhs.contentScale) &&
            (this->pixelFormat == rhs.pixelFormat));
}
inline bool
DisplayMode::operator!=(const DisplayMode& rhs) const
{
    return !(*this == rhs);
}
inline void
DisplayMode::SetXPos(uint x)
{
    this->xPos = x;
}
inline uint
DisplayMode::GetXPos() const
{
    return this->xPos;
}
inline void
DisplayMode::SetYPos(uint y)
{
    this->yPos = y;
}
inline uint
DisplayMode::GetYPos() const
{
    return this->yPos;
}
inline void
DisplayMode::SetWidth(SizeT w)
{
    this->width = w;
}
inline SizeT
DisplayMode::GetWidth() const
{
    return this->width;
}
inline void
DisplayMode::SetHeight(SizeT h)
{
    this->height = h;
}
inline SizeT
DisplayMode::GetHeight() const
{
    return this->height;
}
inline void
DisplayMode::SetPixelFormat(PixelFormat::Code p)
{
    this->pixelFormat = p;
}
inline PixelFormat::Code
DisplayMode::GetPixelFormat() const
{
    return this->pixelFormat;
}
inline void
DisplayMode::SetAspectRatio(float a)
{
    this->aspectRatio = a;
}
inline float
DisplayMode::GetAspectRatio() const
{
    return this->aspectRatio;
}
inline void
DisplayMode::SetContentScale(float a)
{
    this->contentScale = a;
}
inline float
DisplayMode::GetContentScale() const
{
    return this->contentScale;
}
inline void
DisplayMode::SetRefreshRate( uint refreshRate )
{
    this->refreshRate = refreshRate;
}
inline uint
DisplayMode::GetRefreshRate() const
{
    return this->refreshRate;
}
}