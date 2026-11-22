#pragma once
#include "core/types.h"
#include "util/string.h"
namespace Frame
{
class FrameBatchType
{
public:
    enum Code
    {
        Depth,
        Solid,
        Alpha,
        Geometry,
        Shapes,
        Text,
        UI,
        ResolveDepthBuffer,
        MousePointers,
        Lights,
        LightProbes,
        System,
        InvalidBatchType
    };
    static FrameBatchType::Code FromString(const Util::String& str);
    static Util::String ToString(FrameBatchType::Code c);
};
inline FrameBatchType::Code
FrameBatchType::FromString(const Util::String& str)
{
    if (str == "Depth")                    return Depth;
    else if (str == "Solid")               return Solid;
    else if (str == "Alpha")               return Alpha;
    else if (str == "Geometry")            return Geometry;
    else if (str == "Shapes")              return Shapes;
    else if (str == "Text")                return Text;
    else if (str == "UI")                  return UI;
    else if (str == "ResolveDepthBuffer")  return ResolveDepthBuffer;
    else if (str == "MousePointers")       return MousePointers;
    else if (str == "Lights")              return Lights;
    else if (str == "LightProbes")         return LightProbes;
    else
    {
        n_error("BatchType::FromString(): недопустимая строка '%s'!", str.AsCharPtr());
        return InvalidBatchType;
    }
}
inline Util::String
FrameBatchType::ToString(FrameBatchType::Code c)
{
    switch (c)
    {
        case Depth:                 return "Depth";
        case Solid:                 return "Solid";
        case Alpha:                 return "Alpha";
        case Geometry:              return "Geometry";
        case Shapes:                return "Shapes";
        case Text:                  return "Text";
        case UI:                    return "UI";
        case ResolveDepthBuffer:    return "ResolveDepthBuffer";
        case MousePointers:         return "MousePointers";
        case Lights:                return "Lights";
        case LightProbes:           return "LightProbes";
        default:
            n_error("BatchType::ToString(): неверный тип пакета!");
            return "";
    }
}
}