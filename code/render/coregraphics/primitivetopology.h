#pragma once
#include "core/types.h"
#include "util/string.h"
namespace CoreGraphics
{
class PrimitiveTopology
{
public:
    enum Code
    {
        InvalidPrimitiveTopology,
        PointList,
        LineList,
        LineStrip,
        LineListAdjacency,
        LineStripAdjacency,
        TriangleList,
        TriangleStrip,
        TriangleFan,
        TriangleListAdjacency,
        TriangleStripAdjacency,
        PatchList,
        NumTopologies = PatchList
    };
    static Code FromString(const Util::String& str);
    static Util::String ToString(Code code);
    static SizeT NumberOfVertices(Code topology, SizeT numPrimitives);
    static SizeT NumberOfPrimitives(Code topology, SizeT numVertices);
};
};