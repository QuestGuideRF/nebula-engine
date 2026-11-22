#pragma once
#include "core/types.h"
#include "util/string.h"
namespace ToolkitUtil
{
class ImageConverter
{
public:
    enum Result
    {
        Success = 1,
        ErrorOpenSrcFile,
        ErrorWriteDstFile,
    };
    ImageConverter();
    ~ImageConverter();
    void SetSrcFile(const Util::String& srcFile);
    void SetDstFile(const Util::String& dstFile);
    void SetMaxWidth(SizeT maxWidth);
    void SetMaxHeight(SizeT maxHeight);
    Result Convert();
    SizeT GetDstNumChannels() const;
    SizeT GetDstWidth() const;
    SizeT GetDstHeight() const;
private:
    Util::String srcFile;
    Util::String dstFile;
    SizeT maxWidth;
    SizeT maxHeight;
    SizeT dstNumChannels;
    SizeT dstWidth;
    SizeT dstHeight;
};
inline void
ImageConverter::SetSrcFile(const Util::String& f)
{
    this->srcFile = f;
}
inline void
ImageConverter::SetDstFile(const Util::String& f)
{
    this->dstFile = f;
}
inline void
ImageConverter::SetMaxWidth(SizeT w)
{
    this->maxWidth = w;
}
inline void
ImageConverter::SetMaxHeight(SizeT h)
{
    this->maxHeight = h;
}
inline SizeT
ImageConverter::GetDstNumChannels() const
{
    return this->dstNumChannels;
}
inline SizeT
ImageConverter::GetDstWidth() const
{
    return this->dstWidth;
}
inline SizeT
ImageConverter::GetDstHeight() const
{
    return this->dstHeight;
}
}