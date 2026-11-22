//------------------------------------------------------------------------------
//  ngltfscene.cc
//  (C) 2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "ngltfscene.h"
#include "model/modelutil/modeldatabase.h"
#include "model/skeletonutil/skeletonbuilder.h"
#include "model/n3util/n3modeldata.h"
#include "timing/timer.h"
#include "model/import/base/uniquestring.h"
#include <array>
#include <functional>
#include "math/scalar.h"

using namespace Util;
using namespace ToolkitUtil;
namespace ToolkitUtil
{

//------------------------------------------------------------------------------
/**
*/
CoreAnimation::CurveType::Code
GltfPathToCurveType(Util::String const& path)
{
    if (path == "translation")
    {
        return CoreAnimation::CurveType::Translation;
    }
    else if (path == "rotation")
    {
        return CoreAnimation::CurveType::Rotation;
    }
    else if (path == "scale")
    {
        return CoreAnimation::CurveType::Scale;
    }
    else if (path == "weights")
    {
        n_error("Цели трансформации в настоящее время не поддерживаются!\n");
    }
}

//------------------------------------------------------------------------------
/**
    возвращает значение, увеличивает bytesRead на количество прочитанных байтов
*/
float
ReadCurveValue(byte const* const data, Gltf::Accessor::ComponentType componentType, uint64_t& bytesRead)
{
    switch (componentType)
    {
    case Gltf::Accessor::ComponentType::Float:
        bytesRead += sizeof(float);
        return *((float*)data);
    case Gltf::Accessor::ComponentType::Byte:
        bytesRead += sizeof(int8_t);
        return Math::normtofloat(*((int8_t*)data));
    case Gltf::Accessor::ComponentType::UnsignedByte:
        bytesRead += sizeof(uint8_t);
        return Math::normtofloat(*((uint8_t*)data));
    case Gltf::Accessor::ComponentType::Short:
        bytesRead += sizeof(int16_t);
        return Math::normtofloat(*((int16_t*)data));
    case Gltf::Accessor::ComponentType::UnsignedShort:
        bytesRead += sizeof(uint16_t);
        return Math::normtofloat(*((uint16_t*)data));
    case Gltf::Accessor::ComponentType::UnsignedInt: // это не поддерживается анимацией gltf
    default:
        n_error("ОШИБКА: Неверный GLTF!");
        return 0.0f;
    }
}

//------------------------------------------------------------------------------
/**
*/
NglTFScene::NglTFScene()
{
}

//------------------------------------------------------------------------------
/**
*/
NglTFScene::~NglTFScene()
{
}

//------------------------------------------------------------------------------
/**
*/
void
NglTFScene::Setup(Gltf::Document* scene
                  , const ToolkitUtil::ExportFlags& exportFlags
                  , float scale)
{
    n_assert(scene);

    // установить настройки экспорта
    this->flags = exportFlags;

    if (scene->scene == -1)
    {
        // if omitted, we can pick any scene, let's use 0
        scene->scene = 0;
    }
    // установить масштаб
    SceneScale = scale;
    AdjustedScale = 1.0f / scale;

    size_t nodeCount = scene->nodes.Size();
    this->nodes.Reserve((SizeT)nodeCount);

    // Пройдитесь по всем узлам и добавьте их в наш поиск.
    Util::Dictionary<Gltf::Node*, SceneNode*> nodeLookup;
    const int numRootNodes = scene->scenes[scene->scene].nodes.Size();
    
    // Пройдитесь по иерархии узлов и подсчитайте, сколько SceneNodes нам понадобится.
    int meshCounter = 0;
    for (int i = 0; i < numRootNodes; i++)
    {
        Gltf::Node* node = &scene->nodes[scene->scenes[scene->scene].nodes[i]];

        std::function<void(Gltf::Node*)> counter = [&](Gltf::Node* node)
        {
            if (node->mesh != -1)
            {
                // Сетки glTF могут содержать несколько примитивов, каждый из которых должен быть уникальным SceneNode.
                // из-за того, что они могут различаться по расположению вершин
                Gltf::Mesh* mesh = &scene->meshes[node->mesh];
                meshCounter += mesh->primitives.Size();
            }

            for (int i = 0; i < node->children.Size(); i++)
            {
                counter(&scene->nodes[node->children[i]]);
            }
        };

        counter(node);
    }
    this->meshes.Resize(meshCounter);
    // IMPORTANT: We need to preallocate all the nodes needed so that the parent/child references (pointers) don't get invalidated if the array grows.
    this->nodes.Reserve(meshCounter + scene->nodes.Size());

    for (size_t i = 0; i < numRootNodes; i++)
    {
        Gltf::Node* rootNode = &(scene->nodes[scene->scenes[scene->scene].nodes[i]]);
        ParseNodeHierarchy(scene, rootNode, nullptr, nodeLookup, this->nodes);
    }
    
    // Узлы соединений в настоящее время являются узлами преобразования, поскольку GLTF не помечает их как соединения.
    // Прежде чем продолжить, нам нужно преобразовать их в объединенные узлы.

    for (size_t i = 0; i < scene->skins.Size(); i++)
    {
        Gltf::Skin const& skin = scene->skins[i];
        n_assert2(skin.inverseBindMatrices != -1, "Поддержка gltfs без матриц обратной привязки пока не реализована!\n");

        // Аксессор, на который ссылается inverseBindMatrices, ДОЛЖЕН иметь компоненты с плавающей запятой
        // Тип «MAT4». Число элементов метода доступа, на которые ссылаются inverseBindMatrices, ДОЛЖНО быть больше или
        // равно числу элементов соединения. Четвертая строка каждой матрицы ДОЛЖНА быть установлена ​​на [0,0, 0,0, 0,0, 1,0].
        Gltf::Accessor const& bindAccessor = scene->accessors[skin.inverseBindMatrices];
        Gltf::BufferView const& bindBufferView = scene->bufferViews[bindAccessor.bufferView];
        Gltf::Buffer const& bindBuffer = scene->buffers[bindBufferView.buffer];
        Math::mat4 const* const buf = (Math::mat4*)((byte*)bindBuffer.data.GetPtr() + bindBufferView.byteOffset + bindAccessor.byteOffset);

        
        for (uint64_t jointIndex = 0; jointIndex < skin.joints.Size(); jointIndex++)
        {
            Math::mat4 const& inverseBindMatrix = buf[jointIndex];
            Gltf::Node* gltfJointNode = &scene->nodes[skin.joints[jointIndex]];
            SceneNode* node = nodeLookup[gltfJointNode];
            node->type = SceneNode::NodeType::Joint;
            node->anim.animIndex = 0; // по умолчанию анимация 0
            node->skeleton.bindMatrix = inverseBindMatrix;
            node->skeleton.jointIndex = jointIndex;
            node->skeleton.skeletonIndex = i;
        }
    }

    // Find the root joints. Need to do this here, since we sometimes need to check if the parent is a joint or not
    // чего мы не можем сделать в приведенном выше цикле, поскольку все соединения еще не отмечены как соединения.
    for (size_t i = 0; i < scene->skins.Size(); i++)
    {
        Gltf::Skin const& skin = scene->skins[i];
        for (auto jointNodeIndex : skin.joints)
        {
            Gltf::Node* gltfJointNode = &scene->nodes[jointNodeIndex];
            SceneNode* node = nodeLookup[gltfJointNode];
            node->skeleton.isSkeletonRoot = node->base.parent == nullptr || skin.skeleton == jointNodeIndex ||
                                            node->base.parent->type != SceneNode::NodeType::Joint;
        }
    }

    // Настройка родительского индекса суставов
    for (IndexT i = 0; i < this->nodes.Size(); i++)
    {
        if (this->nodes[i].skeleton.isSkeletonRoot)
        {
            std::function<void(SceneNode*)> convertFunc = [&](SceneNode* node)
            {
                if (node->base.parent != nullptr)
                    node->skeleton.parentIndex = node->base.parent->skeleton.jointIndex;

                for (auto& child : node->base.children)
                    convertFunc(child);
            };
            convertFunc(&this->nodes[i]);
        }
    }

    // вычислить глобальные преобразования узлов для всех узлов, которые не являются соединениями
    for (auto& node : this->nodes)
    {
        if (node.base.parent == nullptr && node.type != SceneNode::NodeType::Joint)
            node.CalculateGlobalTransforms();
    }

    this->ExtractSkeletons();

    Timing::Timer timer;

    // Извлечение сеток и примитивов

    // Создайте сопоставление между сетками и примитивами.
    IndexT basePrimitiveIndex = 0;
    std::function<void(Gltf::Node*, SceneNode*, IndexT)> nodeSetup =
        [&](Gltf::Node* gltfNode, SceneNode* parent, IndexT nodeIndex)
        {
            if (!nodeLookup.Contains(gltfNode))
                return;
            // Если мы сталкиваемся с сеткой, нам нужно создать один SceneNode для каждого примитива сетки.
            SceneNode* node = nodeLookup[gltfNode];
            if (gltfNode->mesh != -1)
            {
                Util::Array<SceneNode*> primitiveNodes;
                const Gltf::Mesh& mesh = scene->meshes[gltfNode->mesh];
                primitiveNodes.Reserve(mesh.primitives.Size());

                // Создайте по одному узлу сетки для каждого примитивного узла и сделайте его родительским для узла преобразования выше.
                for (IndexT i = 0; i < mesh.primitives.Size(); i++)
                {
                    SceneNode* primitiveNode = &this->nodes.Emplace();
                    primitiveNode->Setup(SceneNode::NodeType::Mesh);
                    NglTFNode::Setup(gltfNode, primitiveNode, node);
                    primitiveNode->base.name = Util::String::Sprintf("%s:%d", node->base.name.AsCharPtr(), i);

                    // Уничтожить преобразования примитива, поскольку он принадлежит родительскому преобразованию
                    primitiveNode->base.rotation = Math::quat();
                    primitiveNode->base.scale = Math::vec3(1);
                    primitiveNode->base.translation = Math::vec3(0);
                    
                    primitiveNodes.Append(primitiveNode);

                    if (gltfNode->skin != -1)
                    {
                        primitiveNode->mesh.meshFlags = (MeshFlags)(primitiveNode->mesh.meshFlags | MeshFlags::HasSkin);
                        primitiveNode->base.isSkin = true;
                        primitiveNode->skeleton.skeletonIndex = gltfNode->skin; // ТОДО: это правильно?

                        // При необходимости пометьте сетчатый материал как очищенный.
                        Util::String& materialExtras = scene->materials[scene->meshes[gltfNode->mesh].primitives[i].material].extras;
                        // Предполагая, что на данный момент это пусто. Если нет, нам, вероятно, нужно прочитать его как JSON и добавить дополнительные метаданные как таковые.
                        // Информация о скинах используется в ngltfmaterialextractor.
                        n_assert(materialExtras.IsEmpty());
                        materialExtras = "n_skinned";
                    }
                }
                NglTFMesh::Setup(
                    this->meshes,
                    &scene->meshes[gltfNode->mesh],
                    scene,
                    flags,
                    basePrimitiveIndex,
                    gltfNode->mesh,
                    primitiveNodes.Begin()
                );
                basePrimitiveIndex += mesh.primitives.Size();
            }
            
            // Рекурсия вниз к детям
            for (int i = 0; i < gltfNode->children.Size(); i++)
            {
                nodeSetup(&scene->nodes[gltfNode->children[i]], node, gltfNode->children[i]);
            }
        };

    // Снова переберите узлы и настройте сетку и примитивные узлы.
    for (int i = 0; i < numRootNodes; i++)
    {
        Gltf::Node* gltfNode = &scene->nodes[scene->scenes[scene->scene].nodes[i]];
        nodeSetup(gltfNode, nullptr, scene->scenes[scene->scene].nodes[i]);
    }

    // загрузите анимации и добавьте их в список анимаций сцены
    // TODO: Переместить в отдельную функцию
    if (!scene->animations.IsEmpty())
    {
        timer.Reset();
        timer.Start();
        AnimBuilder& animBuilder = this->animations.Emplace();
        for (auto const& animation : scene->animations)
        {
            if (animation.channels.IsEmpty())
                continue;

            AnimBuilderClip& clip = animBuilder.clips.Emplace();
            clip.SetName(animation.name);

            clip.firstCurveOffset = animBuilder.curves.Size();
            clip.numCurves = animation.channels.Size();

            for (auto const& channel : animation.channels)
            {
                AnimBuilderCurve& curve = animBuilder.curves.Emplace();
                Gltf::Animation::Sampler const& sampler = animation.samplers[channel.sampler];
                Gltf::Accessor const& curveAccessor = scene->accessors[sampler.output];
                Gltf::Accessor const& timestampAccessor = scene->accessors[sampler.input];

                int jointNode = channel.target.node;

                curve.curveType = GltfPathToCurveType(channel.target.path);
                curve.firstKeyOffset = animBuilder.keys.size();
                curve.firstTimeOffset = animBuilder.keyTimes.size();

                if (curve.curveType == CoreAnimation::CurveType::Code::Translation)
                {
                    n_assert(curveAccessor.type == Gltf::Accessor::Type::Vec3);
                }
                else if (curve.curveType == CoreAnimation::CurveType::Code::Rotation)
                {
                    // quaternion
                    n_assert(curveAccessor.type == Gltf::Accessor::Type::Vec4);
                }
                else if (curve.curveType == CoreAnimation::CurveType::Code::Scale)
                {
                    n_assert(curveAccessor.type == Gltf::Accessor::Type::Vec3);
                }
                else if (curve.curveType == CoreAnimation::CurveType::Code::Float4)
                {
                    n_assert(curveAccessor.type == Gltf::Accessor::Type::Vec4);
                }
                else
                {
                    n_error("Неподдерживаемый тип кривой!");
                    return;
                }

                // заполните ключи и ключевые времена

                // GLTF не определяет логику цикла.
                curve.preInfinityType = CoreAnimation::InfinityType::Code::Cycle;
                curve.postInfinityType = CoreAnimation::InfinityType::Code::Cycle;

                curve.numKeys = curveAccessor.count;

                Gltf::BufferView const& curveBufferView = scene->bufferViews[curveAccessor.bufferView];
                Util::Blob const& curveData = scene->buffers[curveBufferView.buffer].data;
                int const curveBufferOffset = curveAccessor.byteOffset + curveBufferView.byteOffset;
                byte const* const curveBuf = (byte*)curveData.GetPtr() + curveBufferOffset;

                Gltf::BufferView const& timestampBufferView = scene->bufferViews[timestampAccessor.bufferView];
                Util::Blob const& timestampData = scene->buffers[timestampBufferView.buffer].data;
                int const timestampBufferOffset = timestampAccessor.byteOffset + timestampBufferView.byteOffset;
                byte const* const timeBuf = (byte*)timestampData.GetPtr() + timestampBufferOffset;

                // Текущее предположение состоит в том, что временных меток столько же, сколько и ключевых кадров. если это утверждение не сработает, мы могли бы сделать неправильное предположение
                n_assert(curveAccessor.count == timestampAccessor.count);
                n_assert(timestampAccessor.type == Gltf::Accessor::Type::Scalar);

                uint64_t curveBufByteOffset = 0;
                uint64_t timestampBufByteOffset = 0;
                for (SizeT i = 0; i < curveAccessor.count; i++)
                {
                    animBuilder.keyTimes.Append(
                        ReadCurveValue(timeBuf + timestampBufByteOffset, timestampAccessor.componentType, timestampBufByteOffset) * 1000.0f
                    );

                    switch (curveAccessor.type)
                    {
                    case Gltf::Accessor::Type::Scalar:
                        animBuilder.keys.Append(ReadCurveValue(curveBuf + curveBufByteOffset, curveAccessor.componentType, curveBufByteOffset));
                        break;
                    case Gltf::Accessor::Type::Vec2:
                        animBuilder.keys.Append(ReadCurveValue(curveBuf + curveBufByteOffset, curveAccessor.componentType, curveBufByteOffset));
                        animBuilder.keys.Append(ReadCurveValue(curveBuf + curveBufByteOffset, curveAccessor.componentType, curveBufByteOffset));
                        break;
                    case Gltf::Accessor::Type::Vec3:
                        animBuilder.keys.Append(ReadCurveValue(curveBuf + curveBufByteOffset, curveAccessor.componentType, curveBufByteOffset));
                        animBuilder.keys.Append(ReadCurveValue(curveBuf + curveBufByteOffset, curveAccessor.componentType, curveBufByteOffset));
                        animBuilder.keys.Append(ReadCurveValue(curveBuf + curveBufByteOffset, curveAccessor.componentType, curveBufByteOffset));
                        break;
                    case Gltf::Accessor::Type::Vec4:
                        animBuilder.keys.Append(ReadCurveValue(curveBuf + curveBufByteOffset, curveAccessor.componentType, curveBufByteOffset));
                        animBuilder.keys.Append(ReadCurveValue(curveBuf + curveBufByteOffset, curveAccessor.componentType, curveBufByteOffset));
                        animBuilder.keys.Append(ReadCurveValue(curveBuf + curveBufByteOffset, curveAccessor.componentType, curveBufByteOffset));
                        animBuilder.keys.Append(ReadCurveValue(curveBuf + curveBufByteOffset, curveAccessor.componentType, curveBufByteOffset));
                        break;
                    default:
                        break;
                    }
                }
                // TODO: Дважды проверьте, что это правильно.
                clip.duration = animBuilder.keyTimes.Back() - animBuilder.keyTimes[curve.firstTimeOffset];
            }
        }
        timer.Stop();
        n_printf("    [glTF – чтение анимации (%d, %d мс)]\n", scene->animations.Size(), timer.GetTime() * 1000);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
NglTFScene::ParseNodeHierarchy(
    Gltf::Document* scene,
    Gltf::Node* gltfNode,
    SceneNode* parent,
    Util::Dictionary<Gltf::Node*, SceneNode*>& lookup,
    Util::Array<SceneNode>& nodes
)
{
    SceneNode& node = nodes.Emplace();
    n_assert(!lookup.Contains(gltfNode));
    lookup.Add(gltfNode, &node);
    
    // Временно установите все узлы в качестве преобразований.
    // Позже мы преобразуем некоторые из них в суставы, так как мы не можем
    // сделайте это здесь, поскольку GLTF не всегда отслеживает это.
    // Мы также сгенерируем дополнительные узлы, которые будут действовать как сетки, поскольку GLTF
    // имеет поддержку различных макетов вершин для примитивов внутри одного и того же.
    // сетка, а мы нет. Каждый примитив GLTF становится сеткой Nebula.
    node.Setup(SceneNode::NodeType::Transform);
    NglTFNode::Setup(gltfNode, &node, parent);

    node.base.name = gltfNode->name;
    if (node.base.name.IsEmpty())
    {
        node.base.name = UniqueString::New("unnamed");
    }

    size_t numChildren = gltfNode->children.Size();
    for (size_t i = 0; i < numChildren; i++)
    {
        Gltf::Node* child = &scene->nodes[gltfNode->children[i]];
        ParseNodeHierarchy(scene, child, &node, lookup, nodes);
    }
}

} // пространство имен ToolkitUtil
