#pragma once
namespace Graphics
{
struct BindlessRegistryCreateInfo
{
};
typedef uint32_t BindlessIndex;
void CreateBindlessRegistry(const BindlessRegistryCreateInfo& info);
void DestroyBindlessRegistry();
BindlessIndex RegisterTexture(const CoreGraphics::TextureId& tex, CoreGraphics::TextureType type, bool depth = false, bool stencil = false);
void ReregisterTexture(const CoreGraphics::TextureId& tex, CoreGraphics::TextureType type, BindlessIndex id, bool depth = false, bool stencil = false);
void UnregisterTexture(const BindlessIndex id, const CoreGraphics::TextureType type);
}