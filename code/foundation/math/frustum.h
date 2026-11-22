#pragma once
#include "math/plane.h"
#include "math/mat4.h"
#include "math/bbox.h"
#include "math/clipstatus.h"
namespace Math
{
class frustum
{
public:
    enum PlaneIndex
    {
        Near = 0,
        Far,
        Left,
        Right,
        Top,
        Bottom,
        NumPlanes
    };
    frustum();
    frustum(const mat4& invViewProj);
    void set(const mat4& invViewProj);
    void set(const bbox& box, const mat4& boxTransform);
    bool inside(const point& p) const;
    uint clipmask(const point& p) const;
    ClipStatus::Type clip(const line& l, line& clippedLine) const;
    ClipStatus::Type clipstatus(const bbox& box) const;
    ClipStatus::Type clipstatus(const bbox& box, const mat4& boxTransform) const;
    template<typename T> T as() const;
    static const int TopLeftFar = 0;
    static const int TopRightFar = 1;
    static const int BottomLeftFar = 2;
    static const int BottomRightFar = 3;
    static const int TopLeftNear = 4;
    static const int TopRightNear = 5;
    static const int BottomLeftNear = 6;
    static const int BottomRightNear = 7;
    plane planes[NumPlanes];
};
inline
frustum::frustum()
{
}
inline
frustum::frustum(const mat4& invViewProj)
{
    this->set(invViewProj);
}
inline void
frustum::set(const mat4& invViewProj)
{
    vec4 projPoints[8];
    projPoints[TopLeftFar].set(-1.0f, 1.0f, 1.0f, 1);
    projPoints[TopRightFar].set(1.0f, 1.0f, 1.0f, 1);
    projPoints[BottomLeftFar].set(-1.0f, -1.0f, 1.0f, 1);
    projPoints[BottomRightFar].set(1.0f, -1.0f, 1.0f, 1);
    projPoints[TopLeftNear].set(-1.0f, 1.0f, 0.0f, 1);
    projPoints[TopRightNear].set(1.0f, 1.0f, 0.0f, 1);
    projPoints[BottomLeftNear].set(-1.0f, -1.0f, 0.0f, 1);
    projPoints[BottomRightNear].set(1.0f, -1.0f, 0.0f, 1);
    vec3 worldPoints[8];
    IndexT i;
    for (i = 0; i < 8; i++)
    {
        vec4 p = invViewProj * projPoints[i];
        p *= 1.0f / p.w;
        worldPoints[i] = xyz(p);
    }
    this->planes[Near] = plane(worldPoints[TopRightNear], worldPoints[TopLeftNear], worldPoints[BottomLeftNear]);
    this->planes[Far] = plane(worldPoints[TopLeftFar], worldPoints[TopRightFar], worldPoints[BottomRightFar]);
    this->planes[Left] = plane(worldPoints[BottomLeftFar], worldPoints[BottomLeftNear], worldPoints[TopLeftNear]);
    this->planes[Right] = plane(worldPoints[TopRightFar], worldPoints[TopRightNear], worldPoints[BottomRightNear]);
    this->planes[Top] = plane(worldPoints[TopLeftNear], worldPoints[TopRightNear], worldPoints[TopRightFar]);
    this->planes[Bottom] = plane(worldPoints[BottomLeftFar], worldPoints[BottomRightFar], worldPoints[BottomRightNear]);
}
inline void
frustum::set(const bbox& box, const mat4& boxTransform)
{
    vec4 localPoint;
    vec3 worldPoints[8];
    IndexT i;
    for (i = 0; i < 8; i++)
    {
        switch (i)
        {
            case TopLeftFar:        localPoint.set(box.pmin.x, box.pmax.y, box.pmax.z, 1); break;
            case TopRightFar:       localPoint.set(box.pmax.x, box.pmax.y, box.pmax.z, 1); break;
            case BottomLeftFar:     localPoint.set(box.pmin.x, box.pmin.y, box.pmax.z, 1); break;
            case BottomRightFar:    localPoint.set(box.pmax.x, box.pmin.y, box.pmax.z, 1); break;
            case TopLeftNear:       localPoint.set(box.pmin.x, box.pmax.y, box.pmin.z, 1); break;
            case TopRightNear:      localPoint.set(box.pmax.x, box.pmax.y, box.pmin.z, 1); break;
            case BottomLeftNear:    localPoint.set(box.pmin.x, box.pmin.y, box.pmin.z, 1); break;
            case BottomRightNear:   localPoint.set(box.pmax.x, box.pmin.y, box.pmin.z, 1); break;
        }
        worldPoints[i] = xyz(boxTransform * localPoint);
    }
    this->planes[Near] = plane(worldPoints[TopLeftNear], worldPoints[TopRightNear], worldPoints[BottomLeftNear]);
    this->planes[Far] = plane(worldPoints[TopRightFar], worldPoints[TopLeftFar], worldPoints[BottomRightFar]);
    this->planes[Left] = plane(worldPoints[BottomLeftNear], worldPoints[BottomLeftFar], worldPoints[TopLeftNear]);
    this->planes[Right] = plane(worldPoints[BottomRightNear], worldPoints[TopRightNear], worldPoints[TopRightFar]);
    this->planes[Top] = plane(worldPoints[TopRightNear], worldPoints[TopLeftNear], worldPoints[TopRightFar]);
    this->planes[Bottom] = plane(worldPoints[BottomRightFar], worldPoints[BottomLeftFar], worldPoints[BottomRightNear]);
}
inline bool
frustum::inside(const point& p) const
{
    IndexT i;
    for (i = 0; i < NumPlanes; i++)
    {
        if (dot(this->planes[i], p) > 0.0f)
        {
            return false;
        }
    }
    return true;
}
inline uint
frustum::clipmask(const point& p) const
{
    uint clipMask = 0;
    IndexT i;
    for (i = 0; i < NumPlanes; i++)
    {
        if (dot(this->planes[i], p) > 0.0f)
        {
            clipMask |= 1<<i;
        }
    }
    return clipMask;
}
inline ClipStatus::Type
frustum::clip(const line& l, line& clippedLine) const
{
    ClipStatus::Type clipStatus = ClipStatus::Inside;
    line l0(l);
    line l1;
    IndexT i;
    for (i = 0; i < NumPlanes; i++)
    {
        ClipStatus::Type planeClipStatus = Math::clip(this->planes[i], l0, l1);
        if (ClipStatus::Outside == planeClipStatus)
        {
            return ClipStatus::Outside;
        }
        else if (ClipStatus::Clipped == planeClipStatus)
        {
            clipStatus = ClipStatus::Clipped;
        }
        l0 = l1;
    }
    clippedLine = l0;
    return clipStatus;
}
inline ClipStatus::Type
frustum::clipstatus(const bbox& box) const
{
    uint andFlags = 0xffff;
    uint orFlags = 0;
    point p;
    IndexT i;
    for (i = 0; i < 8; i++)
    {
        switch (i)
        {
            case 0:     p = box.pmin; break;
            case 1:     p.set(box.pmin.x, box.pmax.y, box.pmin.z); break;
            case 2:     p.set(box.pmax.x, box.pmax.y, box.pmin.z); break;
            case 3:     p.set(box.pmax.x, box.pmin.y, box.pmin.z); break;
            case 4:     p = box.pmax; break;
            case 5:     p.set(box.pmin.x, box.pmax.y, box.pmax.z); break;
            case 6:     p.set(box.pmin.x, box.pmin.y, box.pmax.z); break;
            case 7:     p.set(box.pmax.x, box.pmin.y, box.pmax.z); break;
        }
        uint clipMask = this->clipmask(p);
        andFlags &= clipMask;
        orFlags  |= clipMask;
    }
    if (0 == orFlags)       return ClipStatus::Inside;
    else if (0 != andFlags) return ClipStatus::Outside;
    else                    return ClipStatus::Clipped;
}
inline ClipStatus::Type
frustum::clipstatus(const bbox& box, const mat4& boxTransform) const
{
    uint andFlags = 0xffff;
    uint orFlags = 0;
    vec4 localPoint, transformedPoint;
    IndexT i;
    for (i = 0; i < 8; i++)
    {
        switch (i)
        {
            case 0:     localPoint = box.pmin; break;
            case 1:     localPoint.set(box.pmin.x, box.pmax.y, box.pmin.z, 1); break;
            case 2:     localPoint.set(box.pmax.x, box.pmax.y, box.pmin.z, 1); break;
            case 3:     localPoint.set(box.pmax.x, box.pmin.y, box.pmin.z, 1); break;
            case 4:     localPoint = box.pmax; break;
            case 5:     localPoint.set(box.pmin.x, box.pmax.y, box.pmax.z, 1); break;
            case 6:     localPoint.set(box.pmin.x, box.pmin.y, box.pmax.z, 1); break;
            case 7:     localPoint.set(box.pmax.x, box.pmin.y, box.pmax.z, 1); break;
        }
        transformedPoint = boxTransform * localPoint;
        uint clipMask = this->clipmask(xyz(transformedPoint));
        andFlags &= clipMask;
        orFlags  |= clipMask;
    }
    if (0 == orFlags)       return ClipStatus::Inside;
    else if (0 != andFlags) return ClipStatus::Outside;
    else                    return ClipStatus::Clipped;
}
}