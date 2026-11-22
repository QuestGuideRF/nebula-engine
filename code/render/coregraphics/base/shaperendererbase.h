#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "math/mat4.h"
#include "math/vec4.h"
#include "math/bbox.h"
#include "coregraphics/primitivetopology.h"
#include "coregraphics/indextype.h"
#include "coregraphics/shader.h"
#include "coregraphics/rendershape.h"
#include "threading/threadid.h"
namespace Base
{
class ShapeRendererBase : public Core::RefCounted
{
    __DeclareClass(ShapeRendererBase);
    __DeclareSingleton(ShapeRendererBase);
public:
    ShapeRendererBase();
    virtual ~ShapeRendererBase();
    void Open();
    void Close();
    bool IsOpen() const;
    void ClearShapes();
    void AddShape(const CoreGraphics::RenderShape& shape);
    void AddShapes(const Util::Array<CoreGraphics::RenderShape>& shapeArray);
    void AddWireFrameBox(const Math::bbox& boundingBox, const Math::vec4& color);
    void DrawShapes();
    enum ShaderTypes
    {
        Mesh,
        MeshNoDepth,
        MeshWireframe,
        Primitives,
        PrimitivesNoDepth,
        PrimitivesWireframeTriangles,
        PrimitivesWireframeLines,
        NumShaders
    };
protected:
    bool isOpen;
    Util::Array<CoreGraphics::RenderShape> shapes[ShaderTypes::NumShaders];
    Util::Array<CoreGraphics::RenderShape> primitives[ShaderTypes::NumShaders];
    CoreGraphics::ShaderProgramId programs[ShaderTypes::NumShaders];
    SizeT numIndicesThisFrame;
    SizeT numVerticesThisFrame;
};
inline bool
ShapeRendererBase::IsOpen() const
{
    return this->isOpen;
}
}