#pragma once
namespace Multiplayer
{
class BaseMultiplayerClient;
void SetupClientProcessors(BaseMultiplayerClient* client);
void ShutdownClientProcessors();
}