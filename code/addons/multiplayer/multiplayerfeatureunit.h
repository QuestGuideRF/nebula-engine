#pragma once
#include "game/featureunit.h"
#include "server/basemultiplayerserver.h"
#include "client/basemultiplayerclient.h"
namespace Multiplayer
{
const double NEBULA_DEFAULT_TICK_RATE = 1.0/8.0;
class MultiplayerFeatureUnit : public Game::FeatureUnit
{
    __DeclareClass(MultiplayerFeatureUnit)
    __DeclareSingleton(MultiplayerFeatureUnit)
public:
    MultiplayerFeatureUnit();
    ~MultiplayerFeatureUnit();
    void OnAttach() override;
    void OnActivate() override;
    void OnDeactivate() override;
    void OnBeginFrame() override;
    void OnFrame() override;
    void OnEndFrame() override;
    virtual void OnRenderDebug();
    bool IsServer() const;
    void SetServer(BaseMultiplayerServer* server);
    void SetClient(BaseMultiplayerClient* client);
private:
    BaseMultiplayerServer* server = nullptr;
    BaseMultiplayerClient* client = nullptr;
};
inline bool
MultiplayerFeatureUnit::IsServer() const
{
    return this->server != nullptr;
}
inline void
MultiplayerFeatureUnit::SetServer(BaseMultiplayerServer* server)
{
    this->server = server;
}
inline void
MultiplayerFeatureUnit::SetClient(BaseMultiplayerClient* client)
{
    this->client = client;
}
}