#pragma once
#include "coregraphics/gpubuffertypes.h"
#include "coregraphics/config.h"
namespace CoreGraphics
{
RESOURCE_ID_TYPE(SparseBufferId);
struct SparseBufferCreateInfo
{
};
SparseBufferId CreateSparseBuffer(const SparseBufferCreateInfo& info);
void DestroySparseBuffer(const SparseBufferId id);
}