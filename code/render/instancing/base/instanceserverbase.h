#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "models/modelnodeinstance.h"
#include "models/modelnode.h"
namespace Instancing
{
    class InstanceRenderer;
}
namespace Base
{
class InstanceServerBase : public Core::RefCounted
{
    __DeclareSingleton(InstanceServerBase);
    __DeclareClass(InstanceServerBase);
public:
    InstanceServerBase();
    virtual ~InstanceServerBase();
    virtual bool Open();
    virtual void Close();
    bool IsOpen() const;
    void BeginInstancing(const Ptr<Models::ModelNode>& modelNode, const SizeT multiplier, const Ptr<CoreGraphics::Shader>& shader, const IndexT& pass);
    void AddInstance(const IndexT& instanceCode, const Ptr<Models::ModelNodeInstance>& instance);
    void EndInstancing();
    virtual void Render(IndexT frameIndex);
protected:
    IndexT pass;
    Ptr<CoreGraphics::Shader> shader;
    Ptr<Instancing::InstanceRenderer> renderer;
    Ptr<Models::ModelNode> modelNode;
    Util::Dictionary<IndexT, Util::Array<Ptr<Models::ModelNodeInstance>>> instancesByCode;
    bool isBeginInstancing;
    bool isOpen;
    SizeT multiplier;
};
inline bool
InstanceServerBase::IsOpen() const
{
    return this->isOpen;
}
}