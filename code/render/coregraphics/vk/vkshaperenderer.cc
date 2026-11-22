//------------------------------------------------------------------------------
// vkshaperenderer.cc
// (C) 2016–2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "vkshaperenderer.h"
#include "coregraphics/mesh.h"
#include "coregraphics/vertexcomponent.h"
#include "coregraphics/shaderserver.h"
#include "coregraphics/meshresource.h"
#include "resources/resourceid.h"
#include "resources/resourceserver.h"
#include "frame/framesubgraph.h"

#include "frame/default.h"

using namespace Base;
using namespace Threading;
using namespace Math;
using namespace CoreGraphics;
using namespace Threading;
using namespace Resources;
namespace Vulkan
{

__ImplementClass(Vulkan::VkShapeRenderer, 'VKSR', Base::ShapeRendererBase);
//------------------------------------------------------------------------------
/**
*/
VkShapeRenderer::VkShapeRenderer()  
    : ibos{ InvalidBufferId }
    , vbos{ InvalidBufferId }
    , vertexBufferPtr(nullptr)
    , indexBufferPtr(nullptr)
    , indexBufferActiveIndex()
    , vertexBufferActiveIndex()
    , vertexBufferCapacity(0)
    , indexBufferCapacity(0)
    , vertexBufferOffset(0)
    , indexBufferOffset(0)

{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
VkShapeRenderer::~VkShapeRenderer()
{
    n_assert(!this->IsOpen());
}

//------------------------------------------------------------------------------
/**
*/
void
VkShapeRenderer::Open()
{
    n_assert(!this->IsOpen());

    ShapeRendererBase::Open();

    // создать экземпляр шейдера формы
    CoreGraphics::ShaderId shapesShader = CoreGraphics::ShaderGet("shd:system_shaders/shapes.fxb"_atm);
    this->shapeMeshResources.SetSize(CoreGraphics::RenderShape::NumShapeTypes);
    this->meshes.SetSize(CoreGraphics::RenderShape::NumShapeTypes);

    /*
            Коробка,
        Сфера,
        Цилиндр,
        Тор,
        Конус,
        Стрела,
    */
    const Util::String meshResourceNames[] = {
        "sysmsh:box.nvx"
        , "sysmsh:sphere.nvx"
        , "sysmsh:cylinder.nvx"
        , "sysmsh:torus.nvx"
        , "sysmsh:cone.nvx"
    };

    const RenderShape::Type types[] = {
        RenderShape::Box
        , RenderShape::Sphere
        , RenderShape::Cylinder
        , RenderShape::Torus
        , RenderShape::Cone
    };

    for (IndexT i = 0; i < RenderShape::Primitives - 1; i++)
    {
        this->shapeMeshResources[types[i]] = CreateResource(meshResourceNames[i], "render_system", nullptr, nullptr, true);
        this->meshes[types[i]] = MeshResourceGetMesh(this->shapeMeshResources[i], 0);
    }

    // поиск переменной шейдера ModelViewProjection
    this->model = ShaderGetConstantBinding(shapesShader, "ShapeModel");
    this->diffuseColor = ShaderGetConstantBinding(shapesShader, "ShapeColor");
    this->lineWidth = ShaderGetConstantBinding(shapesShader, "LineWidth");

    this->programs[ShaderTypes::Primitives] = ShaderGetProgram(shapesShader, CoreGraphics::ShaderFeatureMask("Primitives"));
    this->programs[ShaderTypes::PrimitivesNoDepth] = ShaderGetProgram(shapesShader, CoreGraphics::ShaderFeatureMask("Примитивы|NoDepth"));
    this->programs[ShaderTypes::PrimitivesWireframeTriangles] = ShaderGetProgram(shapesShader, CoreGraphics::ShaderFeatureMask("Примитивы|Каркас|Треугольники"));
    this->programs[ShaderTypes::PrimitivesWireframeLines] = ShaderGetProgram(shapesShader, CoreGraphics::ShaderFeatureMask("Примитивы|Каркас|Линии"));
    this->programs[ShaderTypes::Mesh] = ShaderGetProgram(shapesShader, CoreGraphics::ShaderFeatureMask("Mesh"));
    this->programs[ShaderTypes::MeshNoDepth] = ShaderGetProgram(shapesShader, CoreGraphics::ShaderFeatureMask("Сетка|Без глубины"));
    this->programs[ShaderTypes::MeshWireframe] = ShaderGetProgram(shapesShader, CoreGraphics::ShaderFeatureMask("Сетка|Каркас|Треугольники"));

    this->comps.Append(VertexComponent(VertexComponent::Position, VertexComponent::Float3));
    this->comps.Append(VertexComponent(VertexComponent::Color, VertexComponent::UByte4N));

    // также создайте дополнительный макет вершин на случай, если мы получим сетку, которая не соответствует нашему специальному макету.
    this->vertexLayout = CreateVertexLayout(VertexLayoutCreateInfo{ .name = "Средство визуализации формы вулкана"_atm, .comps = comps });

    FrameScript_default::RegisterSubgraph_DebugShapes_Pass([](const CoreGraphics::CmdBufferId cmdBuf, const Math::rectangle<int>& viewport, const IndexT frame, const IndexT bufferIndex)
    {
        auto thisPtr = static_cast<Vulkan::VkShapeRenderer*>(VkShapeRenderer::Instance());
        thisPtr->DrawShapes(cmdBuf);
        thisPtr->numIndicesThisFrame = 0;
        thisPtr->numVerticesThisFrame = 0;
    });
}

//------------------------------------------------------------------------------
/**
*/
void
VkShapeRenderer::Close()
{
    n_assert(this->IsOpen());

    // выгрузить сетки фигур
    DiscardResource(this->shapeMeshResources[RenderShape::Box]);
    DiscardResource(this->shapeMeshResources[RenderShape::Sphere]);
    DiscardResource(this->shapeMeshResources[RenderShape::Cylinder]);
    DiscardResource(this->shapeMeshResources[RenderShape::Torus]);
    DiscardResource(this->shapeMeshResources[RenderShape::Cone]);
    //DiscardResource(this->shapeMeshResources[RenderShape::Arrow]);
    this->shapeMeshResources.Clear();

    // выгрузить динамические буферы
    // мы не утверждаем здесь, поскольку буфер выделяется лениво
    CoreGraphics::BufferId activeVBId = this->vbos[this->vertexBufferActiveIndex];
    CoreGraphics::BufferId activeIBId = this->ibos[this->indexBufferActiveIndex];
    if (activeVBId != CoreGraphics::InvalidBufferId)
    {
        BufferUnmap(activeVBId);
    }
    if (activeIBId != CoreGraphics::InvalidBufferId)
    {
        BufferUnmap(activeIBId);
    }

    for (IndexT i = 0; i < MaxVertexIndexBuffers; i++)
    {
        if (this->vbos[i] != InvalidBufferId)
            DestroyBuffer(this->vbos[i]);
        if (this->ibos[i] != InvalidBufferId)
        DestroyBuffer(this->ibos[i]);
    }
    
    this->vertexBufferPtr = this->indexBufferPtr = nullptr;

    // вызвать родительский класс
    ShapeRendererBase::Close();
}

//------------------------------------------------------------------------------
/**
*/
void
VkShapeRenderer::DrawShapes(const CoreGraphics::CmdBufferId cmdBuf)
{
    n_assert(this->IsOpen());

    if (this->numIndicesThisFrame > this->indexBufferCapacity)
        this->GrowIndexBuffer();
    if (this->numVerticesThisFrame > this->vertexBufferCapacity)
        this->GrowVertexBuffer();

    for (int shaderType = 0; shaderType < ShaderTypes::NumShaders; shaderType++)
    {
        if (this->primitives[shaderType].Size() > 0)
        {
            IndexT i;
            for (i = 0; i < this->primitives[shaderType].Size(); i++)
            {
                const RenderShape& curShape = this->primitives[shaderType][i];
                if (curShape.GetShapeType() == RenderShape::Primitives)
                    this->DrawPrimitives(
                        curShape.GetModelTransform(),
                        curShape.GetTopology(),
                        curShape.GetNumVertices(),
                        curShape.GetVertexData(),
                        curShape.GetLineThickness());
                else if (curShape.GetShapeType() == RenderShape::IndexedPrimitives)
                    this->DrawIndexedPrimitives(
                        curShape.GetModelTransform(),
                        curShape.GetTopology(),
                        curShape.GetNumVertices(),
                        curShape.GetVertexData(),
                        curShape.GetNumIndices(),
                        curShape.GetIndexData(),
                        curShape.GetIndexType(),
                        curShape.GetLineThickness());
                else n_error("Тип фигуры %d не является примитивом!", curShape.GetShapeType());
            }

            // применить шейдер
            CoreGraphics::CmdSetShaderProgram(cmdBuf, this->programs[shaderType]);
            CoreGraphics::CmdSetVertexLayout(cmdBuf, this->vertexLayout);

            // очистить все буферизованные примитивы
            this->DrawBufferedIndexedPrimitives(cmdBuf);
            this->DrawBufferedPrimitives(cmdBuf);
        }
    }

    if (this->vertexBufferOffset > 0)
        CoreGraphics::BufferFlush(this->vbos[this->vertexBufferActiveIndex]);
    if (this->indexBufferOffset > 0)
        CoreGraphics::BufferFlush(this->ibos[this->indexBufferActiveIndex]);

    // сбросить индекс и счетчики примитивов
    this->indexBufferOffset = 0;
    this->vertexBufferOffset = 0;

    for (int shaderType = 0; shaderType < ShaderTypes::NumShaders; shaderType++)
    {
        if (this->shapes[shaderType].Size() > 0)
        {
            CoreGraphics::CmdSetShaderProgram(cmdBuf, this->programs[shaderType]);
            CoreGraphics::CmdSetGraphicsPipeline(cmdBuf);

            IndexT i;
            for (i = 0; i < this->shapes[shaderType].Size(); i++)
            {
                const RenderShape& curShape = this->shapes[shaderType][i];
                if (curShape.GetShapeType() == RenderShape::RenderMesh)
                    this->DrawMesh(cmdBuf, curShape.GetModelTransform(), curShape.GetMesh(), curShape.GetColor(), curShape.GetLineThickness());
                else
                    this->DrawSimpleShape(cmdBuf, curShape.GetModelTransform(), curShape.GetShapeType(), curShape.GetColor(), curShape.GetLineThickness());
            }
        }       
    }

    // четкие формы
    this->ClearShapes();
}

//------------------------------------------------------------------------------
/**
*/
void
VkShapeRenderer::DrawSimpleShape(const CoreGraphics::CmdBufferId cmdBuf, const Math::mat4& modelTransform, CoreGraphics::RenderShape::Type shapeType, const Math::vec4& color, float lineThickness)
{
    n_assert(this->shapeMeshResources[shapeType] != ResourceId::Invalid());
    n_assert(shapeType < RenderShape::NumShapeTypes);

    // resolve model-view-projection matrix and update shader
    CoreGraphics::CmdPushConstants(cmdBuf, CoreGraphics::GraphicsPipeline, this->model, sizeof(modelTransform), (byte*)&modelTransform);
    CoreGraphics::CmdPushConstants(cmdBuf, CoreGraphics::GraphicsPipeline, this->diffuseColor, sizeof(color), (byte*)&color);
    CoreGraphics::CmdPushConstants(cmdBuf, CoreGraphics::GraphicsPipeline, this->lineWidth, sizeof(float), &lineThickness);

    const MeshId mesh = this->meshes[shapeType];
    CoreGraphics::CmdSetVertexLayout(cmdBuf, CoreGraphics::MeshGetVertexLayout(mesh));
    CoreGraphics::CmdSetIndexBuffer(cmdBuf, MeshGetIndexType(mesh), CoreGraphics::GetIndexBuffer(), MeshGetIndexOffset(mesh));
    CoreGraphics::CmdSetVertexBuffer(cmdBuf, 0, CoreGraphics::GetVertexBuffer(), MeshGetVertexOffset(mesh, 0));
    const Util::Array<CoreGraphics::PrimitiveGroup>& groups = MeshGetPrimitiveGroups(mesh);

    IndexT i;
    for (i = 0; i < groups.Size(); i++)
    {
        // установить ресурсы
        

        // draw
        CoreGraphics::CmdDraw(cmdBuf, groups[i]);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
VkShapeRenderer::DrawMesh(const CoreGraphics::CmdBufferId cmdBuf, const Math::mat4& modelTransform, const CoreGraphics::MeshId mesh, const Math::vec4& color, float lineThickness)
{
    n_assert(mesh != InvalidMeshId);

    // resolve model-view-projection matrix and update shader
    CoreGraphics::CmdPushConstants(cmdBuf, CoreGraphics::GraphicsPipeline, this->model, sizeof(modelTransform), (byte*)&modelTransform);
    CoreGraphics::CmdPushConstants(cmdBuf, CoreGraphics::GraphicsPipeline, this->diffuseColor, sizeof(color), (byte*)&color);
    CoreGraphics::CmdPushConstants(cmdBuf, CoreGraphics::GraphicsPipeline, this->lineWidth, sizeof(float), &lineThickness);

    const Util::Array<CoreGraphics::PrimitiveGroup>& groups = MeshGetPrimitiveGroups(mesh);

    // установить ресурсы
    CoreGraphics::CmdSetVertexLayout(cmdBuf, MeshGetVertexLayout(mesh));
    CoreGraphics::CmdSetIndexBuffer(cmdBuf, MeshGetIndexType(mesh), MeshGetIndexBuffer(mesh), MeshGetIndexOffset(mesh));
    CoreGraphics::CmdSetVertexBuffer(cmdBuf, 0, MeshGetVertexBuffer(mesh, 0), MeshGetVertexOffset(mesh, 0));

    IndexT i;
    for (i = 0; i < groups.Size(); i++)
    {

        // draw
        CoreGraphics::CmdDraw(cmdBuf, groups[i]);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
VkShapeRenderer::DrawPrimitives(
    const Math::mat4& modelTransform
    , CoreGraphics::PrimitiveTopology::Code topology
    , SizeT numVertices
    , const void* vertices
    , float lineThickness)
{
    n_assert(0 != vertices);
    static const SizeT vertexWidth = sizeof(RenderShape::RenderShapeVertex);

    // вычислить количество вершин
    byte* verts = this->vertexBufferPtr;

    // разблокировать буфер, чтобы избежать потери данных
    memcpy(verts + this->vertexBufferOffset, vertices, numVertices * vertexWidth);

    // set vertex offset in primitive group
    CoreGraphics::PrimitiveGroup group;
    group.SetBaseVertex(this->vertexBufferOffset);
    group.SetNumVertices(numVertices);
    group.SetNumIndices(0);
    this->unindexed[topology].firstVertexOffset.Append(this->vertexBufferOffset);
    this->unindexed[topology].primitives.Append(group);
    this->unindexed[topology].transforms.Append(modelTransform);
    this->unindexed[topology].lineThicknesses.Append(lineThickness);
    this->vertexBufferOffset += numVertices * vertexWidth;
}

//------------------------------------------------------------------------------
/**
*/
void
VkShapeRenderer::DrawIndexedPrimitives(
    const Math::mat4& modelTransform
    , CoreGraphics::PrimitiveTopology::Code topology
    , SizeT numVertices
    , const void* vertices
    , SizeT numIndices
    , const void* indices
    , CoreGraphics::IndexType::Code indexType
    , float lineThickness)
{
    n_assert(0 != vertices);
    n_assert(0 != indices);
    static const SizeT vertexWidth = sizeof(RenderShape::RenderShapeVertex);

    // вычислить количество индексов и размер типа индекса
    SizeT indexSize = CoreGraphics::IndexType::SizeOf(indexType);

    byte* verts = this->vertexBufferPtr;
    byte* inds = this->indexBufferPtr;

    // разблокировать буфер и скопировать данные
    memcpy(verts + this->vertexBufferOffset, vertices, numVertices * vertexWidth);
    memcpy(inds + this->indexBufferOffset, indices, numIndices * indexSize);

    // set vertex offset in primitive group
    CoreGraphics::PrimitiveGroup group;
    group.SetBaseVertex(0);
    group.SetBaseIndex(0);
    group.SetNumIndices(numIndices);
    this->indexed[topology].firstIndexOffset.Append(this->indexBufferOffset);
    this->indexed[topology].firstVertexOffset.Append(this->vertexBufferOffset);
    this->indexed[topology].primitives.Append(group);
    this->indexed[topology].transforms.Append(modelTransform);
    this->indexed[topology].indexType.Append(indexType);
    this->indexed[topology].lineThicknesses.Append(lineThickness);
    this->vertexBufferOffset += numVertices * vertexWidth;
    this->indexBufferOffset += numIndices * indexSize;
}

//------------------------------------------------------------------------------
/**
*/
void
VkShapeRenderer::DrawBufferedPrimitives(const CoreGraphics::CmdBufferId cmdBuf)
{
    IndexT j;
    for (j = 1; j < CoreGraphics::PrimitiveTopology::NumTopologies; j++)
    {
        CoreGraphics::CmdSetPrimitiveTopology(cmdBuf, CoreGraphics::PrimitiveTopology::Code(j));
        CoreGraphics::CmdSetVertexLayout(cmdBuf, this->vertexLayout);
        CoreGraphics::CmdSetGraphicsPipeline(cmdBuf);

        IndexT i;
        for (i = 0; i < this->unindexed[j].primitives.Size(); i++)
        {
            const CoreGraphics::PrimitiveGroup& group = this->unindexed[j].primitives[i];
            const Math::mat4& modelTransform = this->unindexed[j].transforms[i];
            const Math::vec4 color(1);
            const float lineThickness = this->unindexed[j].lineThicknesses[i];
            const uint64_t& vertexOffset = this->unindexed[j].firstVertexOffset[i];

            CoreGraphics::CmdPushConstants(cmdBuf, CoreGraphics::GraphicsPipeline, this->model, sizeof(modelTransform), (byte*)&modelTransform);
            CoreGraphics::CmdPushConstants(cmdBuf, CoreGraphics::GraphicsPipeline, this->diffuseColor, sizeof(color), (byte*)&color);
            CoreGraphics::CmdPushConstants(cmdBuf, CoreGraphics::GraphicsPipeline, this->lineWidth, sizeof(float), &lineThickness);

            CoreGraphics::CmdSetVertexBuffer(cmdBuf, 0, this->vbos[this->vertexBufferActiveIndex], vertexOffset);
            CoreGraphics::CmdDraw(cmdBuf, group);
        }
        this->unindexed[j].primitives.Clear();
        this->unindexed[j].transforms.Clear();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
VkShapeRenderer::DrawBufferedIndexedPrimitives(const CoreGraphics::CmdBufferId cmdBuf)
{
    IndexT j;
    for (j = 1; j < CoreGraphics::PrimitiveTopology::NumTopologies; j++)
    {
        auto topology = CoreGraphics::PrimitiveTopology::Code(j);
        CoreGraphics::CmdSetPrimitiveTopology(cmdBuf, topology);
        CoreGraphics::CmdSetVertexLayout(cmdBuf, this->vertexLayout);
        CoreGraphics::CmdSetGraphicsPipeline(cmdBuf);

        IndexT i;
        for (i = 0; i < this->indexed[j].primitives.Size(); i++)
        {
            const CoreGraphics::PrimitiveGroup& group = this->indexed[j].primitives[i];
            const Math::mat4& modelTransform = this->indexed[j].transforms[i];
            const Math::vec4 color(1);
            const float lineThickness = this->unindexed[j].lineThicknesses[i];
            const uint64_t& vertexOffset = this->indexed[j].firstVertexOffset[i];
            const uint64_t& indexOffset = this->indexed[j].firstIndexOffset[i];

            CoreGraphics::CmdPushConstants(cmdBuf, CoreGraphics::GraphicsPipeline, this->model, sizeof(modelTransform), (byte*)&modelTransform);
            CoreGraphics::CmdPushConstants(cmdBuf, CoreGraphics::GraphicsPipeline, this->diffuseColor, sizeof(color), (byte*)&color);
            CoreGraphics::CmdPushConstants(cmdBuf, CoreGraphics::GraphicsPipeline, this->lineWidth, sizeof(float), &lineThickness);

            CoreGraphics::CmdSetIndexBuffer(cmdBuf, this->indexed[j].indexType[i], this->ibos[this->indexBufferActiveIndex], indexOffset);
            CoreGraphics::CmdSetVertexBuffer(cmdBuf, 0, this->vbos[this->vertexBufferActiveIndex], vertexOffset);

            CoreGraphics::CmdDraw(cmdBuf, group);
        }
        this->indexed[j].primitives.Clear();
        this->indexed[j].transforms.Clear();
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
VkShapeRenderer::GrowIndexBuffer()
{
    BufferCreateInfo iboInfo;
    iboInfo.name = "ShapeRenders IBO"_atm;
    iboInfo.size = this->numIndicesThisFrame;
    iboInfo.elementSize = IndexType::SizeOf(IndexType::Index32);
    iboInfo.mode = CoreGraphics::HostCached;
    iboInfo.usageFlags = CoreGraphics::IndexBuffer;
    iboInfo.data = nullptr;
    iboInfo.dataSize = 0;

    // отменить отображение текущего указателя
    if (this->indexBufferPtr)
        BufferUnmap(this->ibos[this->indexBufferActiveIndex]);

    // удалить следующий буфер, если он существует
    this->indexBufferActiveIndex = (this->indexBufferActiveIndex + 1) % MaxVertexIndexBuffers;
    if (this->ibos[this->indexBufferActiveIndex] != InvalidBufferId)
        DestroyBuffer(this->ibos[this->indexBufferActiveIndex]);

    this->indexBufferCapacity = this->numIndicesThisFrame;

    // наконец выделите новый буфер
    this->ibos[this->indexBufferActiveIndex] = CreateBuffer(iboInfo);
    this->indexBufferPtr = (byte*)BufferMap(this->ibos[this->indexBufferActiveIndex]);
    n_assert(0 != this->indexBufferPtr);
}

//------------------------------------------------------------------------------
/**
*/
void 
VkShapeRenderer::GrowVertexBuffer()
{
    BufferCreateInfo vboInfo;
    vboInfo.name = "ShapeRenderer VBO"_atm;
    vboInfo.size = this->numVerticesThisFrame;
    vboInfo.elementSize = VertexLayoutGetSize(this->vertexLayout);
    vboInfo.mode = CoreGraphics::HostCached;
    vboInfo.usageFlags = CoreGraphics::VertexBuffer;
    vboInfo.data = nullptr;
    vboInfo.dataSize = 0;

    // отменить отображение текущего указателя
    if (this->vertexBufferPtr)
        BufferUnmap(this->vbos[this->vertexBufferActiveIndex]);

    // удалить следующий буфер, если он существует
    this->vertexBufferActiveIndex = (this->vertexBufferActiveIndex + 1) % MaxVertexIndexBuffers;
    if (this->vbos[this->vertexBufferActiveIndex] != InvalidBufferId)
        DestroyBuffer(this->vbos[this->vertexBufferActiveIndex]);

    this->vertexBufferCapacity = this->numVerticesThisFrame;

    // наконец выделите новый буфер
    this->vbos[this->vertexBufferActiveIndex] = CreateBuffer(vboInfo);
    this->vertexBufferPtr = (byte*)BufferMap(this->vbos[this->vertexBufferActiveIndex]);
    n_assert(0 != this->vertexBufferPtr);
}

} // пространство имен Вулкан
