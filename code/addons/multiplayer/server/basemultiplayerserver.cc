//------------------------------------------------------------------------------
//  basemultiplayerserver.cc
//  (C) 2025 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "basemultiplayerserver.h"
#include "components/multiplayer.h"
#include "core/debug.h"
#include "GameNetworkingSockets/steam/steamnetworkingsockets.h"
#include "game/api.h"
#include "game/filter.h"
#include "game/world.h"
#include "memdb/database.h"
#include "nflatbuffer/nebula_flat.h"
#include "nflatbuffer/flatbufferinterface.h"
#include "flat/addons/multiplayer/standardprotocol.h"
#include "flatbuffers/buffer.h"
#include "flatbuffers/flatbuffer_builder.h"
#include "steam/isteamnetworkingutils.h"
#include "multiplayer/multiplayerfeatureunit.h"
#include "serverprocessors.h"

namespace Multiplayer
{

using namespace Util;

//--------------------------------------------------------------------------
/**
*/
// ХАК: MP-сервер, вероятно, должен быть одноэлементным
static BaseMultiplayerServer* callbackServerInstance = nullptr;
static void
SvSteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* info)
{
    callbackServerInstance->OnNetConnectionStatusChanged(info);
}

//------------------------------------------------------------------------------
/**
*/
BaseMultiplayerServer::BaseMultiplayerServer()
    : isOpen(false),
    tickInterval(NEBULA_DEFAULT_TICK_RATE)
{
    for (int i = 0; i < (int)ClientGroup::NumClientGroups; i++)
    {
        this->pollGroupIntervals[i] = 1.0/60.0;
        this->pollGroupTimers[i].Start();
    }
}

//------------------------------------------------------------------------------
/**
*/
BaseMultiplayerServer::~BaseMultiplayerServer()
{
    n_assert(!this->IsOpen());
}

