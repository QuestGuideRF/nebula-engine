#pragma once
#include "math/vec4.h"
#include "math/mat4.h"
#include "math/quat.h"
#include "math/plane.h"
#include "math/scalar.h"
#include "core/types.h"
#include "util/typepunning.h"
namespace Debug
{
class DebugFloat
{
public:
    static void printHex(float val, const char *msg = NULL);
    static void printHex(const Math::vec4 &v, const char *msg = NULL);
    static void printHex(const Math::mat4 &m, const char *msg = NULL);
    static void printHex(const Math::quat &q, const char *msg = NULL);
    static void printHex(const Math::plane &p, const char *msg = NULL);
    static void restoreHex(float &v, int val);
    static void restoreHex(Math::vec4 &v, int x, int y, int z, int w);
    static void restoreHex(Math::mat4 &m,
                                       int m0, int m1, int m2, int m3,
                                       int m4, int m5, int m6, int m7,
                                       int m8, int m9, int m10, int m11,
                                       int m12, int m13, int m14, int m15);
    static void restoreHex(Math::quat &q, int x, int y, int z, int w);
    static void restoreHex(Math::plane &p, int x, int y, int z, int w);
    static void print(const float &v, const char *msg = NULL);
    static void print(const Math::vec4 &v, const char *msg = NULL);
    static void print(const Math::mat4 &m, const char *msg = NULL);
    static void print(const Math::quat &q, const char *msg = NULL);
    static void print(const Math::plane &p, const char *msg = NULL);
};
inline
void
DebugFloat::restoreHex(float &v, int val)
{
    v = Util::TypePunning<float, int>(val);
}
inline
void
DebugFloat::restoreHex(Math::vec4 &v, int x, int y, int z, int w)
{
    const int vRaw[4] = {x, y, z, w};
    float *p = (float*)&v;
    for(int index = 0; index < 4; ++index)
    {
        *p++ = Util::TypePunning<float, int>(vRaw[index]);
    }
}
inline
void
DebugFloat::restoreHex(Math::mat4 &m,
                       int m0, int m1, int m2, int m3,
                       int m4, int m5, int m6, int m7,
                       int m8, int m9, int m10, int m11,
                       int m12, int m13, int m14, int m15)
{
    const int mRaw[4][4] = {{m0, m1, m2, m3},
                            {m4, m5, m6, m7},
                            {m8, m9, m10, m11},
                            {m12, m13, m14, m15}};
    float *p = (float*)&m.row0;
    for(int index = 0; index < 16; ++index)
    {
        *p++ = Util::TypePunning<float, int>(mRaw[index/4][index%4]);
    }
}
inline
void
DebugFloat::restoreHex(Math::quat &q, int x, int y, int z, int w)
{
    const int vRaw[4] = {x, y, z, w};
    float *p = (float*)&q;
    for(int index = 0; index < 4; ++index)
    {
        *p++ = Util::TypePunning<float, int>(vRaw[index]);
    }
}
inline
void
DebugFloat::restoreHex(Math::plane &pl, int x, int y, int z, int w)
{
    const int vRaw[4] = {x, y, z, w};
    float *p = (float*)&pl;
    for(int index = 0; index < 4; ++index)
    {
        *p++ = Util::TypePunning<float, int>(vRaw[index]);
    }
}
inline
void
DebugFloat::printHex(const Math::mat4 &m, const char *msg)
{
    if(msg) n_printf("%s: ", msg);
    printHex(m.row0);
    n_printf(", ");
    printHex(m.row1);
    n_printf(", ");
    printHex(m.row2);
    n_printf(", ");
    printHex(m.row3);
    if(msg) n_printf("\n");
}
inline
void
DebugFloat::printHex(const Math::vec4 &v, const char *msg)
{
    if(msg) n_printf("%s: ", msg);
    const float *p = (const float*)&v;
    printHex(p[0]);
    n_printf(", ");
    printHex(p[1]);
    n_printf(", ");
    printHex(p[2]);
    n_printf(", ");
    printHex(p[3]);
    if(msg) n_printf("\n");
}
inline
void
DebugFloat::printHex(const Math::quat &q, const char *msg)
{
    if(msg) n_printf("%s: ", msg);
    const float *p = (const float*)&q;
    printHex(p[0]);
    n_printf(", ");
    printHex(p[1]);
    n_printf(", ");
    printHex(p[2]);
    n_printf(", ");
    printHex(p[3]);
    if(msg) n_printf("\n");
}
inline
void
DebugFloat::printHex(const Math::plane &pl, const char *msg)
{
    if(msg) n_printf("%s: ", msg);
    const float *p = (const float*)&pl;
    printHex(p[0]);
    n_printf(", ");
    printHex(p[1]);
    n_printf(", ");
    printHex(p[2]);
    n_printf(", ");
    printHex(p[3]);
    if(msg) n_printf("\n");
}
inline
void
DebugFloat::printHex(float val, const char *msg)
{
    if(msg) n_printf("%s: ", msg);
    const int bits = Util::TypePunning<int, float>(val);
    unsigned char tmp;
    char buffer[(sizeof(int)*2)+3];
    char *p = buffer;
    *p++ = '0';
    *p++ = 'x';
    for( int i = sizeof(int)*2-1; i > -1; --i )
    {
        tmp = ( bits >> (i*4) ) & 0xF;
        if( tmp >= 10 )
        {
            *p++ = tmp - 10 + 'A';
        }
        else
        {
            *p++ = tmp + '0';
        }
    }
    *p = '\0';
    n_printf("%s", buffer );
    if(msg) n_printf("\n");
}
inline
void
DebugFloat::print(const Math::vec4 &v, const char *msg)
{
    if(msg) n_printf("%s: ", msg);
    n_printf("%10.4f %10.4f %10.4f %10.4f", v.x, v.y, v.z, v.w);
    if(msg) n_printf("\n");
}
inline
void
DebugFloat::print(const float &v, const char *msg)
{
    if(msg) n_printf("%s:\n", msg);
    n_printf("%10.4f", v);
    if(msg) n_printf("\n");
}
inline
void
DebugFloat::print(const Math::mat4 &m, const char *msg)
{
    if(msg) n_printf("%s:\n", msg);
    print(Math::vec4(m.row0.x, m.row1.x, m.row2.x, m.row3.x), NULL);
    n_printf("\n");
    print(Math::vec4(m.row0.y, m.row1.y, m.row2.y, m.row3.y), NULL);
    n_printf("\n");
    print(Math::vec4(m.row0.z, m.row1.z, m.row2.z, m.row3.z), NULL);
    n_printf("\n");
    print(Math::vec4(m.row0.w, m.row1.w, m.row2.w, m.row3.w), NULL);
    n_printf("\n");
}
inline
void
DebugFloat::print(const Math::quat &q, const char *msg)
{
    if(msg) n_printf("%s:\n", msg);
    if(Math::nearequal(length(q), 1.0f, 0.00001f))
    {
        const Math::scalar angle = Math::acos(q.w) * 2.0f;
        n_printf("    ось: %f %f %f\n"
                 "    w        : %f\n"
                 "    магнитуда: %f рад: %f град: %f\n",
                 q.x, q.y, q.z, q.w, length(q), angle, Math::rad2deg(angle));
    }
    else
    {
        n_printf("    ось: %f %f %f\n"
                 "    w        : %f\n"
                 "    магнитуда: %f\n",
                 q.x, q.y, q.z, q.w, length(q));
    }
    if(msg) n_printf("\n");
}
}