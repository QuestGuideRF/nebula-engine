#pragma once
#include "coregraphics/base/textrendererbase.h"
#include "coregraphics/shader.h"
#include "coregraphics/texture.h"
#include "coregraphics/primitivegroup.h"
#include "coregraphics/resourcetable.h"
#include "stb_truetype.h"
namespace Vulkan
{
class VkTextRenderer : public Base::TextRendererBase
{
    __DeclareClass(VkTextRenderer);
    __DeclareSingleton(VkTextRenderer);
public:
    struct TextElementVertex
    {
        Math::float2 vertex;
        Math::float2 uv;
        Math::float4 color;
    };
    VkTextRenderer();
    virtual ~VkTextRenderer();
    void Open();
    void Close();
    void DrawTextElements(const CoreGraphics::CmdBufferId cmdBuf);
    static const int MaxNumChars = 65535;
private:
    void Draw(const CoreGraphics::CmdBufferId cmdBuf, TextElementVertex* buffer, SizeT numChars);
    Math::vec2 TransformTextVertex(const Math::vec2& pos, const Math::vec2& offset, const Math::vec2& scale);
    stbtt_packedchar* cdata;
    stbtt_fontinfo font;
    CoreGraphics::ResourceTableId textTable;
    CoreGraphics::ShaderProgramId program;
    IndexT texVar;
    IndexT modelVar;
    CoreGraphics::TextureId glyphTexture;
    CoreGraphics::PrimitiveGroup group;
    CoreGraphics::BufferId vbo;
    CoreGraphics::VertexLayoutId layout;
    byte* vertexPtr;
};
}