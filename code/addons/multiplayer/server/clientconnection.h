#pragma once
#include "GameNetworkingSockets/steam/steamnetworkingtypes.h"
#include "timing/time.h"
namespace Multiplayer
{
class BaseMultiplayerServer;
enum class ClientGroup
{
    DontCare = 0,
    Lobby,
    Game,
    Monitoring,
    NumClientGroups
};
class ClientConnection
{
public:
    ClientConnection();
    virtual ~ClientConnection();
    void Initialize(BaseMultiplayerServer* server, HSteamNetConnection connectionId);
    bool IsConnected() const;
    virtual void Shutdown();
    void SetClientGroup(ClientGroup group);
    ClientGroup GetClientGroup() const;
    uint32_t GetConnectionId() const;
    Timing::Time GetCurrentPing() const;
    uint64_t GetUserData() const;
    void SetUserData(uint64_t);
protected:
    BaseMultiplayerServer* server;
    ClientGroup group = ClientGroup::DontCare;
    HSteamNetConnection connectionId = k_HSteamNetConnection_Invalid;
    uint64_t userData;
};
inline uint32_t
ClientConnection::GetConnectionId() const
{
    return this->connectionId;
}
inline ClientGroup
ClientConnection::GetClientGroup() const
{
    return this->group;
}
inline uint64_t
ClientConnection::GetUserData() const
{
    return this->userData;
}
inline void
ClientConnection::SetUserData(uint64_t data)
{
    this->userData = data;
}
}