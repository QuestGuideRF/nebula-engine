//------------------------------------------------------------------------------
//  emittermesh.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "particles/emittermesh.h"

#include "debug/debugfloat.h"

namespace Particles
{
using namespace CoreGraphics;
using namespace Util;
using namespace Math;

//------------------------------------------------------------------------------
/**
*/
EmitterMesh::EmitterMesh() :
    numPoints(0),
    points(NULL)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
EmitterMesh::~EmitterMesh()
{
    if (this->IsValid())
    {
        this->Discard();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
EmitterMesh::Setup(const CoreGraphics::MeshId mesh, IndexT primGroupIndex)
{
    n_assert(!this->IsValid());

    // нам нужно извлечь вершины сетки из группы примитивов
    // без повторяющихся вершин
    const PrimitiveGroup& primGroup = MeshGetPrimitiveGroups(mesh)[primGroupIndex];// сетка->GetPrimitiveGroupAtIndex(primGroupIndex);
    const BufferId indexBuffer = MeshGetIndexBuffer(mesh);
    //n_assert(IndexBufferGetType(indexBuffer) == IndexType::Index32);
    const BufferId vertexBuffer = MeshGetVertexBuffer(mesh, 0);
    SizeT numVBufferVertices = BufferGetSize(vertexBuffer);

    IndexT baseIndex = primGroup.GetBaseIndex();
    SizeT numIndices = primGroup.GetNumIndices();

    int* indices = (int*) BufferMap(indexBuffer);
    //int index[] = { 0 };

    // выделить «массив флагов», который содержит 0 в
    // индекс вершины, который еще не встречался, и
    // 1, если индекс уже встречался ранее
    SizeT size = numVBufferVertices * sizeof(uchar);
    uchar* flagArray = (uchar*) Memory::Alloc(Memory::ScratchHeap, size);
    Memory::Clear(flagArray, size);

    Array<int> emitterIndices;
    emitterIndices.Reserve(numVBufferVertices);
    IndexT i;
    for (i = baseIndex; i < (baseIndex + numIndices); i++)
    {
        int vertexIndex = indices[i];
        n_assert(vertexIndex < numVBufferVertices);
        if (flagArray[vertexIndex] == 0)
        {
            // этот индекс еще не встречался
            emitterIndices.Append(vertexIndex);
            flagArray[vertexIndex] = 1;
        }
    }
    Memory::Free(Memory::ScratchHeap, flagArray);
    flagArray = 0;
    BufferUnmap(indexBuffer);

    // массив emitterIndices теперь содержит индексы всех вершин
    // нам нужно скопировать
    this->numPoints = emitterIndices.Size();
    this->points = new EmitterPoint[this->numPoints];

    // убедитесь, что сетка эмиттера действительно содержит нужные нам компоненты
    
    const VertexLayoutId& vertexLayout = MeshGetVertexLayout(mesh);

    const Util::Array<VertexComponent>& comps = VertexLayoutGetComponents(vertexLayout);
    bool posValid = false, normValid = false, tanValid = false;
    IndexT posByteOffset = 0, normByteOffset = 0, tanByteOffset = 0;
    for (i = 0; i < comps.Size(); i++)
    {
        const VertexComponent& comp = comps[i];
        const IndexT name = comp.GetIndex();
        const VertexComponent::Format fmt = comp.GetFormat();
        if (name == VertexComponent::Position && fmt == VertexComponent::Float3)
            posValid = true, posByteOffset = comp.GetByteOffset();
        else if (name == VertexComponent::Normal && fmt == VertexComponent::Byte4N)
            normValid = true, normByteOffset = comp.GetByteOffset();
        else if (name == VertexComponent::Tangent && fmt == VertexComponent::Byte4N)
            tanValid = true, tanByteOffset = comp.GetByteOffset();
    }
    n_assert(posValid && normValid && tanValid);


    // получить доступ к вершинам и передать информацию о вершинах
    const uchar* verts = (uchar*) BufferMap(vertexBuffer);
    const SizeT vertexByteSize = VertexLayoutGetSize(vertexLayout);
    for (i = 0; i < this->numPoints; i++)
    {
        const uchar* src = verts + vertexByteSize * emitterIndices[i];
        EmitterPoint &dst = this->points[i];
        dst.position.load_float3(src + posByteOffset, 1.0f);
        dst.normal.load_byte4n(src + normByteOffset);
        dst.normal.w = 0.0f;
        dst.normal = Math::normalize(dst.normal);
        dst.tangent.load_byte4n(src + tanByteOffset);
        dst.tangent.w = 0.0f;
        dst.tangent = Math::normalize(dst.tangent);
    }
    BufferUnmap(vertexBuffer);
}

//------------------------------------------------------------------------------
/**
*/
void
EmitterMesh::Discard()
{
    n_assert(this->IsValid());
    delete[] this->points;
    this->points = 0;
    this->numPoints = 0;
}

//------------------------------------------------------------------------------
/**
*/
const EmitterMesh::EmitterPoint& 
EmitterMesh::GetEmitterPoint(IndexT key) const
{
    n_assert(key >= 0);
    n_assert(this->numPoints > 0);
    IndexT pointIndex = key % this->numPoints;
    return this->points[pointIndex];
}

} // Частицы пространства имен
