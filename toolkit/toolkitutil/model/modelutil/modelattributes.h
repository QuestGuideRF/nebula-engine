#pragma once
#include "core/refcounted.h"
#include "toolkit-common/base/exporttypes.h"
#include "model/n3util/n3modeldata.h"
#include "take.h"
#include "io/stream.h"
#include "particles/emitterattrs.h"
namespace ToolkitUtil
{
class ModelAttributes : public Core::RefCounted
{
    __DeclareClass(ModelAttributes);
public:
    ModelAttributes();
    virtual ~ModelAttributes();
    void SetName(const Util::String& name);
    const Util::String& GetName() const;
    void SetScale(float f);
    const float GetScale() const;
    void SetState(const Util::String& node, const ToolkitUtil::State& state);
    const ToolkitUtil::State& GetState(const Util::String& node);
    bool HasState(const Util::String& node);
    void DeleteState(const Util::String& node);
    void SetEmitterAttrs(const Util::String& node, const Particles::EmitterAttrs& attrs);
    const Particles::EmitterAttrs& GetEmitterAttrs(const Util::String& node);
    bool HasEmitterAttrs(const Util::String& node);
    void DeleteEmitterAttrs(const Util::String& node);
    void SetEmitterMesh(const Util::String& node, const Util::String& mesh);
    const Util::String& GetEmitterMesh(const Util::String& node);
    void DeleteEmitterMesh(const Util::String& node);
    void AddTake(const Ptr<Take>& take);
    const Ptr<Take>& GetTake(uint index);
    const Ptr<Take>& GetTake(const Util::String& name);
    const Util::Array<Ptr<Take> >& GetTakes() const;
    const bool HasTake(const Ptr<Take>& take);
    const bool HasTake(const Util::String& name);
    void ClearTakes();
    void SetJointMask(const JointMask& mask);
    void SetJointMasks(const Util::Array<JointMask>& masks);
    const Util::Array<JointMask>& GetJointMasks() const;
    void SetExportFlags(const ToolkitUtil::ExportFlags& exportFlags);
    const ToolkitUtil::ExportFlags& GetExportFlags() const;
    void Clear();
    void Save(const Ptr<IO::Stream>& stream);
    void Load(const Ptr<IO::Stream>& stream);
private:
    Util::Dictionary<Util::String, ToolkitUtil::State> nodeStateMap;
    Util::Dictionary<Util::String, Particles::EmitterAttrs> particleAttrMap;
    Util::Dictionary<Util::String, Util::String> particleMeshMap;
    Util::String name;
    Util::String checksum;
    float scaleFactor;
    Util::Array<Ptr<Take>> takes;
    Util::Array<JointMask> jointMasks;
    ToolkitUtil::ExportFlags exportFlags;
    static const short Version = 1;
};
inline void
ModelAttributes::SetName( const Util::String& name )
{
    n_assert(name.IsValid());
    this->name = name;
}
inline const Util::String&
ModelAttributes::GetName() const
{
    return this->name;
}
inline void
ModelAttributes::SetScale( float f )
{
    this->scaleFactor = f;
}
inline const float
ModelAttributes::GetScale() const
{
    return this->scaleFactor;
}
inline void
ModelAttributes::SetExportFlags( const ToolkitUtil::ExportFlags& exportFlags )
{
    this->exportFlags = exportFlags;
}
inline const ToolkitUtil::ExportFlags&
ModelAttributes::GetExportFlags() const
{
    return this->exportFlags;
}
inline void
ModelAttributes::SetJointMask(const JointMask& mask)
{
    IndexT i = this->jointMasks.FindIndex(mask);
    if (i == InvalidIndex)  this->jointMasks.Append(mask);
    else                    this->jointMasks[i].weights = mask.weights;
}
inline void
ModelAttributes::SetJointMasks(const Util::Array<JointMask>& masks)
{
    this->jointMasks = masks;
}
inline const Util::Array<JointMask>&
ModelAttributes::GetJointMasks() const
{
    return this->jointMasks;
}
}