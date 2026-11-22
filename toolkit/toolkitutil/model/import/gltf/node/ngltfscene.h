#pragma once
#include "model/import/base/scene.h"
#include "ngltfmesh.h"
#include "ngltfnode.h"
#include "model/meshutil/meshbuilder.h"
#include "toolkit-common/base/exporttypes.h"
#include "model/import/gltf/gltfdata.h"
#include "meshprimitive.h"
namespace ToolkitUtil
{
class NglTFScene : public Scene
{
public:
    NglTFScene();
    virtual ~NglTFScene();
    void Setup(Gltf::Document* scene, const ToolkitUtil::ExportFlags& exportFlags, float scale);
private:
    void ParseNodeHierarchy(
        Gltf::Document* scene,
        Gltf::Node* fbxNode,
        SceneNode* parent,
        Util::Dictionary<Gltf::Node*, SceneNode*>& lookup,
        Util::Array<SceneNode>& nodes
    );
};
}