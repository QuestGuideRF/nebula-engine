#pragma once
#ifndef im3d_config_h
#define im3d_config_h
#include "render/stdneb.h"
#include "core/config.h"
#include "memory/memory.h"
#include "math/vec4.h"
#include "math/vec2.h"
#include "math/mat4.h"
#define IM3D_ASSERT(e) n_assert(e)
#define IM3D_MALLOC(size) Memory::Alloc(Memory::DefaultHeap, size)
#define IM3D_FREE(ptr) Memory::Free(Memory::DefaultHeap, ptr)
#define IM3D_VERTEX_ALIGNMENT 4
#define IM3D_VEC2_APP \
    Vec2(const Math::vec2& _v)        { x = _v.x; y = _v.y;     } \
    operator Math::vec2() const       { return Math::vec2(x, y); }
#define IM3D_VEC3_APP \
    Vec3(const Math::point& _v)       { x = _v.x; y = _v.y; z = _v.z; } \
    operator Math::point() const      { return Math::point(x, y, z);    } \
    Vec3(const Math::vector& _v)      { x = _v.x; y = _v.y; z = _v.z; } \
    operator Math::vector() const     { return Math::vector(x, y, z);    } \
    Vec3(const Math::vec3& _v)        { x = _v.x; y = _v.y; z = _v.z; } \
    operator Math::vec3() const       { return Math::vec3(x, y, z); }
#define IM3D_VEC4_APP \
    Vec4(const Math::point& _v)       { x = _v.x; y = _v.y; z = _v.z; w = 1.0f; } \
    operator Math::point() const      { return Math::point(x, y, z);    } \
    Vec4(const Math::vec4& _v)        { x = _v.x; y = _v.y; z = _v.z; w = _v.w; } \
    operator Math::vec4() const       { return Math::vec4(x, y, z, w); }
#define IM3D_MAT3_APP \
    Mat3(const Math::mat4& _m)    { float b[16]; _m.storeu(b); for(int j = 0 ; j < 3;++j) for (int i = 0; i < 3; ++i) m[i+j*3] = b[i + j*4]; } \
    operator Math::mat4() const   { Math::mat4 ret; float b[16]; ret.storeu(b); int k = 0; for(int j = 0 ; j < 3;++j) for (int i = 0; i < 3; ++i) b[i+4*j] = m[i+j*3]; ret.loadu(b); return ret; }
#define IM3D_MAT4_APP \
    Mat4(const Math::mat4& _m)    { _m.storeu(m);} \
    operator Math::mat4() const   { Math::mat4 ret; ret.loadu(m); return ret; }
#endif