#pragma once
#include "math/mat4.h"
#include "math/vec3.h"
#include "math/quat.h"
namespace Math
{
class NEBULA_ALIGN16 transform44
{
public:
    transform44();
    transform44(const transform44& rhs) = default;
    bool operator==(const transform44& rhs) const;
    void loadu(const scalar* ptr);
    void storeu(scalar* ptr) const;
    void setposition(const vec3& p);
    void setposition(const point& p);
    const point& getposition() const;
    void setrotate(const quat& r);
    const quat& getrotate() const;
    void setscale(const vec3& s);
    const vec3& getscale() const;
    void setrotatepivot(const point& p);
    const point& getrotatepivot() const;
    void setscalepivot(const point& p);
    const point& getscalepivot() const;
    void setoffset(const mat4& m);
    const mat4& getoffset() const;
    const mat4& getmatrix();
    bool isdirty() const;
private:
    point position;
    quat rotate;
    vec3 scale;
    point rotatePivot;
    point scalePivot;
    mat4 offset;
    mat4 matrix;
    bool isDirty;
    bool offsetValid;
};
inline
transform44::transform44() :
    position(0.0f, 0.0f, 0.0f),
    rotate(0.0f, 0.0f, 0.0f, 1.0f),
    scale(1.0f, 1.0f, 1.0f),
    rotatePivot(0.0f, 0.0f, 0.0f),
    scalePivot(0.0f, 0.0f, 0.0f),
    offset(mat4()),
    isDirty(false),
    offsetValid(false)
{
}
inline bool
transform44::operator==(const transform44& rhs) const
{
    return (this->position == rhs.position) && (this->rotate == rhs.rotate) && (this->scale == rhs.scale) && (this->rotatePivot == rhs.rotatePivot) && (this->scalePivot == rhs.scalePivot) && (this->matrix == rhs.matrix);
}
__forceinline void
transform44::loadu(const scalar* ptr)
{
    this->position.loadu(ptr);
    this->rotate.loadu(ptr + 4);
    this->scale.loadu(ptr + 8);
    this->rotatePivot.loadu(ptr + 12);
    this->scalePivot.loadu(ptr + 16);
    this->offset.loadu(ptr + 20);
}
__forceinline void
transform44::storeu(scalar* ptr) const
{
    this->position.storeu(ptr);
    this->rotate.storeu(ptr + 4);
    this->scale.storeu(ptr + 8);
    this->rotatePivot.storeu(ptr + 12);
    this->scalePivot.storeu(ptr + 16);
    this->offset.storeu(ptr + 20);
}
inline void
transform44::setposition(const vec3& p)
{
    this->position = p;
    this->isDirty = true;
}
inline void
transform44::setposition(const point& p)
{
    this->position = xyz(Math::vec4(p));
    this->isDirty = true;
}
inline const point&
transform44::getposition() const
{
    return this->position;
}
inline void
transform44::setrotate(const quat& r)
{
    this->rotate = r;
    this->isDirty = true;
}
inline const quat&
transform44::getrotate() const
{
    return this->rotate;
}
inline void
transform44::setscale(const vec3& s)
{
    this->scale = s;
    this->isDirty = true;
}
inline const vec3&
transform44::getscale() const
{
    return this->scale;
}
inline void
transform44::setrotatepivot(const point& p)
{
    this->rotatePivot = p;
    this->isDirty = true;
}
inline const point&
transform44::getrotatepivot() const
{
    return this->rotatePivot;
}
inline void
transform44::setscalepivot(const point& p)
{
    this->scalePivot = p;
    this->isDirty = true;
}
inline const point&
transform44::getscalepivot() const
{
    return this->scalePivot;
}
inline void
transform44::setoffset(const mat4& m)
{
    this->offset = m;
    this->offsetValid = true;
    this->isDirty = true;
}
inline const mat4&
transform44::getoffset() const
{
    return this->offset;
}
inline const mat4&
transform44::getmatrix()
{
    if (this->isDirty)
    {
        quat ident;
        this->matrix = transformation(xyz(this->scalePivot), ident, this->scale, xyz(this->rotatePivot), this->rotate, xyz(this->position));
        if (this->offsetValid)
        {
            this->matrix = this->offset * this->matrix;
        }
        this->isDirty = false;
    }
    return this->matrix;
}
inline bool
transform44::isdirty() const
{
    return this->isDirty;
}
}