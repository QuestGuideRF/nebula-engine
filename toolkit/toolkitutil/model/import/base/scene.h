#pragma once
#include "core/refcounted.h"
#include "scenenode.h"
#include "toolkit-common/base/exporttypes.h"
namespace ToolkitUtil
{
extern int KeysPerMS;
extern float SceneScale;
extern float AdjustedScale;
extern float AnimationFrameRate;
class ModelAttributes;
struct SkeletonBuilder;
class MeshBuilder;
class Scene
{
public:
    Scene();
    virtual ~Scene();
    void SetName(const Util::String& name);
    const Util::String& GetName() const;
    void SetCategory(const Util::String& category);
    const Util::String& GetCategory() const;
    void OptimizeGraphics(Util::Array<SceneNode*>& outMeshNodes, Util::Array<SceneNode*>& outCharacterNodes, Util::Array<MeshBuilder*>& outMeshes);
    void OptimizePhysics(Util::Array<SceneNode*>& outNodes, MeshBuilder*& outMesh);
    const Util::Array<SceneNode>& GetNodes() const;
protected:
    friend class ModelExporter;
    static void GenerateClip(SceneNode* node, AnimBuilder& animBuilder, const Util::String& name);
    void SetupSkeletons();
    void ExtractSkeletons();
    Util::Array<MeshBuilder> meshes;
    Util::Array<SkeletonBuilder> skeletons;
    Util::Array<AnimBuilder> animations;
    Util::Array<SceneNode> nodes;
    Util::String name;
    Util::String category;
    ToolkitUtil::ExportFlags flags;
};
inline void
Scene::SetName(const Util::String& name)
{
    n_assert(name.IsValid());
    this->name = name;
}
inline const Util::String&
Scene::GetName() const
{
    return this->name;
}
inline void
Scene::SetCategory(const Util::String& category)
{
    n_assert(category.IsValid());
    this->category = category;
}
inline const Util::String&
Scene::GetCategory() const
{
    return this->category;
}
inline const
Util::Array<SceneNode>& Scene::GetNodes() const
{
    return this->nodes;
}
}