//------------------------------------------------------------------------------
/**
*/
bool
BaseMultiplayerServer::Open()
{
    n_assert(!this->isOpen);
    n_assert(this->clientConnections.IsEmpty());

    const int NEBULA_DEFAULT_PORT = 61111;
    this->netInterface = SteamNetworkingSockets();

    if (this->netInterface == nullptr)
    {
        n_error("Не удалось инициализировать SteamNetworkingSockets!\n");
        return false;
    }

    SteamNetworkingIPAddr serverLocalAddr;
    serverLocalAddr.Clear();
    serverLocalAddr.m_port = NEBULA_DEFAULT_PORT;
    
    SteamNetworkingConfigValue_t opt;
    // настроить обратный вызов на изменения в соединениях
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SvSteamNetConnectionStatusChangedCallback);
    
    this->listenSock = this->netInterface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
    if (this->listenSock == k_HSteamListenSocket_Invalid)
    {
        n_error("Не удалось прослушать порт %d\n", serverLocalAddr.m_port);
        return false;
    }

    for (int i = 0; i < (int)ClientGroup::NumClientGroups; i++)
    {
        this->pollGroups[i] = this->netInterface->CreatePollGroup();
        if (this->pollGroups[i] == k_HSteamNetPollGroup_Invalid)
        {
            n_error("Не удалось прослушать порт %d\n", serverLocalAddr.m_port);
            return false;
        }
    }   
    n_printf("MultiplayerServer прослушивает порт %d\n", serverLocalAddr.m_port);
    
    this->tickTimer.Start();
    this->isOpen = true;
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerServer::Close()
{
    n_assert(this->isOpen);
    // отключить клиентские соединения
    IndexT clientIndex;
    for (clientIndex = 0; clientIndex < this->clientConnections.Size(); clientIndex++)
    {
        ClientConnection* curConnection = this->clientConnections[clientIndex];
        curConnection->Shutdown();
    }
    this->clientConnections.Clear();

    this->netInterface->CloseListenSocket(this->listenSock);
    this->listenSock = k_HSteamListenSocket_Invalid;

    for (int i = 0; i < (int)ClientGroup::NumClientGroups; i++)
    {
        this->netInterface->DestroyPollGroup(this->pollGroups[i]);
        this->pollGroups[i] = k_HSteamNetPollGroup_Invalid;
    }
    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerServer::AddClientConnection(ClientConnection* conn)
{
    n_assert(conn != nullptr);
    this->clientConnections.Add(conn->GetConnectionId(), conn);
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerServer::Broadcast(void* buf, int size)
{
    const int numClients = this->clientConnections.Size();
    if (numClients == 0)
        return;

    ISteamNetworkingMessage** netMsgs = new ISteamNetworkingMessage*[numClients];
    int i;
    for (i = 0; i < numClients; i++)
    {
        netMsgs[i] = SteamNetworkingUtils()->AllocateMessage(size);
    }
        
    auto it = this->clientConnections.Begin();
    i = 0;
    while (it != this->clientConnections.End())
    {
        // ЗАДАЧА: Можем ли мы построить это вместо сообщения SteamNetworkingMessage?
        Memory::Copy(buf, netMsgs[i]->m_pData, size);

        netMsgs[i]->m_conn = (*(it.val))->GetConnectionId();
        // ХАК: пока ненадежно
        netMsgs[i]->m_nFlags = 0; 
        i++;
        it++;
    }
    
    int64* outMessageNumberOrResult = nullptr;
    this->netInterface->SendMessages(numClients, netMsgs, outMessageNumberOrResult);
}

//--------------------------------------------------------------------------
/**
*/
bool
BaseMultiplayerServer::OnClientIsConnecting(ClientConnection* connection)
{
    // Переопределить в подклассе
    return false;
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerServer::OnClientConnected(ClientConnection* connection)
{
    // Переопределить в подклассе
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerServer::OnClientDisconnected(ClientConnection* connection)
{
    // Переопределить в подклассе
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerServer::OnMessageReceived(ClientConnection* connection, Timing::Time recvTime, byte* data, size_t size)
{
    // Переопределить в подклассе
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerServer::OnNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info)
{
    switch (info->m_info.m_eState)
    {
        case k_ESteamNetworkingConnectionState_None:
            // ПРИМЕЧАНИЕ. Здесь мы будем получать обратные вызовы, когда разрушаем соединения. Мы можем игнорировать это.
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            // Игнорируйте, если они ранее не были подключены. (Если они отключились
            // прежде чем мы приняли соединение.)
            if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
            {
                // Найдите клиента.  Обратите внимание, что он должен был быть найден, потому что это
                // is the only codepath where we remove clients (except on shutdown),
                // и обратные вызовы изменения соединения отправляются в порядке очереди.
                IndexT clientIndex = this->clientConnections.FindIndex(info->m_hConn);
                n_assert(clientIndex != InvalidIndex);

                ClientConnection* connection = this->clientConnections.ValueAtIndex(info->m_hConn, clientIndex);

                // Выберите соответствующие сообщения журнала
                const char* pszDebugLogAction;
                if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
                {
                    pszDebugLogAction = "проблема обнаружена локально";
                }
                else
                {
                    // Обратите внимание, что здесь мы могли бы проверить код причины, чтобы убедиться, что
                    // это была «обычная» связь или «необычная».
                    pszDebugLogAction = "закрыто партнером";
                }

                // Выплесните что-нибудь в наш собственный журнал.  Обратите внимание, что поскольку мы указали их ник
                // в качестве описания соединения оно будет отображаться вместе с их
                // данные, специфичные для транспорта (например, их IP-адрес)
                n_printf(
                    "Соединение %s %s, причина %d: %s\n",
                    info->m_info.m_szConnectionDescription,
                    pszDebugLogAction,
                    info->m_info.m_eEndReason,
                    info->m_info.m_szEndDebug
                );

                this->OnClientDisconnected(this->clientConnections[info->m_hConn]);
                
                this->clientConnections.EraseIndex(info->m_hConn, clientIndex);

                // Отправить сообщение, чтобы все знали, что произошло?
                //Трансляция (температура);
            }
            else
            {
                n_assert(info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
            }

            // Очистите соединение.  Это важно!
            // Соединение «закрыто» в сетевом смысле, но
            // он не был уничтожен.  Мы тоже должны закрыть его со своей стороны.
            // чтобы закончить.  Информация о причине не имеет значения в данном случае,
            // и мы не можем задерживаться, потому что на другом конце уже закрыто,
            // поэтому мы просто передаем 0.
            this->netInterface->CloseConnection(info->m_hConn, 0, nullptr, false);
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
        {
            // Это должно быть новое соединение
            n_assert(this->clientConnections.FindIndex(info->m_hConn) == InvalidIndex);

            n_printf("Запрос на соединение от %s.", info->m_info.m_szConnectionDescription);

            // Клиент пытается подключиться
            // Попробуйте принять соединение.
            if (this->netInterface->AcceptConnection(info->m_hConn) != k_EResultOK)
            {
                // Это может потерпеть неудачу.  Если удаленный хост попытался подключиться, но затем
                // отключено, соединение может быть уже наполовину закрыто.  Только
                // уничтожить все, что есть на нашей стороне.
                this->netInterface->CloseConnection(info->m_hConn, 0, nullptr, false);
                n_printf("Не могу принять соединение. (Он уже закрыт?)\n");
                break;
            }

            
            ClientConnection* connection = new ClientConnection();
            connection->Initialize(this, info->m_hConn);
            
            bool accepted = this->OnClientIsConnecting(connection);

            if (!accepted)
            {
                n_printf("Соединение %s отклонено протоколом.\n", info->m_info.m_szConnectionDescription);
                delete connection;
                break;
            }

            // Назначить группу опроса
            if (!this->netInterface->SetConnectionPollGroup(info->m_hConn, this->pollGroups[(int)connection->GetClientGroup()]))
            {
                this->netInterface->CloseConnection(info->m_hConn, 0, nullptr, false);
                delete connection;
                n_printf("Не удалось установить группу опроса?\n");
                break;
            }

            this->AddClientConnection(connection);
            n_printf("Принято соединение от %s.\n", info->m_info.m_szConnectionDescription);

            // Сообщить всем остальным, что в игре появился новый игрок?
            //Broadcast(temp, info->m_hConn);
            break;
        }

        case k_ESteamNetworkingConnectionState_Connected:
            this->OnClientConnected(this->clientConnections[info->m_hConn]);
            break;

        default:
            // Тишина -Wswitch
            break;
    }
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerServer::OnTick()
{
    // Переопределить в подклассе
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerServer::OnFrame()
{
    // Переопределить в подклассе
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerServer::SyncAll()
{
    this->PollIncomingMessages();
    this->PollConnectionChanges();

    // - Рамка -
    // Происходит «каждый кадр»
    this->OnFrame();

    // - Галочка -
    // Происходит только по истечении количества секунд TicketInterval и после «Frame»

    if (this->tickTimer.GetTime() < this->tickInterval)
    {
        return;
    }

    this->tickTimer.Reset();

    this->OnTick();
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerServer::PollIncomingMessages()
{
    for (int i = 0; i < (int)ClientGroup::NumClientGroups; i++)
    {
        if (this->pollGroupTimers[i].GetTime() < this->pollGroupIntervals[i])
        {
            continue;
        }

        this->pollGroupTimers[i].Reset();

        // TODO: Не выделяйте это для каждого кадра, просто повторно используйте старый буфер.
        ISteamNetworkingMessage** incomingMsg = new ISteamNetworkingMessage*[this->maxMessagesPerFrame];
        int numMsgs = this->netInterface->ReceiveMessagesOnPollGroup(this->pollGroups[i], incomingMsg, this->maxMessagesPerFrame);
        if (numMsgs == 0)
        {
            return;
        }
        if (numMsgs < 0)
        {
            n_error("Ошибка проверки сообщений");
        }

        n_assert(numMsgs <= this->maxMessagesPerFrame && incomingMsg);
        for (int i = 0; i < numMsgs; i++)
        {   
            IndexT clientIndex = this->clientConnections.FindIndex(incomingMsg[i]->m_conn);
            n_assert(clientIndex != InvalidIndex);
            void* data = incomingMsg[i]->m_pData;
            int bytes = incomingMsg[i]->m_cbSize;
            double recvTime = incomingMsg[i]->m_usecTimeReceived;


            if (data != nullptr && bytes > 0)
            {
                this->OnMessageReceived(this->clientConnections[incomingMsg[i]->m_conn], recvTime, (byte*)data, bytes);
                incomingMsg[i]->Release();
            }
        }
        delete[] incomingMsg;
    }
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerServer::PollConnectionChanges()
{
    callbackServerInstance = this;
    this->netInterface->RunCallbacks();
}

} // пространство имен Мультиплеер
