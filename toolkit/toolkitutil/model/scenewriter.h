#pragma once
#include "toolkit-common/platform.h"
#include "model/import/base/scene.h"
namespace ToolkitUtil
{
class N3Writer;
class ModelAttributes;
class ModelPhysics;
class ModelConstants;
class SceneWriter
{
public:
    static void GenerateModels(
        const Util::String& basePath
        , const Scene* scene
        , const Platform::Code platform
        , const Util::Array<SceneNode*>& graphicsNodes
        , const Util::Array<SceneNode*>& physicsNodes
        , const Util::Array<SceneNode*>& characterNodes
        , const Util::String& physicsMeshResource
        , const ToolkitUtil::ExportFlags& flags
    );
private:
    static void CreateModel(
        const Util::String& file
        , const Scene* scene
        , const Util::String& category
        , const Ptr<ModelConstants>& constants
        , const Ptr<ModelAttributes>& attributes
        , const Ptr<ModelPhysics>& physics
        , const Util::Array<SceneNode*>& graphicsNodes
        , const Util::Array<SceneNode*>& physicsNodes
        , const Util::Array<SceneNode*>& characterNodes
    );
    static void UpdateConstants(const Util::String& file, const Ptr<ToolkitUtil::ModelConstants>& constants);
    static void UpdateAttributes(const Util::String& file, const Ptr<ToolkitUtil::ModelAttributes>& attributes);
    static void UpdatePhysics(const Util::String& file, const Ptr<ToolkitUtil::ModelPhysics>& physics);
};
}