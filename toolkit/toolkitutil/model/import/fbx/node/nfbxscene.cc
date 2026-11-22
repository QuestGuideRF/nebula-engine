//------------------------------------------------------------------------------
//  fbxnoderegistry.cc
//  (C) 2012-2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "nfbxscene.h"
#include "model/import/fbx/nfbxexporter.h"
#include "model/skeletonutil/skeletonbuilder.h"
#include "nfbxmeshnode.h"
#include "nfbxjointnode.h"
#include "nfbxlightnode.h"
#include "timing/timer.h"

using namespace Util;
using namespace ToolkitUtil;
namespace ToolkitUtil
{

//------------------------------------------------------------------------------
/**
*/
NFbxScene::NFbxScene()
{
}

//------------------------------------------------------------------------------
/**
*/
NFbxScene::~NFbxScene()
{
}

//------------------------------------------------------------------------------
/**
*/
void
NFbxScene::ParseNodeHierarchy(
    ufbx_node* fbxNode
    , SceneNode* parent
    , Util::Dictionary<ufbx_node*, SceneNode*>& lookup
    , Util::Array<SceneNode>& nodes
)
{
    SceneNode& node = nodes.Emplace();
    lookup.Add(fbxNode, &node);
    ufbx_element_type attr = fbxNode->attrib_type;
    switch (attr)
    {
        case UFBX_ELEMENT_BONE:
        {
            node.Setup(SceneNode::NodeType::Joint);
            NFbxJointNode::Setup(&node, parent, fbxNode); 
            break;
        }
        case UFBX_ELEMENT_MESH:
        {
            node.Setup(SceneNode::NodeType::Mesh);
            NFbxNode::Setup(&node, parent, fbxNode);
            break;
        }
        case UFBX_ELEMENT_LOD_GROUP:
        {
            node.Setup(SceneNode::NodeType::Lod);
            NFbxNode::Setup(&node, parent, fbxNode);
            break;
        }
        case UFBX_ELEMENT_LIGHT:
        {
            node.Setup(SceneNode::NodeType::Light);
            NFbxLightNode::Setup(&node, parent, fbxNode);
            break;
        }
        default:
        {
            node.Setup(SceneNode::NodeType::Transform);
            NFbxNode::Setup(&node, parent, fbxNode);
            break;
        }
    }

    size_t numChildren = fbxNode->children.count;
    for (size_t i = 0; i < numChildren; i++)
    {
        ufbx_node* child = fbxNode->children[i];
        ParseNodeHierarchy(child, &node, lookup, nodes);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
NFbxScene::Setup(
    ufbx_scene* scene
    , const ExportFlags& exportFlags
    , const Ptr<ModelAttributes>& attributes
    , float scale
    , ToolkitUtil::Logger* logger
)
{
    n_assert(scene != nullptr);

    // установить настройки экспорта
    this->flags = exportFlags;
    this->logger = logger;

    // установить масштаб
    SceneScale = scale;
    AdjustedScale = SceneScale;

    
    float fps = TimeModeToFPS(scene->settings.time_mode);
    // обрабатывать специальный случай для пользовательского FPS
    if (fps == -1)
    {
        fps = scene->settings.frames_per_second;
    }
    AnimationFrameRate = fps;

    // Получить количество сеток
    size_t nodeCount = scene->nodes.count;
    this->nodes.Reserve((SizeT)nodeCount);

    // Пройдитесь по всем узлам и добавьте их в наш поиск.
    Util::Dictionary<ufbx_node*, SceneNode*> nodeLookup;
    ParseNodeHierarchy(scene->root_node, nullptr, nodeLookup, this->nodes);

    // Настройка скелетной иерархии
    this->SetupSkeletons(); 

    // Извлечение данных из узлов
    for (int nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        SceneNode* node = &this->nodes[nodeIndex];
        switch (node->type)
        {
            case SceneNode::NodeType::Mesh:
                NFbxMeshNode::ExtractMesh(node, this->meshes, nodeLookup, flags);
                break;
        }
    }

    // Обновите конструктор скелетов
    this->ExtractSkeletons();

    // Извлечение кривых анимации
    size_t animStackCount = scene->anim_stacks.count;
    for (size_t animStackIndex = 0; animStackIndex < animStackCount; animStackIndex++)
    {
        ufbx_anim_stack* animStack = scene->anim_stacks[animStackIndex];
        for (int nodeIndex = 0; nodeIndex < this->nodes.Size(); nodeIndex++)
        {
            SceneNode* node = &this->nodes[nodeIndex];
            NFbxNode::PrepareAnimation(node, animStack);
            if (node->base.isAnimated && (node->base.parent == nullptr || !node->base.parent->base.isAnimated))
            {
                node->anim.animIndex = this->animations.Size();

                AnimBuilder& anim = this->animations.Emplace();

                // Извлечение ключей анимации из иерархии узлов
                NFbxNode::ExtractAnimation(node, anim.keys, anim.keyTimes, animStack);

                // Создать один клип для каждого стека анимации.
                Scene::GenerateClip(node, anim, Util::String(animStack->name.data, animStack->name.length));
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
float
NFbxScene::TimeModeToFPS(const ufbx_time_mode timeMode)
{
    switch (timeMode)
    {
        case UFBX_TIME_MODE_100_FPS: return 100;
        case UFBX_TIME_MODE_120_FPS: return 120;
        case UFBX_TIME_MODE_1000_FPS: return 1000;
        case UFBX_TIME_MODE_30_FPS: return 30;
        case UFBX_TIME_MODE_30_FPS_DROP: return 30;
        case UFBX_TIME_MODE_48_FPS: return 48;
        case UFBX_TIME_MODE_50_FPS: return 50;
        case UFBX_TIME_MODE_60_FPS: return 60;
        case UFBX_TIME_MODE_NTSC_DROP_FRAME: return 29.97002617f;
        case UFBX_TIME_MODE_NTSC_FULL_FRAME: return 29.97002617f;
        case UFBX_TIME_MODE_PAL: return 25;
        case UFBX_TIME_MODE_CUSTOM: return -1; // недопустимый режим, должен обрабатываться отдельно
        default: return 24;
    }
}

} // пространство имен ToolkitUtil