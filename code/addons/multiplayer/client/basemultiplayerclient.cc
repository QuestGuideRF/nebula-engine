//------------------------------------------------------------------------------
//  basemultiplayerclient.cc
//  (C) 2025 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "basemultiplayerclient.h"
#include "basegamefeature/managers/timemanager.h"
#include "components/multiplayer.h"
#include "core/debug.h"
#include "GameNetworkingSockets/steam/steamnetworkingsockets.h"
#include "GameNetworkingSockets/steam/isteamnetworkingutils.h"
#include "game/api.h"
#include "game/world.h"
#include "multiplayer/client/clientprocessors.h"
#include "nflatbuffer/nebula_flat.h"
#include "nflatbuffer/flatbufferinterface.h"
#include "flat/addons/multiplayer/standardprotocol.h"
#include "multiplayer/multiplayerfeatureunit.h"
#include "net/socket/ipaddress.h"
#include "steam/steamnetworkingtypes.h"
#include "components/multiplayer.h"

namespace Multiplayer
{

using namespace Util;

//--------------------------------------------------------------------------
/**
*/
// ХАК: вероятно, должен быть синглтон
static BaseMultiplayerClient* callbackInstance = nullptr;
static void
SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* info)
{
    callbackInstance->OnNetConnectionStatusChanged(info);
}

//------------------------------------------------------------------------------
/**
*/
BaseMultiplayerClient::BaseMultiplayerClient()
    : isOpen(false),
    hostIp(N_IP_ADDR(127, 0, 0, 1))
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
BaseMultiplayerClient::~BaseMultiplayerClient()
{
    n_assert(!this->IsOpen());
}

