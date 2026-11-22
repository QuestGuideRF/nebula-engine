#pragma once
#include "core/refcounted.h"
#include "resources/resourceloader.h"
#include "util/stack.h"
#include "model.h"
#include "nodes/characterskinnode.h"
#include "nodes/particlesystemnode.h"
namespace Visibility
{
class VisibilityContext;
}
namespace Models
{
class ModelNode;
class ModelContext;
class ModelLoader : public Resources::ResourceLoader
{
    __DeclareClass(ModelLoader);
public:
    ModelLoader();
    virtual ~ModelLoader();
    void Setup() override;
private:
    friend class PrimitiveNode;
    friend class CharacterNode;
    friend class CharacterSkinNode;
    friend class ParticleSystemNode;
    friend class Models::ModelContext;
    friend class Visibility::VisibilityContext;
    ResourceLoader::ResourceInitOutput InitializeResource(const ResourceLoadJob& job, const Ptr<IO::Stream>& stream) override;
    void Unload(const Resources::ResourceId id) override;
    ResourceStreamOutput StreamResource(const ResourceLoadJob& job) override;
    uint LodMask(const _StreamData& stream, float lod, bool async) const override;
    Util::Dictionary<Util::FourCC, std::function<Models::ModelNode* ()>> nodeConstructors;
#define IMPLEMENT_NODE_ALLOCATOR(FourCC, Type) \
    nodeConstructors.Add(FourCC, []() -> Models::ModelNode* { \
        Models::ModelNode* node = (Models::ModelNode*)Memory::Alloc(Memory::ObjectHeap, sizeof(Models::Type)); \
        new (node) Models::Type; \
        return node; \
    });
};
const Util::Array<Models::ModelNode*>& ModelGetNodes(const ModelId id);
}