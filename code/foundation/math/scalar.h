#pragma once
namespace Math
{
#ifndef PI
#define PI (3.1415926535897932384626433832795028841971693993751)
#endif
#ifndef N_PI
#define N_PI (Math::scalar(PI))
#endif
#ifndef N_PI_DOUBLE
#define N_PI_DOUBLE (Math::scalar(6.283185307179586476925286766559))
#endif
#ifndef N_PI_HALF
#define N_PI_HALF (Math::scalar(1.5707963267948966192313216916398f))
#endif
#ifndef TINY
#define TINY (0.0000001f)
#endif
#define N_TINY TINY
typedef float scalar;
typedef float float32;
typedef double float64;
#ifndef PI
#define PI (3.1415926535897932384626433832795028841971693993751)
#endif
#ifndef N_PI
#define N_PI (Math::scalar(3.1415926535897932384626433832795028841971693993751))
#endif
struct float2
{
    union
    {
        struct { scalar x, y; };
        scalar v[2];
    };
};
struct float3
{
    union
    {
        struct { scalar x, y, z; };
        scalar v[3];
    };
};
struct float4
{
    union
    {
        struct { scalar x, y, z, w; };
        scalar v[4];
    };
};
struct int2
{
    union
    {
        struct { int x, y; };
        int v[2];
    };
};
struct int3
{
    union
    {
        struct { int x, y, z; };
        int v[3];
    };
};
struct int4
{
    union
    {
        struct { int x, y, z, w; };
        int v[4];
    };
};
struct uint2
{
    union
    {
        struct { unsigned int x, y; };
        unsigned int v[2];
    };
};
struct uint3
{
    union
    {
        struct { unsigned int x, y, z; };
        unsigned int v[3];
    };
};
struct uint4
{
    union
    {
        struct { unsigned int x, y, z, w; };
        unsigned int v[4];
    };
};
struct byte4u
{
    union
    {
        struct { ubyte x, y, z, w; };
        unsigned int v;
    };
};
struct byte4
{
    union
    {
        struct { byte x, y, z, w; };
        unsigned int v;
    };
};
__forceinline scalar
rand()
{
    return scalar(::rand()) / scalar(RAND_MAX);
}
__forceinline scalar
rand(scalar min, scalar max)
{
    scalar unit = scalar(::rand()) / RAND_MAX;
    scalar diff = max - min;
    return min + unit * diff;
}
__forceinline scalar
sin(scalar x)
{
    return sinf(x);
}
__forceinline scalar
cos(scalar x)
{
    return cosf(x);
}
__forceinline scalar
tan(scalar x)
{
    return tanf(x);
}
__forceinline scalar
asin(scalar x)
{
    return asinf(x);
}
__forceinline scalar
acos(scalar x)
{
    return acosf(x);
}
__forceinline scalar
atan(scalar x)
{
    return atanf(x);
}
__forceinline scalar
sqrt(scalar x)
{
    return sqrtf(x);
}
__forceinline int
fchop(scalar f)
{
    return int(f);
}
__forceinline scalar
fmod(scalar x, scalar y)
{
    return fmodf(x, y);
}
__forceinline scalar
fract(scalar x)
{
    return x - (int)x;
}
__forceinline scalar
modangle(scalar a)
{
#if 0
    static const scalar rev = scalar(6.283185307179586476925286766559);
    return n_fmod(a,rev);
#else
    static const scalar REVOLUTION = scalar(6.283185307179586476925286766559);
    a += N_PI;
    scalar temp = fabsf(a);
    temp = temp - floorf(temp/REVOLUTION) *  REVOLUTION;
    temp -= N_PI;
    temp = a<0 ? -temp:temp;
    if(temp < scalar(-N_PI)) temp += REVOLUTION;
    if(temp >= scalar(N_PI)) temp -= REVOLUTION;
    return temp;
#endif
}
__forceinline scalar
log2(scalar f)
{
    return log2f(f);
}
__forceinline scalar
exp2(scalar x)
{
    return exp2f(x);
}
__forceinline scalar
log(scalar x)
{
    return logf(x);
}
__forceinline scalar
exp(scalar x)
{
    return expf(x);
}
__forceinline int
frnd(scalar f)
{
    return fchop(floorf(f + 0.5f));
}
__forceinline scalar
pow(scalar x, scalar y)
{
    return powf(x, y);
}
__forceinline bool
fequal(scalar f0, scalar f1, scalar tol)
{
    scalar f = f0 - f1;
    return ((f > (-tol)) && (f < tol));
}
template<typename TYPE>
__forceinline TYPE
max(TYPE a, TYPE b)
{
    return (a > b) ? a : b;
}
template<typename TYPE0, typename ...TYPEN>
__forceinline TYPE0
max(TYPE0 first, TYPE0 second, TYPEN... rest)
{
    return first > second ? max(first, std::forward<TYPEN>(rest)...) : max(second, std::forward<TYPEN>(rest)...);
}
template<>
__forceinline unsigned int
max(unsigned int a, unsigned int b)
{
    return a ^ ((a ^ b) & -(a < b));
}
template<typename TYPE>
__forceinline TYPE
min(TYPE a, TYPE b)
{
    return (a < b) ? a : b;
}
template<>
__forceinline unsigned int
min(unsigned int a, unsigned int b)
{
    return b ^ ((a ^ b) & -(a < b));
}
template<>
__forceinline uint16_t
min(uint16_t a, uint16_t b)
{
    return b ^ ((a ^ b) & -(a < b));
}
template<typename TYPE0, typename ...TYPEN>
__forceinline TYPE0
min(TYPE0 first, TYPE0 second, TYPEN... rest)
{
    return first < second ? min(first, std::forward<TYPEN>(rest)...) : min(second, std::forward<TYPEN>(rest)...);
}
__forceinline scalar
abs(scalar a)
{
    return (a < 0.0f) ? -a : a;
}
__forceinline double
abs(double a)
{
    return (a < 0.0) ? -a : a;
}
__forceinline int
abs(int a)
{
    return (a < 0) ? -a : a;
}
__forceinline scalar
sgn(scalar a)
{
    return (a < 0.0f) ? -1.0f : 1.0f;
}
__forceinline constexpr scalar
deg2rad(scalar d)
{
    return (scalar)((d * N_PI) / 180.0f);
}
__forceinline constexpr scalar
rad2deg(scalar r)
{
    return (scalar)((r * 180.0f) / N_PI);
}
__forceinline float
clamp(float val, float minVal, float maxVal)
{
    if (val < minVal)      return minVal;
    else if (val > maxVal) return maxVal;
    else return val;
}
__forceinline int64_t
clamp(int64_t val, int64_t minVal, int64_t maxVal)
{
    if (val < minVal)      return minVal;
    else if (val > maxVal) return maxVal;
    else return val;
}
__forceinline int32_t
clamp(int32_t val, int32_t minVal, int32_t maxVal)
{
    if (val < minVal)      return minVal;
    else if (val > maxVal) return maxVal;
    else return val;
}
__forceinline float
ceil(float val)
{
    return ceilf(val);
}
__forceinline float
floor(float val)
{
    return floorf(val);
}
__forceinline float
round(float val)
{
    return roundf(val);
}
__forceinline bool
fless(scalar f0, scalar f1, scalar tol)
{
    return ((f0 - f1) < tol);
}
__forceinline bool
fgreater(scalar f0, scalar f1, scalar tol)
{
    return ((f0 - f1) > tol);
}
__forceinline scalar
saturate(scalar val)
{
    if (val < 0.0f)      return 0.0f;
    else if (val > 1.0f) return 1.0f;
    else return val;
}
__forceinline double
saturate(double val)
{
    if (val < 0.0)      return 0.0;
    else if (val > 1.0) return 1.0;
    else return val;
}
__forceinline float
lerp(float x, float y, float l)
{
    return x + l * (y - x);
}
__forceinline scalar
angulardistance(scalar from, scalar to)
{
    scalar normFrom = modangle(from);
    scalar normTo   = modangle(to);
    scalar dist = normTo - normFrom;
    if (dist < deg2rad(-180.0f))
    {
        dist += deg2rad(360.0f);
    }
    else if (dist > deg2rad(180.0f))
    {
        dist -= deg2rad(360.0f);
    }
    return dist;
}
__forceinline bool
isdenormal(scalar s)
{
#if __GNUC__
    union { scalar s; unsigned int u; } pun;
    pun.s = s;
    return ((pun.u&0x7f800000)==0);
#else
    return (((*(unsigned int*)&s)&0x7f800000)==0);
#endif
}
__forceinline float
undenormalize(scalar s)
{
    if (isdenormal(s))
    {
        return 0.0f;
    }
    else
    {
        return s;
    }
}
__forceinline bool
nearequal(scalar a, scalar b, scalar epsilon)
{
    return abs(a - b) <= epsilon;
}
__forceinline scalar
cot(scalar x)
{
    return scalar(1.0) / tan(x);
}
__forceinline scalar
smoothstep(scalar edge0, scalar edge1, scalar x)
{
    x = saturate((x - edge0) / (edge1 - edge0));
    return x*x*(3-2*x);
}
__forceinline int
irand(int min, int max)
{
    int range = max - min + 1;
    int unit = ::rand() % range;
    return min + unit;
}
__forceinline int
mostsignificant(unsigned int val)
{
#ifdef WIN32
    unsigned long ret;
    bool res = _BitScanReverse(&ret, val);
    ret = res ? ret : 0;
    return ret + 1;
#else
    unsigned long ret;
    ret = __builtin_clz(val);
    return ret + 1;
#endif
}
__forceinline unsigned int
align(unsigned int alignant, unsigned int alignment)
{
    return (alignant + alignment - 1) & ~(alignment - 1);
}
__forceinline uintptr_t
alignptr(uintptr_t alignant, uintptr_t alignment)
{
    return (alignant + alignment - 1) & ~(alignment - 1);
}
__forceinline unsigned int
align_down(unsigned int alignant, unsigned int alignment)
{
    return (alignant / alignment * alignment);
}
__forceinline uintptr_t
align_downptr(uintptr_t alignant, uintptr_t alignment)
{
    return (alignant / alignment * alignment);
}
__forceinline unsigned int
divandroundup(unsigned int dividend, unsigned int divider)
{
    return (dividend % divider != 0) ? (dividend / divider + 1) : (dividend / divider);
}
__forceinline unsigned int
roundtopow2(unsigned int val)
{
    val--;
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;
    val++;
    return val;
}
struct randxorstate
{
    uint64_t x[2];
};
__forceinline uint64_t
randxor(randxorstate& state)
{
    uint64_t t = state.x[0];
    uint64_t const s = state.x[1];
    state.x[0] = s;
    t ^= t << 23;
    t ^= t >> 18;
    t ^= s ^ (s >> 5);
    state.x[1] = t;
    return t + s;
}
__forceinline uint32_t
pointerhash(void* ptr)
{
    return (uint32_t) (((uintptr_t)(void *)ptr) % 4294967291u);
}
inline float
normtofloat(uint16_t value)
{
    return (float)value / 65535.0f;
}
inline float
normtofloat(int16_t value)
{
    return Math::max((float)value / 32767.0f, -1.0f);
}
inline float
normtofloat(uint8_t value)
{
    return (float)value / 255.0f;
}
inline float
normtofloat(int8_t value)
{
    return Math::max((float)value / 127.0f, -1.0f);
}
}
inline long double
operator"" _rad(long double deg)
{
    return Math::deg2rad(deg);
}
inline long double
operator"" _rad(unsigned long long deg)
{
    return Math::deg2rad(deg);
}
inline long double
operator"" _deg(long double rad)
{
    return Math::rad2deg(rad);
}
inline long double
operator"" _deg(unsigned long long deg)
{
    return Math::rad2deg(deg);
}