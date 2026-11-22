//------------------------------------------------------------------------------
//  meshbuildertriangle.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "meshbuildertriangle.h"

namespace ToolkitUtil
{

//------------------------------------------------------------------------------
/**
*/
MeshBuilderTriangle::MeshBuilderTriangle()
{
    this->vertexIndex[0] = 0;
    this->vertexIndex[1] = 0;
    this->vertexIndex[2] = 0;
}

//------------------------------------------------------------------------------
/**
*/
MeshBuilderTriangle::MeshBuilderTriangle(IndexT i0, IndexT i1, IndexT i2)
{
    this->vertexIndex[0] = i0;
    this->vertexIndex[1] = i1;
    this->vertexIndex[2] = i2;
}

//------------------------------------------------------------------------------
/**
*/
void
MeshBuilderTriangle::SetVertexIndices(IndexT i0, IndexT i1, IndexT i2)
{
    this->vertexIndex[0] = i0;
    this->vertexIndex[1] = i1;
    this->vertexIndex[2] = i2;
}

//------------------------------------------------------------------------------
/**
*/
void
MeshBuilderTriangle::GetVertexIndices(IndexT& i0, IndexT& i1, IndexT& i2) const
{
    i0 = this->vertexIndex[0];
    i1 = this->vertexIndex[1];
    i2 = this->vertexIndex[2];
}

//------------------------------------------------------------------------------
/**
*/
void
MeshBuilderTriangle::SetVertexIndex(IndexT cornerIndex, IndexT vertexIndex)
{
    n_assert((cornerIndex >= 0) && (cornerIndex < 3));
    this->vertexIndex[cornerIndex] = vertexIndex;
}

//------------------------------------------------------------------------------
/**
*/
IndexT
MeshBuilderTriangle::GetVertexIndex(IndexT cornerIndex) const
{
    n_assert((cornerIndex >= 0) && (cornerIndex < 3));
    return this->vertexIndex[cornerIndex];
}

//------------------------------------------------------------------------------
/**
*/
int
MeshBuilderTriangle::Compare(const MeshBuilderTriangle& rhs) const
{
    // сравнить индексы вершин, если идентификатор группы идентичен
    IndexT i;
    for (i = 0; i < 3; i++)
    {
        if (this->vertexIndex[i] > rhs.vertexIndex[i])
        {
            return 1;
        }
        else if (this->vertexIndex[i] < rhs.vertexIndex[i])
        {
            return -1;
        }
    }

    // ИСПРАВЛЕНИЕ: По какой-то причине может случиться так, что 2 треугольника
    // с одинаковыми индексами экспортируются. Убедитесь, что мы
    // все равно получите окончательный порядок сортировки.
    // (однако повторяющиеся треугольники не должны создаваться
    // в первую очередь).
    if (this > &rhs)
    {
        return 1;
    }
    else if (this < &rhs)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
MeshBuilderTriangle::operator==(const MeshBuilderTriangle& rhs) const
{
    return (0 == this->Compare(rhs));
}

//------------------------------------------------------------------------------
/**
*/
bool
MeshBuilderTriangle::operator!=(const MeshBuilderTriangle& rhs) const
{
    return (0 != this->Compare(rhs));
}

//------------------------------------------------------------------------------
/**
*/
bool
MeshBuilderTriangle::operator<(const MeshBuilderTriangle& rhs) const
{
    return (-1 == this->Compare(rhs));
}

//------------------------------------------------------------------------------
/**
*/
bool
MeshBuilderTriangle::operator>(const MeshBuilderTriangle& rhs) const
{
    return (+1 == this->Compare(rhs));
}

} // пространство имен ToolkitUtil