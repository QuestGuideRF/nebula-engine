#pragma once
#include "core/types.h"
#include "coregraphics/pixelformat.h"
#ifndef GLIML_ASSERT
#define GLIML_ASSERT(x) n_assert(x)
#endif
#include "gliml.h"
namespace CoreGraphics
{
class Gliml
{
public:
    static CoreGraphics::PixelFormat::Code ToPixelFormat(gliml::context const& ctx);
};
}