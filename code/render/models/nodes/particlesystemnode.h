#pragma once
#include "shaderstatenode.h"
#include "particles/emitterattrs.h"
#include "particles/envelopesamplebuffer.h"
#include "particles/emittermesh.h"
#include "coregraphics/resourcetable.h"
namespace Models
{
class ModelContext;
class ParticleSystemNode : public ShaderStateNode
{
public:
    ParticleSystemNode();
    virtual ~ParticleSystemNode();
    virtual void UpdateMeshResource(const Resources::ResourceName& msh);
    IndexT GetPrimitiveGroupIndex() const;
    void SetEmitterAttrs(const Particles::EmitterAttrs& attrs);
    const Particles::EmitterAttrs& GetEmitterAttrs() const;
    const Particles::EmitterMesh& GetEmitterMesh() const;
    const Particles::EnvelopeSampleBuffer& GetSampleBuffer() const;
    static Util::FixedArray<CoreGraphics::ResourceTableId> CreateResourceTables();
private:
    Particles::EnvelopeCurve ParseEnvelopeCurveData(const Ptr<IO::BinaryReader>& reader) const;
protected:
    friend class Particles::ParticleContext;
    friend class Models::ModelContext;
    virtual void OnFinishedLoading(ModelStreamingData* streamingData) override;
    virtual bool Load(const Util::FourCC& fourcc, const Util::StringAtom& tag, const Ptr<IO::BinaryReader>& reader, bool immediate) override;
    Particles::EnvelopeSampleBuffer sampleBuffer;
    Particles::EmitterAttrs emitterAttrs;
    Particles::EmitterMesh emitterMesh;
    Resources::ResourceName meshResId;
    Util::StringAtom tag;
    IndexT primGroupIndex;
    Resources::ResourceId meshResource;
    CoreGraphics::MeshId mesh;
};
inline const Particles::EmitterMesh&
ParticleSystemNode::GetEmitterMesh() const
{
    return this->emitterMesh;
}
inline const Particles::EnvelopeSampleBuffer&
ParticleSystemNode::GetSampleBuffer() const
{
    return this->sampleBuffer;
}
inline IndexT
ParticleSystemNode::GetPrimitiveGroupIndex() const
{
    return this->primGroupIndex;
}
inline void
ParticleSystemNode::SetEmitterAttrs(const Particles::EmitterAttrs& attrs)
{
    this->emitterAttrs = attrs;
}
inline const Particles::EmitterAttrs&
ParticleSystemNode::GetEmitterAttrs() const
{
    return this->emitterAttrs;
}
}