#pragma once
#include "core/refcounted.h"
#include "coregraphics/texture.h"
#include "io/uri.h"
namespace ToolkitUtil
{
class CubeFilterer : public Core::RefCounted
{
    __DeclareClass(CubeFilterer);
public:
    CubeFilterer();
    virtual ~CubeFilterer();
    void SetCubeFaces(const Util::FixedArray<Ptr<CoreGraphics::Texture>>& cubefaces);
    void SetOutputFile(const IO::URI& output);
    void SetOutputSize(uint size);
    void SetSpecularPower(uint power);
    void SetGenerateMips(bool b);
    void Filter(bool irradiance, void* messageHandler, void(*CubeFilterer_Progress)(const Util::String&, void*));
    void DepthCube(bool genConeMap, void* messageHandler, void(*CubeFilterer_Progress)(const Util::String&, void*));
private:
    Util::FixedArray<Ptr<CoreGraphics::Texture>> cubefaces;
    IO::URI output;
    bool generateMips;
    uint power;
    uint size;
};
inline void
CubeFilterer::SetCubeFaces(const Util::FixedArray<Ptr<CoreGraphics::Texture>>& cubefaces)
{
    this->cubefaces = cubefaces;
}
inline void
CubeFilterer::SetOutputFile(const IO::URI& output)
{
    this->output = output;
}
inline void
CubeFilterer::SetOutputSize(uint size)
{
    this->size = size;
}
inline void
CubeFilterer::SetSpecularPower(uint power)
{
    this->power = power;
}
inline void
CubeFilterer::SetGenerateMips(bool b)
{
    this->generateMips = b;
}
}