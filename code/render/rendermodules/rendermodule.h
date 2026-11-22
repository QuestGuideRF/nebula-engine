#pragma once
#include "core/refcounted.h"
namespace RenderModules
{
class RenderModule : public Core::RefCounted
{
    __DeclareClass(RenderModule);
public:
    RenderModule();
    virtual ~RenderModule();
    virtual void Setup();
    virtual void Discard();
    bool IsValid() const;
    virtual void OnFrame();
private:
    bool isValid;
};
inline bool
RenderModule::IsValid() const
{
    return this->isValid;
}
}