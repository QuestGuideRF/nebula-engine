#pragma once
namespace Multiplayer
{
class BaseMultiplayerServer;
void SetupServerProcessors(BaseMultiplayerServer* server);
void ShutdownServerProcessors();
void SetServerProcessorsActive(bool active);
}