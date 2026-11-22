#pragma once
#include "core/refcounted.h"
#include "instancing/base/instancerendererbase.h"
#include "coregraphics/constantbuffer.h"
namespace Vulkan
{
class VkInstanceRenderer : public Base::InstanceRendererBase
{
    __DeclareClass(VkInstanceRenderer);
public:
    VkInstanceRenderer();
    virtual ~VkInstanceRenderer();
    void Setup();
    void Close();
    void Render(const SizeT multiplier);
private:
    Ptr<CoreGraphics::ShaderState> shaderState;
    Ptr<CoreGraphics::ConstantBuffer> instancingBuffer;
    Ptr<CoreGraphics::ShaderVariable> instancingBlockVar;
    Ptr<CoreGraphics::ShaderVariable> modelArrayVar;
    Ptr<CoreGraphics::ShaderVariable> modelViewArrayVar;
    Ptr<CoreGraphics::ShaderVariable> modelViewProjectionArrayVar;
    Ptr<CoreGraphics::ShaderVariable> idArrayVar;
    static const int MaxInstancesPerBatch = 256;
};
}