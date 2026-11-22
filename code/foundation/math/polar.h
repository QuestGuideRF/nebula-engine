#pragma once
#ifndef MATH_POLAR_H
#define MATH_POLAR_H
#include "math/scalar.h"
#include "math/vec3.h"
#include "math/vec2.h"
namespace Math
{
class polar
{
public:
    polar();
    polar(scalar t, scalar r);
    polar(const vec3& v);
    polar(const polar& src);
    void operator=(const polar& rhs);
    vec3 get_cartesian() const;
    void set(const polar& p);
    void set(scalar t, scalar r);
    void set(const vec3& v);
    scalar theta;
    scalar rho;
};
inline
polar::polar() :
    theta(0.0f),
    rho(0.0f)
{
}
inline
polar::polar(scalar t, scalar r) :
    theta(t),
    rho(r)
{
}
inline
polar::polar(const vec3& v)
{
    this->set(v);
}
inline
polar::polar(const polar& src) :
    theta(src.theta),
    rho(src.rho)
{
}
inline void
polar::operator=(const polar& rhs)
{
    this->theta = rhs.theta;
    this->rho = rhs.rho;
}
inline void
polar::set(const polar& p)
{
    this->theta = p.theta;
    this->rho = p.rho;
}
inline void
polar::set(scalar t, scalar r)
{
    this->theta = t;
    this->rho = r;
}
inline void
polar::set(const vec3& vec)
{
    vec3 normVec3d = normalize(vec);
    this->theta = Math::acos(normVec3d.y);
    vec2 normVec2d(normVec3d.x, normVec3d.z);
    if (normVec2d.length() > TINY)
    {
        normVec2d = vec2::normalize(normVec2d);
    }
    else
    {
        normVec2d.set(1.0f, 0.0f);
    }
    if ((normVec2d.x >= 0.0f) && (normVec2d.y >= 0.0f))
    {
        this->rho = Math::asin(normVec2d.x);
    }
    else if ((normVec2d.x < 0.0f) && (normVec2d.y >= 0.0f))
    {
        this->rho = Math::asin(normVec2d.y) + Math::deg2rad(270.0f);
    }
    else if ((normVec2d.x < 0.0f) && (normVec2d.y < 0.0f))
    {
        this->rho = Math::asin(-normVec2d.x) + Math::deg2rad(180.0f);
    }
    else
    {
        this->rho = Math::asin(-normVec2d.y) + Math::deg2rad(90.0f);
    }
}
inline vec3
polar::get_cartesian() const
{
    scalar sinTheta = Math::sin(this->theta);
    scalar cosTheta = Math::cos(this->theta);
    scalar sinRho   = Math::sin(this->rho);
    scalar cosRho   = Math::cos(this->rho);
    vec3 v(sinTheta * sinRho, cosTheta, sinTheta * cosRho);
    return v;
}
}
#endif