#pragma once
#ifndef MATH_LINE_H
#define MATH_LINE_H
#include "math/point.h"
#include "math/scalar.h"
namespace Math
{
class line
{
public:
    line();
    line(const point& startPoint, const point& endPoint);
    line(const line& rhs);
    void set(const point& startPoint, const point& endPoint);
    void set_point_dir(const point& startPoint, const vector& direction);
    const point& start() const;
    point end() const;
    const vector& vec() const;
    scalar length() const;
    scalar lengthsq() const;
    scalar distance(const point& p) const;
    bool intersect(const line& l, point& pa, point& pb) const;
    scalar distance(const line& l, point& pa, point& pb) const;
    scalar closestpoint(const point& p) const;
    point pointat(scalar t) const;
    point b;
    vector m;
};
inline
line::line()
{
}
inline
line::line(const point& startPoint, const point& endPoint) :
    b(startPoint),
    m(endPoint - startPoint)
{
}
inline
line::line(const line& rhs) :
    b(rhs.b),
    m(rhs.m)
{
}
inline void
line::set(const point& startPoint, const point& endPoint)
{
    this->b = startPoint;
    this->m = endPoint - startPoint;
}
inline void
line::set_point_dir(const point& startPoint, const vector& dir)
{
    this->b = startPoint;
    this->m = dir;
}
inline const point&
line::start() const
{
    return this->b;
}
inline point
line::end() const
{
    return this->b + this->m;
}
inline const vector&
line::vec() const
{
    return this->m;
}
inline scalar
line::length() const
{
    return Math::length(this->m);
}
inline scalar
line::lengthsq() const
{
    return Math::lengthsq(this->m);
}
inline scalar
line::closestpoint(const point& p) const
{
    vector diff(p - this->b);
    scalar l = dot(this->m, this->m);
    if (l > 0.0f)
    {
        scalar t = dot(this->m, diff) / l;
        return t;
    }
    else
    {
        return 0.0f;
    }
}
inline scalar
line::distance(const point& p) const
{
    vector diff(p - this->b);
    scalar l = dot(this->m, this->m);
    if (l > 0.0f)
    {
        scalar t = dot(this->m, diff) / l;
        diff = diff - this->m * t;
        return Math::length(diff);
    }
    else
    {
        vector v(p - this->b);
        return Math::length(v);
    }
}
inline point
line::pointat(scalar t) const
{
    return this->b + this->m * t;
}
}
#endif