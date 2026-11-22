#pragma once
#include "math/vec3.h"
#include "math/mat4.h"
#include "math/bbox.h"
#include "math/rectangle.h"
#include "math/clipstatus.h"
#include "math/line.h"
namespace Math
{
class sphere
{
public:
    sphere();
    sphere(const point& _p, scalar _r);
    sphere(scalar _x, scalar _y, scalar _z, scalar _r);
    sphere(const sphere& rhs);
    void set(const point& _p, scalar _r);
    void set(scalar _x, scalar _y, scalar _z, scalar _r);
    bool inside(const bbox& box) const;
    bool intersects(const sphere& s) const;
    bool intersects(const bbox& box) const;
    bool intersects_ray(const line& l) const;
    bool intersect_sweep(const vector& va, const sphere& sb, const vector& vb, scalar& u0, scalar& u1) const;
    rectangle<scalar> project_screen_rh(const mat4& modelView, const mat4& projection, scalar nearZ) const;
    ClipStatus::Type clipstatus(const bbox& box) const;
    static vec3 random_point_on_unit_sphere();
    point p;
    scalar r;
};
inline
sphere::sphere() :
    r(1.0f)
{
}
inline
sphere::sphere(const point& _p, scalar _r) :
    p(_p),
    r(_r)
{
}
inline
sphere::sphere(scalar _x, scalar _y, scalar _z, scalar _r) :
    p(_x, _y, _z),
    r(_r)
{
}
inline
sphere::sphere(const sphere& rhs) :
    p(rhs.p),
    r(rhs.r)
{
}
inline void
sphere::set(const point& _p, scalar _r)
{
    this->p = _p;
    this->r = _r;
}
inline void
sphere::set(scalar _x, scalar _y, scalar _z, scalar _r)
{
    this->p.set(_x, _y, _z);
    this->r = _r;
}
inline bool
sphere::intersects(const sphere& s) const
{
    vec3 d(s.p - p);
    scalar rsum = s.r + r;
    return (lengthsq(d) <= (rsum * rsum));
}
inline bool
sphere::inside(const bbox& box) const
{
    vector v(this->r, this->r, this->r);
    point pmin(this->p - v);
    point pmax(this->p + v);
    bool lt = less_all(box.pmin, pmin);
    bool ge = greaterequal_all(box.pmax, pmax);
    return lt && ge;
}
inline
ClipStatus::Type
sphere::clipstatus(const bbox& box) const
{
    if (this->inside(box)) return ClipStatus::Inside;
    else if (this->intersects(box)) return ClipStatus::Clipped;
    else return ClipStatus::Outside;
}
}