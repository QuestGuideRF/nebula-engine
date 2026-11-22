#pragma once
#if !SPU
#include "memory/memory.h"
#endif
#include <stddef.h>
#include <stdint.h>
#ifdef DeleteFile
#undef DeleteFile
#endif
#ifdef CopyFile
#undef CopyFile
#endif
#ifdef GetObject
#undef GetObject
#endif
typedef unsigned long  ulong;
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef unsigned char  ubyte;
typedef uintptr_t uintptr;
typedef ptrdiff_t ptrdiff;
typedef int IndexT;
typedef int SizeT;
typedef int64_t Index64T;
typedef int64_t Size64T;
typedef uintptr PtrT;
typedef ptrdiff PtrDiff;
static const int InvalidIndex = -1;
#define N_BIT(x) (1 << x)
template <class MASK, class BITS>
constexpr MASK
SetBits(const MASK mask, const BITS bit)
{
    return MASK(uint(mask) | uint(bit));
}
template <class MASK, class BITS>
constexpr MASK
UnsetBits(const MASK mask, const BITS bit)
{
    return MASK(uint(mask) & ~uint(bit));
}
#define N_ARGB(a,r,g,b) ((uint32_t)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define N_RGBA(r,g,b,a) N_ARGB(a,r,g,b)
#define N_XRGB(r,g,b)   N_ARGB(0xff,r,g,b)
#define N_COLORVALUE(r,g,b,a) N_RGBA((uint32_t)((r)*255.f),(uint)((g)*255.f),(uint)((b)*255.f),(uint)((a)*255.f))
#define N_IP_ADDR(a,b,c,d) (uint32_t)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((c)&0xff)<<8)|((d)&0xff))
template <class FLAGS, class BITS>
constexpr bool
AllBits(const FLAGS flags, const BITS bits)
{
    return (flags & bits) == bits;
}
template <class FLAGS, class BITS>
constexpr bool
AnyBits(const FLAGS flags, const BITS bits)
{
    return (flags & bits) != (FLAGS)0;
}
template <class FLAGS, class BITS>
constexpr bool
OnlyBits(const FLAGS flags, const BITS bits)
{
    return (flags & bits) == flags;
}
#define BITS_TO_BYTES(x) (((x)+7)>>3)
#define BYTES_TO_BITS(x) ((x)<<3)
#if (__OSX__ || __linux__)
inline ushort                _byteswap_ushort(ushort x)              { return ((x>>8) | (x<<8)); }
inline ulong                 _byteswap_ulong(ulong x)                { return ((x&0xff000000)>>24) | ((x&0x00ff0000)>>8) | ((x&0x00000ff00)<<8) | ((x&0x000000ff)<<24); }
inline unsigned long long    _byteswap_uint64(unsigned long long x)  { return ((((unsigned long long)_byteswap_ulong((ulong)(x & 0xffffffff))) << 32) | ((unsigned long long)_byteswap_ulong((ulong)(x >> 32)))); }
#endif
#if __linux__
typedef unsigned char byte;
#endif
#if __WIN32__
#define n_stricmp stricmp
#define n_snprintf StringCchPrintf
#elif (__OSX__ || __APPLE__ || __linux__ )
#define n_stricmp strcasecmp
#define n_snprintf sprintf
#else
#error "Неподдерживаемая платформа!"
#endif
#if __MAYA__
#define ThreadLocal
#elif __WIN32__
#define ThreadLocal __declspec(thread)
#elif __linux__
#define ThreadLocal __thread
#if (__OSX__ || __APPLE__)
#undef ThreadLocal
#define ThreadLocal
#endif
#else
#error "Неподдерживаемая платформа!"
#endif
#define lengthof(x) (sizeof(x) / sizeof(*x))
#define NEBULA_ALIGN16 alignas(16)