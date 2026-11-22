#pragma once
#include "core/singleton.h"
#include "model/import/base/scene.h"
#include "model/meshutil/meshbuilder.h"
#include "toolkit-common/base/exporttypes.h"
#include "toolkit-common/logger.h"
#include "ufbx/ufbx.h"
namespace ToolkitUtil
{
class NFbxScene : public Scene
{
public:
    NFbxScene();
    virtual ~NFbxScene();
    void Setup(
        ufbx_scene* scene
        , const ToolkitUtil::ExportFlags& exportFlags
        , const Ptr<ModelAttributes>& attributes
        , float scale
        , ToolkitUtil::Logger* logger
    );
private:
    void ParseNodeHierarchy(
        ufbx_node* fbxNode
        , SceneNode* parent
        , Util::Dictionary<ufbx_node*, SceneNode*>& lookup
        , Util::Array<SceneNode>& nodes
    );
    float TimeModeToFPS(const ufbx_time_mode timeMode);
    ToolkitUtil::Logger* logger;
};
}