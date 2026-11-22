#pragma once
#include "core/singleton.h"
#include "core/refcounted.h"
#include "math/float2.h"
#include "math/rectangle.h"
namespace Graphics
{
    class CameraEntity;
    class View;
}
namespace CoreGraphics
{
    class Texture;
}
namespace Frame
{
    class FrameScript;
}
namespace Picking
{
class PickingServer : public Core::RefCounted
{
    __DeclareClass(PickingServer);
    __DeclareSingleton(PickingServer);
public:
    PickingServer();
    virtual ~PickingServer();
    void Open();
    void Close();
    const bool IsOpen() const;
    void Render();
    IndexT FetchIndex(const Math::float2& position);
    void FetchSquare(const Math::rectangle<float>& rect, Util::Array<IndexT> & items, Util::Array<IndexT> & edgeItems);
    float FetchDepth(const Math::float2& position);
    Math::float4 FetchNormal(const Math::float2& position);
private:
    bool isOpen;
    IndexT frameIndex;
    Ptr<Frame::FrameScript> frameShader;
    Ptr<CoreGraphics::Texture> pickingBuffer;
    Ptr<CoreGraphics::Texture> depthBuffer;
    Ptr<CoreGraphics::Texture> normalBuffer;
    Ptr<Graphics::CameraEntity> pickingCamera;
    Ptr<Graphics::View> pickingView;
};
inline const bool
PickingServer::IsOpen() const
{
    return this->isOpen;
}
}