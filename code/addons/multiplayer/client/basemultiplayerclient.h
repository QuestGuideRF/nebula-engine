#pragma once
#include "GameNetworkingSockets/steam/steamnetworkingtypes.h"
#include "timing/timer.h"
class ISteamNetworkingSockets;
namespace Multiplayer
{
enum class ConnectionStatus
{
    Disconnected = 0,
    TryingToConnect = 1,
    Connected = 2
};
class BaseMultiplayerClient
{
public:
    BaseMultiplayerClient();
    virtual ~BaseMultiplayerClient();
    virtual bool Open();
    virtual void Close();
    bool IsOpen() const;
    void Send(void* buf, int size);
    bool TryConnect();
    void SetHostIp(uint32_t ip);
    virtual void OnIsConnecting();
    virtual void OnConnected();
    virtual void OnDisconnected();
    virtual void OnMessageReceived(SteamNetworkingMessage_t* msg);
    Timing::Time GetCurrentPing() const;
    ConnectionStatus GetConnectionStatus() const;
    void SyncAll();
    void OnNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info);
    constexpr static SizeT MaxMessagesPerFrame = 1024;
protected:
    void PollIncomingMessages();
    void PollConnectionChanges();
    void PushPendingMessages();
    uint32_t hostIp;
    double ping;
    bool isOpen;
    Timing::Timer timeoutTimer;
    ConnectionStatus connectionStatus = ConnectionStatus::Disconnected;
    HSteamNetConnection connectionId = k_HSteamNetConnection_Invalid;
	ISteamNetworkingSockets* netInterface;
};
inline bool
BaseMultiplayerClient::IsOpen() const
{
    return this->isOpen;
}
inline Timing::Time
BaseMultiplayerClient::GetCurrentPing() const
{
    return this->ping;
}
inline void
BaseMultiplayerClient::SetHostIp(uint32_t ip)
{
    this->hostIp = ip;
}
}