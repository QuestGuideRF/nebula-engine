#pragma once
#include "resources/resourceid.h"
#include "io/uri.h"
#include "io/stream.h"
#include "imagefileformat.h"
namespace CoreGraphics
{
bool
SaveTexture(const Resources::ResourceId& id, const IO::URI& path, IndexT mip, CoreGraphics::ImageFileFormat::Code code);
bool
SaveTexture(const Resources::ResourceId& id, const Ptr<IO::Stream>& stream, IndexT mip, CoreGraphics::ImageFileFormat::Code code);
}