//------------------------------------------------------------------------------
/**
*/
bool
BaseMultiplayerClient::Open()
{
    n_assert(!this->isOpen);
    this->netInterface = SteamNetworkingSockets();
    if (this->netInterface == nullptr)
    {
        n_error("Не удалось инициализировать SteamNetworkingSockets!\n");
        return false;
    }
    this->isOpen = true;

    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerClient::Close()
{
    n_assert(this->isOpen);

    ShutdownClientProcessors();
    const char* reason = nullptr;
#if NEBULA_DEBUG
    reason = "Клиент добровольно закрыл соединение.";
#endif

    n_printf("Закрытие!\n");
    this->netInterface->CloseConnection(this->connectionId, 0, reason, true);
    this->isOpen = false;
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerClient::Send(void* buf, int size)
{
    ISteamNetworkingMessage** netMsgs = new ISteamNetworkingMessage*[1];
    netMsgs[0] = SteamNetworkingUtils()->AllocateMessage(size);
        
    // ЗАДАЧА: Можем ли мы построить это вместо сообщения SteamNetworkingMessage?
    Memory::Copy(buf, netMsgs[0]->m_pData, size);

    netMsgs[0]->m_conn = this->connectionId;
    netMsgs[0]->m_nFlags = 0; 
    
    int64* outMessageNumberOrResult = nullptr;
    this->netInterface->SendMessages(1, netMsgs, outMessageNumberOrResult);
}

//--------------------------------------------------------------------------
/**
*/
bool
BaseMultiplayerClient::TryConnect()
{
    if (this->connectionStatus != ConnectionStatus::Disconnected)
    {
        // Уже пытаюсь подключиться или уже подключено.
        return true;
    }

    const double TIMEOUT_INTERVAL = 1.0;
    if (this->timeoutTimer.Running() && this->timeoutTimer.GetTime() < TIMEOUT_INTERVAL)
    {
        return false;
    }
    
    const int NEBULA_DEFAULT_PORT = 61111;

    SteamNetworkingIPAddr serverAddr;
    serverAddr.Clear();
    // TODO: реализовать поддержку приведения IP к адресу uint32_t и порту uint16_t.
    //Net::IpAddress ipAddress = Net::IpAddress();
    serverAddr.SetIPv4(this->hostIp, NEBULA_DEFAULT_PORT);

    // Начать подключение
    char address[SteamNetworkingIPAddr::k_cchMaxString];
    serverAddr.ToString(address, sizeof(address), true);
    n_printf("Попытка подключиться к серверу %s...\n", address);
    
    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);

    this->connectionId = this->netInterface->ConnectByIPAddress(serverAddr, 1, &opt);
    
    if (this->connectionId == k_HSteamNetConnection_Invalid)
    {
        n_printf("Failed to connect by IP. (Invalid ip address or port?)\n");
        this->timeoutTimer.Reset();
        if (!this->timeoutTimer.Running())
        {
            this->timeoutTimer.Start();
        }
        return false;
    }

    this->connectionStatus = ConnectionStatus::TryingToConnect;
    return true;
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerClient::OnIsConnecting()
{
    // Переопределить в подклассе
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerClient::OnConnected()
{
    // Переопределить в подклассе
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerClient::OnDisconnected()
{
    // Переопределить в подклассе
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerClient::OnMessageReceived(SteamNetworkingMessage_t* msg)
{
    // Переопределить в подклассе
}

//--------------------------------------------------------------------------
/**
*/
ConnectionStatus
BaseMultiplayerClient::GetConnectionStatus() const
{
    return this->connectionStatus;
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerClient::OnNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info)
{
    // Каково состояние соединения?
    switch ( info->m_info.m_eState )
    {
        case k_ESteamNetworkingConnectionState_None:
            // ПРИМЕЧАНИЕ. Здесь мы будем получать обратные вызовы, когда разрушаем соединения. Мы можем пока игнорировать их.
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            // Распечатать соответствующее сообщение
            if ( info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting )
            {
                // Примечание: мы могли различать тайм-аут, отклоненное соединение,
                // или какая-то другая транспортная проблема.
                n_printf("%s\n", info->m_info.m_szEndDebug);
            }
            else if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
            {
                n_printf("Lost connection with server (%s)\n", info->m_info.m_szEndDebug);
            }
            else
            {
                // ПРИМЕЧАНИЕ. Мы могли бы проверить код причины нормального отключения.
                n_printf("Отключено хостом (%s)", info->m_info.m_szEndDebug);
            }

            this->OnDisconnected();

            // Очистите соединение.  Это важно!
            // Соединение «закрыто» в сетевом смысле, но
            // он не был уничтожен.  Мы тоже должны закрыть его со своей стороны.
            // чтобы закончить.  Информация о причине не имеет значения в данном случае,
            // и мы не можем задерживаться, потому что на другом конце уже закрыто,
            // поэтому мы просто передаем 0.
            this->netInterface->CloseConnection(info->m_hConn, 0, nullptr, false);
            if (this->connectionId == info->m_hConn)
            {
                this->connectionId = k_HSteamNetConnection_Invalid;
                this->connectionStatus = ConnectionStatus::Disconnected;
                this->timeoutTimer.Reset();
                if (!this->timeoutTimer.Running())
                {
                    this->timeoutTimer.Start();
                }
            }
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
            // Мы получим этот обратный вызов, когда начнем подключаться.
            this->connectionStatus = ConnectionStatus::TryingToConnect;
            this->connectionId = info->m_hConn;
            this->OnIsConnecting();
            break;

        case k_ESteamNetworkingConnectionState_Connected:
            this->connectionStatus = ConnectionStatus::Connected;
            this->OnConnected();
            n_printf("Соединение с сервером успешно установлено!\n");
            break;

        default:
            break;
    }
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerClient::SyncAll()
{
    this->PollConnectionChanges();

    if (this->connectionStatus != ConnectionStatus::Connected)
        return;

    SteamNetConnectionRealTimeStatus_t status;
    EResult res = this->netInterface->GetConnectionRealTimeStatus(this->connectionId, &status, 0, nullptr);
    if (res == k_EResultNoConnection)
    {
        this->ping = 0.0;
    }
    else
    {
        this->ping = (double)status.m_nPing / 1000.0;
    }

    this->PollIncomingMessages();
    this->PushPendingMessages();
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerClient::PollIncomingMessages()
{
    ISteamNetworkingMessage* incomingMsg[MaxMessagesPerFrame];
    int numMsgs = this->netInterface->ReceiveMessagesOnConnection(this->connectionId, incomingMsg, MaxMessagesPerFrame);
    if (numMsgs == 0)
    {
        return;
    }
    if (numMsgs < 0)
    {
        n_error("Ошибка проверки сообщений");
    }

    n_assert(numMsgs <= MaxMessagesPerFrame && incomingMsg);
    for (int i = 0; i < numMsgs; i++)
    {
        void* data = incomingMsg[i]->m_pData;
        int bytes = incomingMsg[i]->m_cbSize;
        double recvTime = incomingMsg[i]->m_usecTimeReceived;
        if (data != nullptr && bytes > 0)
        {
            this->OnMessageReceived(incomingMsg[i]);
            incomingMsg[i]->Release();
            incomingMsg[i] = nullptr;
        }
    }
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerClient::PollConnectionChanges()
{
    callbackInstance = this;
    this->netInterface->RunCallbacks();
}

//--------------------------------------------------------------------------
/**
*/
void
BaseMultiplayerClient::PushPendingMessages()
{
    //SteamNetworkingMessage_t* netMsgs[4] = { nullptr, nullptr, nullptr, nullptr };
//
    /// TODO: не делайте каждый раз новый билдер
    //Flatbuffers::FlatBufferBuilder строитель(1024);
    //Flatbuffers::Offset<Protocol::MsgTest> msgTest;
    //Flatbuffers::Offset<Protocol::Message> сообщение;
    //
    //for (int i = 0; i < 4; i++)
    //{
    //    строитель.Сброс();
    //    статическое плавающее значение f = 0,0f;
    //    f += 0.01f;
    //    msgTest = Протокол::CreateMsgTest(строитель, f);
    //    сообщение = Протокол::CreateMessage(строитель, Протокол::Data_Test, msgTest.Union());
//
    //    builder.Finish(сообщение);
    //    
    //    uint8_t* buf = builder.GetBufferPointer();
    //    int size = builder.GetSize();
    //    
    //    netMsgs[i] = SteamNetworkingUtils()->AllocateMessage(размер);
    //    
    //    // ЗАДАЧА: Можем ли мы создать это вместо сообщения Steamnetworkingmessage?
    //    Память::Copy(buf, netMsgs[i]->m_pData, size);
    //    
    //    netMsgs[i]->m_conn = this->connectionId;
    //    netMsgs[i]->m_nFlags = 0;
    //}
    //
    //int64 * outMessageNumberOrResult = nullptr;
    //this->netInterface->SendMessages(4, netMsgs, outMessageNumberOrResult);
}

} // пространство имен Мультиплеер
