#pragma once
#include "graphics/graphicscontext.h"
namespace Graphics
{
class LightProbeContext : public GraphicsContext
{
    __DeclareContext();
public:
    LightProbeContext();
    virtual ~LightProbeContext();
    static const Math::mat4& GetTransform(const Graphics::GraphicsEntityId id);
private:
    typedef Ids::IdAllocator<
        Math::mat4,
        Math::mat4
    > LightProbeAllocator;
    static LightProbeAllocator lightProbeAllocator;
    static ContextEntityId Alloc();
    static void Dealloc(ContextEntityId id);
};
}