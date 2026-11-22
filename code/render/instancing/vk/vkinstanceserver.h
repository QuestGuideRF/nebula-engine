#pragma once
#include "instancing/base/instanceserverbase.h"
namespace Vulkan
{
class VkInstanceServer : public Base::InstanceServerBase
{
    __DeclareSingleton(VkInstanceServer);
    __DeclareClass(VkInstanceServer);
public:
    VkInstanceServer();
    virtual ~VkInstanceServer();
    bool Open();
    void Close();
    void Render(IndexT frameIndex);
private:
    CoreGraphics::ShaderFeature::Mask instancingFeatureBits;
};
}