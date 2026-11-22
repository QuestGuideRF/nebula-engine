#pragma once
namespace Graphics
{
struct GlobalConstantsCreateInfo
{
};
void CreateGlobalConstants(const GlobalConstantsCreateInfo& info);
void DestroyGlobalConstants();
void AllocateGlobalConstants();
void UpdateTickParams(const Shared::PerTickParams& tickParams);
void UpdateViewConstants(const Shared::ViewConstants& viewConstants);
void UpdateShadowConstants(const Shared::ShadowViewConstants& shadowViewConstants);
void FlushUpdates(const CoreGraphics::CmdBufferId buf, const CoreGraphics::QueueType queue);
void GetOffsets(uint64_t& tickOffset, uint64_t& viewOffset, uint64_t& shadowOffset);
const Shared::PerTickParams& GetTickParams();
const Shared::ViewConstants& GetViewConstants();
const Shared::ShadowViewConstants& GetShadowViewConstants();
void SetGlobalEnvironmentTextures(const CoreGraphics::TextureId& env, const CoreGraphics::TextureId& irr, const SizeT numMips);
void SetupBufferConstants();
const CoreGraphics::ResourceTableId GetFrameResourceTable(uint32_t bufferIndex);
const CoreGraphics::ResourceTableId GetTickResourceTable(uint32_t bufferIndex);
}