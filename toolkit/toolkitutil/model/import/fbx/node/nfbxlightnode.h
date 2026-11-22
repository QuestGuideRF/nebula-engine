#pragma once
#include "model/import/base/scenenode.h"
namespace ToolkitUtil
{
class SceneNode;
class NFbxLightNode
{
public:
    static void Setup(SceneNode* node, SceneNode* parent, ufbx_node* fbxNode);
    enum LightType
    {
        Spotlight,
        Pointlight,
        AreaLight,
        InvalidLight,
        NumLights
    };
};
}