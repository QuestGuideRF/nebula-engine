#pragma once
#include "core/types.h"
#include "math/scalar.h"
#include "math/vec4.h"
#include "math/sse.h"
namespace Math
{
struct quat;
quat slerp(const quat& q1, const quat& q2, scalar t);
quat rotationmatrix(const mat4& m);
vec3 to_euler(const quat& q);
quat from_euler(const vec3& v);
quat quatyawpitchroll(scalar y, scalar x, scalar z);
vec3 rotate(quat const& q, vec3 const& v);
struct NEBULA_ALIGN16 quat
{
public:
    quat();
    quat(quat const&) = default;
    quat(scalar x, scalar y, scalar z, scalar w);
    quat(const vec4& rhs);
    quat(const __m128& rhs);
    void operator=(const __m128& rhs);
    bool operator==(const quat& rhs) const;
    bool operator!=(const quat& rhs) const;
    void load(const scalar* ptr);
    void loadu(const scalar* ptr);
    void store(scalar* ptr) const;
    void storeu(scalar* ptr) const;
    void stream(scalar* ptr) const;
    vec3 x_axis() const;
    vec3 y_axis() const;
    vec3 z_axis() const;
    void set(scalar x, scalar y, scalar z, scalar w);
    void set(vec4 const &f4);
    friend struct mat4;
    union
    {
        struct
        {
            float x, y, z, w;
        };
        float v[4];
        __m128 vec;
    };
};
__forceinline
quat::quat()
{
    this->vec = _mm_setr_ps(0, 0, 0, 1);
}
__forceinline
quat::quat(scalar x, scalar y, scalar z, scalar w)
{
    this->vec = _mm_setr_ps(x, y, z, w);
}
__forceinline
quat::quat(const vec4& rhs) :
    vec(rhs.vec)
{
}
__forceinline
quat::quat(const __m128& rhs)
{
    this->vec = rhs;
}
__forceinline void
quat::operator=(const __m128& rhs)
{
    this->vec = rhs;
}
__forceinline bool
quat::operator==(const quat& rhs) const
{
    return _mm_movemask_ps(_mm_cmpeq_ps(this->vec, rhs.vec)) == 0x0f;
}
__forceinline bool
quat::operator!=(const quat& rhs) const
{
    return _mm_movemask_ps(_mm_cmpeq_ps(this->vec, rhs.vec)) != 0x0f;
}
__forceinline void
quat::load(const scalar* ptr)
{
    this->vec = _mm_load_ps(ptr);
}
__forceinline void
quat::loadu(const scalar* ptr)
{
    this->vec = _mm_loadu_ps(ptr);
}
__forceinline void
quat::store(scalar* ptr) const
{
    _mm_store_ps(ptr, this->vec);
}
__forceinline void
quat::storeu(scalar* ptr) const
{
    _mm_storeu_ps(ptr, this->vec);
}
__forceinline void
quat::stream(scalar* ptr) const
{
    this->store(ptr);
}
__forceinline vec3
quat::x_axis() const
{
    vec3 const v = vec3(1, 0, 0);
    return rotate(*this, v);
}
__forceinline vec3
quat::y_axis() const
{
    vec3 const v = vec3(0, 1, 0);
    return rotate(*this, v);
}
__forceinline vec3
quat::z_axis() const
{
    vec3 const v = vec3(0, 0, 1);
    return rotate(*this, v);
}
__forceinline void
quat::set(scalar x, scalar y, scalar z, scalar w)
{
    this->vec = _mm_setr_ps(x, y, z, w);
}
__forceinline void
quat::set(const vec4& f4)
{
    this->vec = f4.vec;
}
__forceinline bool
isidentity(const quat& q)
{
    const quat id(0, 0, 0, 1);
    return id == q;
}
__forceinline scalar
length(const quat& q)
{
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(q.vec, q.vec, 0xF1)));
}
__forceinline scalar
lengthsq(const quat& q)
{
    return _mm_cvtss_f32(_mm_dp_ps(q.vec, q.vec, 0xF1));
}
__forceinline quat
quatUndenormalize(const quat& q)
{
    quat ret;
#if __WIN32__
    ret.x = Math::undenormalize(q.x);
    ret.y = Math::undenormalize(q.y);
    ret.z = Math::undenormalize(q.z);
    ret.w = Math::undenormalize(q.w);
#endif
    return ret;
}
__forceinline quat
barycentric(const quat& q0, const quat& q1, const quat& q2, scalar f, scalar g)
{
    scalar s = f + g;
    if (s != 0.0f)
    {
        quat a = slerp(q0, q1, s);
        quat b = slerp(q0, q2, s);
        quat res = slerp(a, b, g / s);
        return res;
    }
    else
    {
        return q0;
    }
}
__forceinline quat
conjugate(const quat& q)
{
    const  __m128 con = { -1.0f, -1.0f, -1.0f, 1.0f };
    quat qq(_mm_mul_ps(q.vec, con));
    return qq;
}
__forceinline scalar
dot(const quat& q0, const quat& q1)
{
    return _mm_cvtss_f32(_mm_dp_ps(q0.vec, q1.vec, 0xF1));
}
__forceinline quat
quatExp(const quat& q)
{
    vec4 f(q.vec);
    scalar theta = length3(f);
    scalar costheta = Math::cos(theta);
    scalar sintheta = Math::sin(theta);
    f *= sintheta / theta;
    f.w = costheta;
    return quat(f.vec);
}
__forceinline quat
identity()
{
    return quat(_mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f));
}
__forceinline quat
inverse(const quat& q)
{
    scalar len = lengthsq(q);
    if (len > 0.00001f)
    {
        quat con = conjugate(q);
        __m128 temp = _mm_set1_ps(1.0f / len);
        con.vec = _mm_mul_ps(con.vec, temp);
        return con;
    }
    return quat(0.0f, 0.0f, 0.0f, 0.0f);
}
__forceinline quat
ln(const quat& q)
{
    quat ret;
    scalar a = Math::acos(q.w);
    scalar isina = 1.0f / Math::sin(a);
    scalar aisina = a * isina;
    if (isina > 0)
    {
        vec3 mul(aisina, aisina, aisina);
        ret = vec4(vec3(q.vec) * mul, 0.0f);
    }
    else
    {
        ret.set(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return ret;
}
__forceinline quat
operator*(const quat& q0, const quat& q1)
{
    static const __m128 controlWZYX = _mm_setr_ps(1, -1, 1, -1);
    static const __m128 controlZWXY = _mm_setr_ps(1, 1, -1, -1);
    static const __m128 controlYXWZ = _mm_setr_ps(-1, 1, 1, -1);
    __m128 res = _mm_shuffle_ps(q1.vec, q1.vec, _MM_SHUFFLE(3, 3, 3, 3));
    __m128 q1x = _mm_shuffle_ps(q1.vec, q1.vec, _MM_SHUFFLE(0, 0, 0, 0));
    __m128 q1y = _mm_shuffle_ps(q1.vec, q1.vec, _MM_SHUFFLE(1, 1, 1, 1));
    __m128 q1z = _mm_shuffle_ps(q1.vec, q1.vec, _MM_SHUFFLE(2, 2, 2, 2));
    res = _mm_mul_ps(res, q0.vec);
    __m128 q0shuffle = q0.vec;
    q0shuffle = _mm_shuffle_ps(q0shuffle, q0shuffle, _MM_SHUFFLE(0, 1, 2, 3));
    q1x = _mm_mul_ps(q1x, q0shuffle);
    q0shuffle = _mm_shuffle_ps(q0shuffle, q0shuffle, _MM_SHUFFLE(2, 3, 0, 1));
    res = fmadd(q1x, controlWZYX, res);
    q1y = _mm_mul_ps(q1y, q0shuffle);
    q0shuffle = _mm_shuffle_ps(q0shuffle, q0shuffle, _MM_SHUFFLE(0, 1, 2, 3));
    q1y = _mm_mul_ps(q1y, controlZWXY);
    q1z = _mm_mul_ps(q1z, q0shuffle);
    q1y = fmadd(q1z, controlYXWZ, q1y);
    return _mm_add_ps(res, q1y);
}
__forceinline quat
normalize(const quat& q)
{
    return quat(_mm_div_ps(q.vec, _mm_sqrt_ps(_mm_dp_ps(q.vec, q.vec, 0xff))));
}
__forceinline quat
rotationquataxis(const vec3& axis, scalar angle)
{
    n_assert2(Math::nearequal(length(axis), 1.0f, 0.001f), "ось должна быть нормализована");
    float sinangle = Math::sin(0.5f * angle);
    float cosangle = Math::cos(0.5f * angle);
    __m128 b = _mm_and_ps(axis.vec, _mask_xyz);
    b = _mm_or_ps(b, _id_w);
    return _mm_mul_ps(b, _mm_set_ps(cosangle, sinangle, sinangle, sinangle));
}
__forceinline quat
slerp(const quat& q1, const quat& q2, scalar t)
{
    __m128 to;
    float qdot = dot(q1, q2);
    if (qdot < 0)
    {
        qdot = -qdot;
        to = _mm_mul_ps(q2.vec, _mm_set_ps1(-1.0f));
    }
    else
    {
        to = q2.vec;
    }
    if (qdot < 0.95f)
    {
        float clamped = Math::clamp(qdot, -1.0f, 1.0f);
        float angle = Math::acos(clamped);
        float sin_angle = Math::sin(angle);
        float sin_angle_t = Math::sin(angle * t);
        float sin_omega_t = Math::sin(angle * (1.0f - t));
        __m128 s0 = _mm_set_ps1(sin_omega_t);
        __m128 s1 = _mm_set_ps1(sin_angle_t);
        __m128 sin_div = _mm_set_ps1(1.0f / sin_angle);
        return  _mm_mul_ps(_mm_add_ps(_mm_mul_ps(q1.vec, s0), _mm_mul_ps(to, s1)), sin_div);
    }
    else
    {
        float scale0 = 1.0f - t;
        float scale1 = t;
        __m128 s0 = _mm_set_ps1(scale0);
        __m128 s1 = _mm_set_ps1(scale1);
        return _mm_add_ps(_mm_mul_ps(q1.vec, s0), _mm_mul_ps(to, s1));
    }
}
__forceinline void
squadsetup(const quat& q0, const quat& q1, const quat& q2, const quat& q3, quat& aOut, quat& bOut, quat& cOut)
{
    n_error("FIXME: не реализовано");
}
__forceinline quat
squad(const quat& q1, const quat& a, const quat& b, const quat& c, scalar t)
{
    return slerp(slerp(q1, c, t), slerp(a, b, t), 2.0f * t * (1.0f - t));
}
__forceinline void
to_axisangle(const quat& q, vec4& outAxis, scalar& outAngle)
{
    outAxis = q.vec;
    outAxis.w = 0;
    outAngle = 2.0f * Math::acos(q.w);
    outAxis.w = 0.0f;
}
__forceinline vec3
rotate(quat const& q, vec3 const& v)
{
    vec3 const i = q.vec;
    vec3 const qxv = cross(i, v);
    vec3 const rv = v * q.w;
    vec3 const rot = cross(i, qxv + rv) * 2.0f;
    return v + rot;
}
}