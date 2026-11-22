//------------------------------------------------------------------------------
//  bbox.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "math/bbox.h"

namespace Math
{

//------------------------------------------------------------------------------
/**
    Вернуть статус коробки/бокса.
*/
ClipStatus::Type
bbox::clipstatus(const bbox& other) const
{
    if (this->contains(other))
    {
        return ClipStatus::Inside;
    }
    else if (this->intersects(other))
    {
        return ClipStatus::Clipped;
    }
    else 
    {
        return ClipStatus::Outside;
    }
}

//------------------------------------------------------------------------------
/**
    Возвращает один из 8 углов ограничивающей рамки.
*/
point
bbox::corner_point(int index) const
{
    n_assert((index >= 0) && (index < 8));
    switch (index)
    {
        case 0:     return this->pmin;
        case 1:     return point(this->pmin.x, this->pmax.y, this->pmin.z);
        case 2:     return point(this->pmax.x, this->pmax.y, this->pmin.z);
        case 3:     return point(this->pmax.x, this->pmin.y, this->pmin.z);
        case 4:     return this->pmax;
        case 5:     return point(this->pmin.x, this->pmax.y, this->pmax.z);
        case 6:     return point(this->pmin.x, this->pmin.y, this->pmax.z);
        default:    return point(this->pmax.x, this->pmin.y, this->pmax.z);
    }    
}

//------------------------------------------------------------------------------
/**
    Получите боковые плоскости ограничивающей рамки в пространстве отсечения.
*/
void
bbox::get_clipplanes(const mat4& viewProj, Util::Array<vec4>& outPlanes) const
{
    mat4 inverseTranspose = transpose(inverse(viewProj));
    vec4 planes[6] = {};
    planes[0].set(-1, 0, 0, +this->pmax.x);
    planes[1].set(+1, 0, 0, -this->pmin.x);
    planes[2].set(0, -1, 0, +this->pmax.y);
    planes[3].set(0, +1, 0, -this->pmin.y);
    planes[4].set(0, 0, -1, +this->pmax.z);
    planes[5].set(0, 0, +1, -this->pmin.z);
    IndexT i;
    for (i = 0; i < 6; i++)
    {
        outPlanes.Append(inverseTranspose * planes[i]);
    }
}


} // пространство имен Математика
