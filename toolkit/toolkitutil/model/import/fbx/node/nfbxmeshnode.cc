//------------------------------------------------------------------------------
//  fbxmeshnode.cc
//  (C) 2012-2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "nfbxnode.h"
#include "nfbxmeshnode.h"
#include "model/meshutil/meshbuildervertex.h"
#include "nfbxscene.h"
#include "ufbx/ufbx.h"

namespace ToolkitUtil
{

using namespace Math;
using namespace Util;
using namespace CoreAnimation;
using namespace ToolkitUtil;

uint MeshCounter = 0;

//------------------------------------------------------------------------------
/**
*/
void 
NFbxMeshNode::Setup(
    SceneNode* node
    , SceneNode* parent
    , ufbx_node* fbxNode
)
{
    NFbxNode::Setup(node, parent, fbxNode);
}

//------------------------------------------------------------------------------
/**
*/
void 
NFbxMeshNode::ExtractMesh(
    SceneNode* node
    , Util::Array<MeshBuilder>& meshes
    , const Util::Dictionary<ufbx_node*, SceneNode*>& nodeLookup
    , const ToolkitUtil::ExportFlags flags
)
{
    node->mesh.meshIndex = meshes.Size();

    // Добавьте один примитив сетки для узлов FBX.
    MeshBuilder& mesh = meshes.Emplace();
    
    if (node->fbx.node->materials.count)
    {
        ufbx_material* mat = node->fbx.node->materials[0];
        node->mesh.material = Util::String(mat->name.data, mat->name.length);
    }

    ufbx_mesh* fbxMesh = node->fbx.node->mesh;

    // создать маску
    uint meshMask = ToolkitUtil::NoMeshFlags;
    if (fbxMesh->skin_deformers.count > 0)
    {
        meshMask |= ToolkitUtil::HasSkin;
    }

    // отметьте как физику, если родительское имя — физика, обратите внимание, что это будет работать только с прямым родителем, называемым физикой!
    SceneNode* parent = node->base.parent;
    if (parent != nullptr && parent->base.isPhysics)
    {
        node->mesh.material = "physics";
    }

    if (fbxMesh->uv_sets.count > 1)
        meshMask |= HasMultipleUVs;

    if (fbxMesh->vertex_color.values.count > 0)
        meshMask |= HasVertexColors;

    if (fbxMesh->vertex_tangent.values.count > 0)
        meshMask |= HasTangents;

    if (fbxMesh->vertex_normal.values.count > 0)
        meshMask |= HasNormals;

    // Провоцировать генерацию LOD-групп
    ufbx_lod_group* lodGroup = nullptr;
    if (node->fbx.node->parent != nullptr)
    {
        lodGroup = ufbx_as_lod_group(&node->fbx.node->parent->element);
    }

    /* TODO: Generate tangents?
    if (AllBits(meshMask, HasNormals))
    {
        bool res = fbxMesh->GenerateTangentsData(0, true);
        n_assert(res);
        meshMask |= HasTangents;
    }

    */


    // установить маску
    node->mesh.meshFlags = (ToolkitUtil::MeshFlags)meshMask;

    // Приступаем к извлечению сетки
    size_t vertexCount = fbxMesh->vertex_position.values.count;
    size_t polyCount = fbxMesh->faces.count;
    size_t uvCount = fbxMesh->uv_sets.count;
    size_t normalCount = fbxMesh->vertex_normal.indices.count;
    size_t tangentCount = fbxMesh->vertex_tangent.indices.count;
    size_t colorCount = fbxMesh->vertex_color.indices.count;

    // это здесь просто для того, чтобы сообщить, если художник забыл применить набор UV или у сетки нет нормалей перед импортом.
    if (vertexCount > 0)
    {
        n_assert_msg(uvCount > 0, "Вам нужен хотя бы один UV-канал, иначе шейдер будет неприменим!");
        n_assert_msg(normalCount > 0, "Вам нужен хотя бы один набор нормалей, иначе шейдер будет неприменим!");
    }

    // получить масштаб
    uint groupId = MeshCounter++;
    node->mesh.groupId = groupId;

    MeshBuilderVertex::ComponentMask componentMask = 0x0;
    Util::FixedArray<Math::vec3> controlPoints;
    controlPoints.Resize((SizeT)vertexCount);

    mesh.NewMesh(controlPoints.Size(), (SizeT)fbxMesh->num_triangles);
    mesh.SetPrimitiveTopology(CoreGraphics::PrimitiveTopology::TriangleList);
    for (IndexT i = 0; i < controlPoints.Size(); i++)
    {
        ufbx_vec3 v = fbxMesh->vertex_position.values[i];
        controlPoints[i] = FbxToMath(v);
        componentMask |= MeshBuilderVertex::Position;
    }

    Util::FixedArray<Math::vec4> skinWeights;
    Util::FixedArray<Math::uint4> skinIndices;

    // if we export using a skeletal model, we always need a skin, so if the mesh isn't skinned, we do it manually
    if (node->mesh.meshFlags & HasSkin)
    {
        // извлечь кожу
        skinWeights.Resize(controlPoints.Size());
        skinIndices.Resize(controlPoints.Size());
        ExtractSkin(node, skinIndices, skinWeights, nodeLookup, fbxMesh);
        componentMask |= MeshBuilderVertex::SkinWeights | MeshBuilderVertex::SkinIndices;
    }
    else if (flags & ToolkitUtil::CalcRigidSkin)
    { 
        // при необходимости создать жесткую кожу
        skinWeights.Resize(controlPoints.Size());
        skinIndices.Resize(controlPoints.Size());
        GenerateRigidSkin(node, skinIndices, skinWeights, (uint&)node->mesh.meshFlags);
        componentMask |= MeshBuilderVertex::SkinWeights | MeshBuilderVertex::SkinIndices;
    }

    ufbx_vertex_vec2* uv0Elements = nullptr, *uv1Elements = nullptr;
    ufbx_vertex_vec4* colorElements = nullptr;
    ufbx_vertex_vec3* normalElements = nullptr, *tangentElements = nullptr;

    // Преобразуйте все сетки, чтобы сопоставить их индексы с однократным значением для каждой вершины.
    if (uvCount > 0)
        uv0Elements = &fbxMesh->uv_sets[0].vertex_uv;
    if (uvCount > 1)
        uv1Elements = &fbxMesh->uv_sets[1].vertex_uv;
    if (colorCount > 0)
        colorElements = &fbxMesh->vertex_color;
    if (normalCount > 0)
        normalElements = &fbxMesh->vertex_normal;
    if (tangentCount > 0)
        tangentElements = &fbxMesh->vertex_tangent;

    // настроить треугольники
    int vertexId = 0; 
    for (int polygonIndex = 0; polygonIndex < polyCount; polygonIndex++)
    {
        const ufbx_face& face = fbxMesh->faces[polygonIndex];
        size_t requiredIndices = (face.num_indices - 2) * 3;
        uint32_t* indices = ArrayAllocStack<uint32_t>(requiredIndices);
        uint status = ufbx_triangulate_face(indices, requiredIndices, fbxMesh, face);
        n_assert_fmt(status != 0, "Не удалось провести триангуляцию на полигоне %d.", polygonIndex);
        for (int tri = 0; tri < requiredIndices / 3; tri++)
        {
            MeshBuilderTriangle meshTriangle;
            for (int triangleVertexIndex = 0; triangleVertexIndex < 3; triangleVertexIndex++)
            {
                // мы хотим сместить индекс вершины с текущим размером сетки
                int baseIndex = indices[tri * 3 + triangleVertexIndex];
                int polygonVertex = fbxMesh->vertex_position.indices[baseIndex];
                MeshBuilderVertex meshVertex;
                meshVertex.SetPosition(Math::vec4(controlPoints[polygonVertex], 1));

                // извлечь UV из 0-го канала
                Math::vec2 uv = Extract(uv0Elements, baseIndex);
                meshVertex.SetUv(uv);
                componentMask |= MeshBuilderVertex::Uvs;

                if (AllBits(componentMask, MeshBuilderVertex::SkinWeights | MeshBuilderVertex::SkinIndices))
                {
                    meshVertex.SetSkinWeights(skinWeights[polygonVertex]);
                    meshVertex.SetSkinIndices(skinIndices[polygonVertex]);
                }

                // извлечь многослойные UV-развертки или обычные UV-развертки, если мы не импортируем их как многослойные
                if (AllBits(flags, ToolkitUtil::ImportSecondaryUVs))
                {
                    if (AllBits(node->mesh.meshFlags, HasMultipleUVs))
                    {
                        // просто извлеките два ультрафиолета
                        Math::vec2 uv2 = Extract(uv1Elements, baseIndex);
                        meshVertex.SetSecondaryUv(uv2);
                    }
                    else
                    {
                        // if mesh has none, flood to 0
                        meshVertex.SetSecondaryUv(Math::vec2(0, 0));
                    }
                    componentMask |= MeshBuilderVertex::SecondUv;
                }

                // извлечь цвета, если потребуется
                if (AllBits(flags, ToolkitUtil::ImportColors))
                {
                    if (AllBits(node->mesh.meshFlags, HasVertexColors))
                    {
                        // извлекать цвета
                        Math::vec4 color = Extract(colorElements, baseIndex);
                        meshVertex.SetColor(color);
                    }
                    else
                    {
                        // установите цвет вершин белым, если цвета вершин не могут быть извлечены из FBX
                        meshVertex.SetColor(Math::vec4(1));
                    }
                    componentMask |= MeshBuilderVertex::Color;
                }

                Math::vec3 normal = Extract(normalElements, baseIndex);
                meshVertex.SetNormal(normal);
                componentMask |= MeshBuilderVertex::Normals;

                if (AllBits(node->mesh.meshFlags, HasTangents))
                {
                    Math::vec3 tangent = Extract(tangentElements, baseIndex);
                    meshVertex.SetTangent(tangent);
                    meshVertex.SetSign(1.0f);
                    componentMask |= MeshBuilderVertex::Tangents;
                }
                else
                {
                    meshVertex.SetTangent({ 0, 0, 0 });
                    meshVertex.SetSign(0);
                }

                mesh.AddVertex(meshVertex);
                meshTriangle.SetVertexIndex(triangleVertexIndex, vertexId);

                vertexId++;
            }
            mesh.AddTriangle(meshTriangle);
        }
        ArrayFreeStack(requiredIndices, indices);
    }

    if (AllBits(node->mesh.meshFlags, HasNormals))
    {
        mesh.CalculateTangents();
        componentMask |= MeshBuilderVertex::Tangents;
    }

    // перевернуть UV, если отмечено
    if (AllBits(flags, ToolkitUtil::FlipUVs))
    {
        mesh.FlipUvs();
    }

    // вычислить ограничивающую рамку
    node->base.boundingBox = mesh.ComputeBoundingBox();
    mesh.SetComponents(componentMask);

    // удалить лишние вершины
    mesh.Cleanup(nullptr);

    // Рассчитать Лоды
    if (lodGroup != nullptr)
    {
        node->mesh.lodIndex = node->base.parent->base.children.FindIndex(node);
        if (lodGroup->lod_levels.count > node->mesh.lodIndex - 1)
            node->mesh.minLodDistance = lodGroup->lod_levels[node->mesh.lodIndex - 1].distance;
        else
            node->mesh.minLodDistance = 0;
        if (lodGroup->lod_levels.count > node->mesh.lodIndex)
            node->mesh.maxLodDistance = lodGroup->lod_levels[node->mesh.lodIndex].distance;
        else
            node->mesh.maxLodDistance = 0;
    }
}

//------------------------------------------------------------------------------
/**
    Хм, может быть, есть способ проверить, если у нас более 255 индексов, тогда нам следует переключиться на использование индексов Joint в качестве полных uint?
*/
void 
NFbxMeshNode::ExtractSkin(SceneNode* node, Util::FixedArray<Math::uint4>& indices, Util::FixedArray<Math::vec4>& weights, const Util::Dictionary<ufbx_node*, SceneNode*>& nodeLookup, ufbx_mesh* fbxMesh)
{
    size_t vertexCount = fbxMesh->num_vertices;
    size_t skinCount = fbxMesh->skin_deformers.count;
    
    Util::FixedArray<Util::Array<std::tuple<int, float>>> keyWeightPairs((SizeT)vertexCount);
    ufbx_matrix geometricTransform = ufbx_transform_to_matrix(&node->fbx.node->geometry_transform);
    for (int skinIndex = 0; skinIndex < skinCount; skinIndex++)
    {
        ufbx_skin_deformer* skin = fbxMesh->skin_deformers[skinIndex];
        size_t clusterCount = skin->clusters.count;
        node->base.isSkin = true;

        for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
        {
            ufbx_skin_cluster* cluster = skin->clusters[clusterIndex];
            ufbx_node* joint = cluster->bone_node;
            SceneNode* jointNode = nodeLookup[joint];

            n_assert(jointNode != nullptr);
            n_assert(jointNode->skeleton.bindMatrix == Math::mat4()); 
            n_assert(jointNode->type == SceneNode::NodeType::Joint);
            jointNode->skeleton.bindMatrix = FbxToMath(cluster->geometry_to_bone);

            size_t clusterVertexIndexCount = cluster->vertices.count;
            for (int vertexIndex = 0; vertexIndex < clusterVertexIndexCount; vertexIndex++)
            {
                int vertex = cluster->vertices[vertexIndex]; 
                float weight = cluster->weights[vertexIndex];
                if (weight > 0)
                    keyWeightPairs[vertex].Append(std::make_tuple(jointNode->skeleton.jointIndex, weight));
            }
        }
    }

    // наконец, пройдите через каждую вершину и установите их совместные индексы и веса
    for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
    {
        Util::Array<std::tuple<int, float>>& weightPairs = keyWeightPairs[vertexIndex];
        weightPairs.SortWithFunc([](const std::tuple<int, float>& lhs, const std::tuple<int, float>& rhs)->bool
        {
            const auto& [i0, w0] = lhs;
            const auto& [i1, w1] = rhs;
            return w0 > w1;
        });

        const auto& [i0, w0] = weightPairs.Size() > 0 ? weightPairs[0] : std::make_tuple(0, 0.0f);
        const auto& [i1, w1] = weightPairs.Size() > 1 ? weightPairs[1] : std::make_tuple(0, 0.0f);
        const auto& [i2, w2] = weightPairs.Size() > 2 ? weightPairs[2] : std::make_tuple(0, 0.0f);
        const auto& [i3, w3] = weightPairs.Size() > 3 ? weightPairs[3] : std::make_tuple(0, 0.0f);

        float normalizeFactor = 1.0f / (w0 + w1 + w2 + w3);

        // установить веса
        weights[vertexIndex] = vec4(w0 * normalizeFactor, w1 * normalizeFactor, w2 * normalizeFactor, w3 * normalizeFactor);
        indices[vertexIndex] = uint4{ (uint)i0, (uint)i1, (uint)i2, (uint)i3 };
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
NFbxMeshNode::GenerateRigidSkin(SceneNode* node, Util::FixedArray<Math::uint4>& indices, Util::FixedArray<Math::vec4>& weights, uint& meshFlags)
{
    if (!AllBits(meshFlags, ToolkitUtil::HasSkin))
    {
        // родительское соединение, если оно существует, в противном случае просто родительское соединение
        SceneNode* parent = node->base.parent;
        if (parent != nullptr && parent->type == SceneNode::NodeType::Joint)
        {
            IndexT parentJointIndex = parent->skeleton.jointIndex;

            int vertCount = indices.Size();
            IndexT vertIndex;
            for (vertIndex = 0; vertIndex < vertCount; vertIndex++)
            {
                // for each vertex, set a rigid skin by using a weight of 1 and the index of the parent node
                indices[vertIndex] = uint4{ (uint)parentJointIndex, 0, 0, 0 };
                weights[vertIndex] = vec4(1, 0, 0, 0);
            }
        }
        else
        {
            int vertCount = indices.Size();
            IndexT vertIndex;
            for (vertIndex = 0; vertIndex < vertCount; vertIndex++)
            {
                // for each vertex, set a rigid skin by using a weight of 1 and the index of the parent node
                indices[vertIndex] = uint4{ 0, 0, 0, 0 };
                weights[vertIndex] = vec4(1, 0, 0, 0);
            }
        }
        
        // установить флаг скина
        meshFlags |= ToolkitUtil::HasSkin;
    }
}

} // пространство имен ToolkitUtil
