#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "framescript.h"
#include "io/uri.h"
namespace Frame
{
class FrameServer : public Core::RefCounted
{
    __DeclareClass(FrameServer);
    __DeclareSingleton(FrameServer);
public:
    FrameServer();
    virtual ~FrameServer();
    void Open();
    bool IsOpen() const;
    void Close();
    void OnWindowResize();
    void RebuildScripts();
    Ptr<FrameScript> LoadFrameScript(const Resources::ResourceName& name, const IO::URI& path, const CoreGraphics::WindowId window);
    const Ptr<FrameScript>& GetFrameScript(const Resources::ResourceName& name);
    void UnloadFrameScript(const Resources::ResourceName& name);
private:
    Util::Dictionary<Resources::ResourceName, Ptr<FrameScript>> frameScripts;
    CoreGraphics::TextureId windowTexture;
    bool isOpen;
};
inline const Ptr<Frame::FrameScript>&
FrameServer::GetFrameScript(const Util::StringAtom& name)
{
    return this->frameScripts[name];
}
inline bool
FrameServer::IsOpen() const
{
    return this->isOpen;
}
}