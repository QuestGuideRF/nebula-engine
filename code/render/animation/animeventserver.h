#pragma once
#include "messaging/handler.h"
#include "core/singleton.h"
#include "animation/animeventhandlerbase.h"
namespace Animation
{
class AnimEventServer : public Messaging::Handler
{
    __DeclareClass(AnimEventServer);
    __DeclareSingleton(AnimEventServer);
public:
    AnimEventServer();
    virtual ~AnimEventServer();
    void Open();
    void Close();
    bool IsOpen() const;
    void OnFrame(Timing::Time time);
    void RegisterAnimEventHandler(const Ptr<AnimEventHandlerBase>& newHandler);
    void UnregisterAnimEventHandler(const Util::StringAtom& categoryName);
    void UnregisterAnimEventHandler(const Ptr<AnimEventHandlerBase>& handler);
    bool HandleAnimEvents(const Util::Array<Animation::AnimEventInfo>& eventz);
    virtual bool HandleMessage(const Ptr<Messaging::Message>& msg);
private:
    bool isOpen;
    Util::Dictionary<Util::StringAtom, Ptr<AnimEventHandlerBase> > animEventHandler;
};
inline bool
AnimEventServer::IsOpen() const
{
    return this->isOpen;
}
}