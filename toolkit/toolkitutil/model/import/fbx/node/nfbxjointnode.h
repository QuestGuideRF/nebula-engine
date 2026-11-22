#pragma once
#include "model/import/base/scenenode.h"
namespace ToolkitUtil
{
class SceneNode;
class NFbxJointNode
{
public:
    static void Setup(SceneNode* node, SceneNode* parent, ufbx_node* fbxNode);
};
}