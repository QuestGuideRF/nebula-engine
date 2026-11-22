#pragma once
#include "core/refcounted.h"
#include "math/matrix44.h"
#include "coregraphics/shaderstate.h"
#include "coregraphics/shader.h"
namespace Base
{
class InstanceRendererBase : public Core::RefCounted
{
    __DeclareClass(InstanceRendererBase);
public:
    InstanceRendererBase();
    virtual ~InstanceRendererBase();
    void Setup();
    void Close();
    void SetShader(const Ptr<CoreGraphics::Shader>& shader);
    const Ptr<CoreGraphics::Shader>& GetShader() const;
    void SetInstanceMultiplier(SizeT multiplier);
    void BeginUpdate(SizeT amount);
    void AddTransform(const Math::matrix44& matrix);
    void AddId(const int id);
    void EndUpdate();
    virtual void Render(const SizeT multiplier);
protected:
    Ptr<CoreGraphics::Shader> shader;
    Util::Array<Math::matrix44> modelTransforms;
    Util::Array<Math::matrix44> modelViewTransforms;
    Util::Array<Math::matrix44> modelViewProjectionTransforms;
    Util::Array<int> objectIds;
    bool inBeginUpdate;
    bool isOpen;
};
inline void
InstanceRendererBase::SetShader(const Ptr<CoreGraphics::Shader>& shader)
{
    n_assert(shader.isvalid());
    this->shader = shader;
}
inline const Ptr<CoreGraphics::Shader>&
InstanceRendererBase::GetShader() const
{
    return this->shader;
}
}