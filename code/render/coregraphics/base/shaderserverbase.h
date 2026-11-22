#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "coregraphics/shader.h"
#include "coregraphics/shaderfeature.h"
#include "coregraphics/shader.h"
#include "coregraphics/shaderidentifier.h"
#include "coregraphics/shaderloader.h"
#include "threading/safequeue.h"
namespace Threading
{
class Thread;
}
namespace IO
{
class FileWatcher;
class FileWatcherThread;
}
namespace CoreGraphics
{
class Shader;
}
namespace Frame
{
class FrameScript;
};
namespace Base
{
class ShaderServerBase : public Core::RefCounted
{
    __DeclareClass(ShaderServerBase);
    __DeclareSingleton(ShaderServerBase);
public:
    ShaderServerBase();
    virtual ~ShaderServerBase();
    bool Open();
    void Close();
    bool IsOpen() const;
    bool HasShader(const Resources::ResourceName& resId) const;
    const Util::Dictionary<Resources::ResourceName, Resources::ResourceId>& GetAllShaders() const;
    const CoreGraphics::ShaderId GetShader(Resources::ResourceName resId) const;
    void ResetFeatureBits();
    void SetFeatureBits(CoreGraphics::ShaderFeature::Mask m);
    void ClearFeatureBits(CoreGraphics::ShaderFeature::Mask m);
    CoreGraphics::ShaderFeature::Mask GetFeatureBits() const;
    CoreGraphics::ShaderFeature::Mask FeatureStringToMask(const Util::String& str);
    Util::String FeatureMaskToString(CoreGraphics::ShaderFeature::Mask mask);
    void LoadShader(const Resources::ResourceName& shdName);
    void BeforeFrame();
protected:
    friend class CoreGraphics::ShaderIdentifier;
    friend class ShaderBase;
    CoreGraphics::ShaderIdentifier shaderIdentifierRegistry;
    CoreGraphics::ShaderFeature shaderFeature;
    CoreGraphics::ShaderFeature::Mask curShaderFeatureBits;
    Util::Dictionary<Resources::ResourceName, Resources::ResourceId> shaders;
    Threading::SafeQueue<Resources::ResourceName> pendingShaderReloads;
    Ids::Id32 objectIdShaderVar;
    bool isOpen;
};
inline bool
ShaderServerBase::IsOpen() const
{
    return this->isOpen;
}
inline bool
ShaderServerBase::HasShader(const Resources::ResourceName& resId) const
{
    return this->shaders.Contains(resId);
}
inline const Util::Dictionary<Resources::ResourceName, Resources::ResourceId>&
ShaderServerBase::GetAllShaders() const
{
    return this->shaders;
}
inline const CoreGraphics::ShaderId
ShaderServerBase::GetShader(Resources::ResourceName resId) const
{
    n_assert_fmt(this->shaders.Contains(resId), "%s не найден!\n Возможно, это проблема с экспортом. Проверьте папку экспорта!\n", resId.Value());
    Resources::ResourceId shader = this->shaders[resId];
    CoreGraphics::ShaderId ret = shader.resource;
    return ret;
}
inline CoreGraphics::ShaderFeature::Mask
ShaderServerBase::FeatureStringToMask(const Util::String& str)
{
    return this->shaderFeature.StringToMask(str);
}
inline Util::String
ShaderServerBase::FeatureMaskToString(CoreGraphics::ShaderFeature::Mask mask)
{
    return this->shaderFeature.MaskToString(mask);
}
inline void
ShaderServerBase::ResetFeatureBits()
{
    this->curShaderFeatureBits = 0;
}
inline void
ShaderServerBase::SetFeatureBits(CoreGraphics::ShaderFeature::Mask m)
{
    this->curShaderFeatureBits |= m;
}
inline void
ShaderServerBase::ClearFeatureBits(CoreGraphics::ShaderFeature::Mask m)
{
    this->curShaderFeatureBits &= ~m;
}
inline CoreGraphics::ShaderFeature::Mask
ShaderServerBase::GetFeatureBits() const
{
    return this->curShaderFeatureBits;
}
}