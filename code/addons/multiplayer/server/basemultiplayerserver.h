#pragma once
#include "core/refcounted.h"
#include "clientconnection.h"
#include "GameNetworkingSockets/steam/steamnetworkingtypes.h"
#include "timing/timer.h"
class ISteamNetworkingSockets;
namespace Multiplayer
{
class BaseMultiplayerServer
{
public:
    BaseMultiplayerServer();
    virtual ~BaseMultiplayerServer();
    virtual bool Open();
    virtual void Close();
    bool IsOpen() const;
    void Broadcast(void* buf, int size);
    virtual bool OnClientIsConnecting(ClientConnection* connection);
    virtual void OnClientConnected(ClientConnection* connection);
    virtual void OnClientDisconnected(ClientConnection* connection);
    virtual void OnMessageReceived(ClientConnection* connection, Timing::Time recvTime, byte* data, size_t size);
    virtual void OnFrame();
    virtual void OnTick();
    Timing::Time GetTickInterval() const;
    void SetTickInterval(Timing::Time interval);
    void SetClientGroupPollInterval(ClientGroup group, Timing::Time msBetweenTicks);
    void SyncAll();
    void OnNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info);
    SizeT maxMessagesPerFrame = 1024;
protected:
    friend ClientConnection;
    void AddClientConnection(ClientConnection* connection);
    void PollIncomingMessages();
    void PollConnectionChanges();
    bool isOpen;
    Util::HashTable<HSteamNetConnection, ClientConnection*, 128> clientConnections;
	ISteamNetworkingSockets* netInterface;
    HSteamListenSocket listenSock;
	HSteamNetPollGroup pollGroups[(int)ClientGroup::NumClientGroups];
    Timing::Timer pollGroupTimers[(int)ClientGroup::NumClientGroups];
    Timing::Time pollGroupIntervals[(int)ClientGroup::NumClientGroups];
    Timing::Timer tickTimer;
    Timing::Time tickInterval;
};
inline bool
BaseMultiplayerServer::IsOpen() const
{
    return this->isOpen;
}
inline Timing::Time
BaseMultiplayerServer::GetTickInterval() const
{
    return this->tickInterval;
}
inline void
BaseMultiplayerServer::SetTickInterval(Timing::Time interval)
{
    this->tickInterval = interval;
}
}