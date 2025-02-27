#pragma once

#include <extlib/steam/steam_api.h>

#include <unordered_map>

#include "Network/Packet.hpp"
#include "Network/IPacketData.hpp"
#include "Network/PacketDataJoinInfo.hpp"
#include "Network/PacketDataPlayerInfo.hpp"
#include "Network/PacketDataWorldInfo.hpp"
#include "Network/PacketDataObjectHit.hpp"
#include "Network/PacketDataObjectBuilt.hpp"
#include "Network/PacketDataObjectDestroyed.hpp"
#include "Network/PacketDataItemPickupsCreated.hpp"
#include "Network/PacketDataItemPickupDeleted.hpp"
#include "Network/PacketDataItemPickupsCreateRequest.hpp"
#include "Network/PacketDataInventoryAddItem.hpp"
#include "Network/PacketDataChestOpened.hpp"
#include "Network/PacketDataChestClosed.hpp"
#include "Network/PacketDataChestDataModified.hpp"
#include "Network/PacketDataChunkDatas.hpp"
#include "Network/PacketDataChunkRequests.hpp"

#include "GUI/InventoryGUI.hpp"
#include "World/ChunkPosition.hpp"
#include "Player/Player.hpp"

class Game;

class NetworkHandler
{
public:
    NetworkHandler(Game* game);
    void reset();

    void startHostServer();

    void receiveMessages();

    void sendGameUpdatesToClients();
    void sendGameUpdatesToHost();

    void leaveLobby();

    EResult sendPacketToClients(const Packet& packet, int nSendFlags, int nRemoteChannel);
    EResult sendPacketToHost(const Packet& packet, int nSendFlags, int nRemoteChannel);

    bool isMultiplayerGame();
    bool getIsLobbyHost();

private:
    void receiveMessagesHost();
    void receiveMessagesClient();

    void callbackLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);

    void registerNetworkPlayer(uint64_t id, bool notify = true);
    void deleteNetworkPlayer(uint64_t id);

    STEAM_CALLBACK(NetworkHandler, callbackLobbyJoinRequested, GameLobbyJoinRequested_t);
    STEAM_CALLBACK(NetworkHandler, callbackLobbyEnter, LobbyEnter_t);
    STEAM_CALLBACK(NetworkHandler, callbackLobbyUpdated, LobbyChatUpdate_t);
    STEAM_CALLBACK(NetworkHandler, callbackMessageSessionRequest, SteamNetworkingMessagesSessionRequest_t);

private:
    bool multiplayerGame;
    uint64_t steamLobbyId;
    bool isLobbyHost;
    uint64_t lobbyHost;

    Game* game;

    std::unordered_map<uint64_t, Player> networkPlayers;

    // Client-specific
    static constexpr float CHUNK_REQUEST_OUTSTANDING_MAX_TIME = 2.0f;
    std::unordered_map<ChunkPosition, float> chunkRequestsOutstanding;

    // Steam callback results
    CCallResult<NetworkHandler, LobbyCreated_t> m_SteamCallResultCreateLobby;

};