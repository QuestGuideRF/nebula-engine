#pragma once
#include "core/types.h"
#include "util/hashtable.h"
#include "coregraphics/texture.h"
#include "coregraphics/buffer.h"
#include "coregraphics/vertexlayout.h"
#include "coregraphics/pass.h"
#include "Ultralight/Ultralight.h"
#include "AppCore/Platform.h"
namespace StaticUI
{
class UltralightRenderer : public ultralight::GPUDriver
{
public:
    UltralightRenderer();
    ~UltralightRenderer();
    uint32_t NextTextureId() override;
    void CreateTexture(uint32_t texture_id, ultralight::Ref<ultralight::Bitmap> bitmap) override;
    void UpdateTexture(uint32_t texture_id, ultralight::Ref<ultralight::Bitmap> bitmap) override;
    void DestroyTexture(uint32_t texture_id) override;
    uint32_t NextRenderBufferId() override;
    void CreateRenderBuffer(uint32_t render_buffer_id, const ultralight::RenderBuffer& buffer) override;
    void DestroyRenderBuffer(uint32_t render_buffer_id) override;
    uint32_t NextGeometryId() override;
    void CreateGeometry(uint32_t geometry_id,
                                const ultralight::VertexBuffer& vertices,
                                const ultralight::IndexBuffer& indices) override;
    void UpdateGeometry(uint32_t geometry_id,
                                const ultralight::VertexBuffer& vertices,
                                const ultralight::IndexBuffer& indices) override;
    void DestroyGeometry(uint32_t geometry_id) override;
    void UpdateCommandList(const ultralight::CommandList& list) override;
    void BeginSynchronize() override;
    void EndSynchronize() override;
    void PreDraw(const ultralight::RenderTarget& view);
    void Render(const CoreGraphics::CmdBufferId& cmds, IndexT bufferIndex);
    void DrawToBackbuffer(const CoreGraphics::CmdBufferId& cmds, IndexT bufferIndex);
private:
    friend uint GetTexture(UltralightRenderer* renderer, uint textureId);
    struct TextureHandle
    {
        CoreGraphics::TextureId tex;
        bool initial;
    };
    Util::HashTable<uint, TextureHandle> textureMap;
    struct RenderBufferHandle
    {
        CoreGraphics::PassId pass;
        CoreGraphics::TextureViewId texView;
    };
    Util::HashTable<uint, RenderBufferHandle> renderBufferMap;
    struct GeometryHandle
    {
        CoreGraphics::BufferId vbo;
        CoreGraphics::BufferId ibo;
        CoreGraphics::VertexLayoutId vlo;
    };
    Util::HashTable<uint, GeometryHandle> geometryMap;
    uint textureId, renderBufferId, geometryId;
    Util::Array<ultralight::Command> commands;
    Util::Array<uint> constantOffsets;
};
}