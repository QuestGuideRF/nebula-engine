#pragma once
#include "core/refcounted.h"
#include "modelphysics.h"
#include "modelconstants.h"
#include "modelattributes.h"
#include "model/binarymodelwriter.h"
namespace ToolkitUtil
{
class ModelBuilder : public Core::RefCounted
{
    __DeclareClass(ModelBuilder);
public:
    ModelBuilder();
    virtual ~ModelBuilder();
    void SetConstants(const Ptr<ModelConstants>& constants);
    const Ptr<ModelConstants>& GetConstants() const;
    void SetAttributes(const Ptr<ModelAttributes>& attributes);
    const Ptr<ModelAttributes>& GetAttributes() const;
    void SetPhysics(const Ptr<ModelPhysics>& physics);
    const Ptr<ModelPhysics>& GetPhysics() const;
    bool SaveN3Physics(const IO::URI& uri, Platform::Code platform);
    bool SaveN3(const IO::URI& uri, Platform::Code platform);
private:
    void WriteShapes(const Ptr<ModelWriter>& writer);
    void WriteCharacters(const Ptr<ModelWriter>& writer);
    void WriteSkins(const Ptr<ModelWriter>& writer);
    Util::Blob WritePhysics();
    void WriteParticles(const Ptr<ModelWriter>& writer);
    Ptr<ModelConstants> constants;
    Ptr<ModelAttributes> attributes;
    Ptr<ModelPhysics> physics;
};
inline void
ModelBuilder::SetConstants(const Ptr<ModelConstants>& constants)
{
    n_assert(constants.isvalid());
    this->constants = constants;
}
inline const Ptr<ModelConstants>&
ModelBuilder::GetConstants() const
{
    return this->constants;
}
inline void
ModelBuilder::SetAttributes(const Ptr<ModelAttributes>& attributes)
{
    n_assert(attributes.isvalid());
    this->attributes = attributes;
}
inline const Ptr<ModelAttributes>&
ModelBuilder::GetAttributes() const
{
    return this->attributes;
}
inline void
ModelBuilder::SetPhysics(const Ptr<ModelPhysics>& physics)
{
    n_assert(physics.isvalid());
    this->physics = physics;
}
inline const Ptr<ModelPhysics>&
ModelBuilder::GetPhysics() const
{
    return this->physics;
}
}