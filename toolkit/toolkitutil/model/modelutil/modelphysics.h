#pragma once
#include "core/refcounted.h"
#include "toolkit-common/base/exporttypes.h"
#include "io/stream.h"
#include "flat/physics/material.h"
namespace ToolkitUtil
{
class ModelPhysics : public Core::RefCounted
{
    __DeclareClass(ModelPhysics);
public:
    ModelPhysics();
    virtual ~ModelPhysics();
    void SetName(const Util::String& name);
    const Util::String& GetName() const;
    void SetMeshMode(Physics::MeshTopology flags);
    Physics::MeshTopology GetMeshMode();
    void SetExportMode(PhysicsExportMode flags);
    PhysicsExportMode GetExportMode();
    void SetPhysicsMesh(const Util::String & mesh);
    const Util::String& GetPhysicsMesh() const;
    const Util::String& GetMaterial() const;
    void SetMaterial(const Util::String& material);
    void Clear();
    void Save(const Ptr<IO::Stream>& stream);
    void Load(const Ptr<IO::Stream>& stream);
private:
    PhysicsExportMode physicsMode;
    Physics::MeshTopology meshMode;
    Util::String name;
    Util::String material;
    Util::String checksum;
    Util::String physicsMesh;
    static const short Version = 1;
};
inline void
ModelPhysics::SetName(const Util::String& name)
{
    n_assert(name.IsValid());
    this->name = name;
}
inline const Util::String&
ModelPhysics::GetName() const
{
    return this->name;
}
inline void
ModelPhysics::SetPhysicsMesh(const Util::String& name)
{
    n_assert(name.IsValid());
    this->physicsMesh = name;
}
inline const Util::String&
ModelPhysics::GetPhysicsMesh() const
{
    return this->physicsMesh;
}
inline void
ModelPhysics::SetMaterial(const Util::String& material)
{
    n_assert(material.IsValid());
    this->material = material;
}
inline const Util::String&
ModelPhysics::GetMaterial() const
{
    return this->material;
}
}