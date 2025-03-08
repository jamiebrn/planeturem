#pragma once

#include <extlib/steam/steam_api.h>

#include <unordered_map>
#include <vector>

#include "Network/Packet.hpp"
#include "Network/IPacketData.hpp"
#include "Network/PacketDataJoinQuery.hpp"
#include "Network/PacketDataJoinReply.hpp"
#include "Network/PacketDataJoinInfo.hpp"
#include "Network/PacketDataServerInfo.hpp"
#include "Network/PacketDataPlayerData.hpp"
#include "Network/PacketDataPlayerCharacterInfo.hpp"
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
#include "Player/PlayerData.hpp"
#include "Player/NetworkPlayer.hpp"

class Game;

class NetworkHandler
{
public:
    NetworkHandler() = default;
    NetworkHandler(Game* game);
    void reset(Game* game);

    void startHostServer();
    
    void sendWorldJoinReply(std::string playerName = "");

    void receiveMessages();

    void sendGameUpdates();
    void sendGameUpdatesToClients();
    void sendGameUpdatesToHost();

    void leaveLobby();
    
    EResult sendPacketToClients(const Packet& packet, int nSendFlags, int nRemoteChannel);
    EResult sendPacketToClient(uint64_t steamID, const Packet& packet, int nSendFlags, int nRemoteChannel);
    EResult sendPacketToHost(const Packet& packet, int nSendFlags, int nRemoteChannel);
    
    void requestChunksFromHost(PlanetType planetType, std::vector<ChunkPosition>& chunks);

    void sendPlayerDataToHost(const PlayerData& playerData);
    
    bool isMultiplayerGame();
    bool isLobbyHostOrSolo();
    bool getIsLobbyHost();
    bool isClient();
    int getNetworkPlayerCount();
    std::optional<uint64_t> getLobbyID();

    NetworkPlayer* getNetworkPlayer(uint64_t id);
    std::unordered_map<uint64_t, NetworkPlayer>& getNetworkPlayers();

    std::vector<WorldObject*> getNetworkPlayersToDraw(const LocationState& locationState, sf::Vector2f playerPosition);

    std::string getPlayerName(uint64_t id);

    const PlayerData* getSavedNetworkPlayerData(uint64_t id);
    void setSavedNetworkPlayerData(uint64_t id, const PlayerData& networkPlayerData);
    const std::unordered_map<uint64_t, PlayerData> getSavedNetworkPlayerDataMap();

private:
    void processMessage(const SteamNetworkingMessage_t& message, const Packet& packet);
    void processMessageAsHost(const SteamNetworkingMessage_t& message, const Packet& packet);
    void processMessageAsClient(const SteamNetworkingMessage_t& message, const Packet& packet);
    
    void callbackLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);
    
    void registerNetworkPlayer(uint64_t id, bool notify = true);
    void deleteNetworkPlayer(uint64_t id);

    void handleChunkDatasFromHost(const PacketDataChunkDatas& chunkDatas);

    void sendPlayerDataToClients(const PacketDataPlayerData& playerDataPacket);

    // Callbacks
    STEAM_CALLBACK(NetworkHandler, callbackLobbyJoinRequested, GameLobbyJoinRequested_t);
    STEAM_CALLBACK(NetworkHandler, callbackLobbyEnter, LobbyEnter_t);
    STEAM_CALLBACK(NetworkHandler, callbackLobbyUpdated, LobbyChatUpdate_t);
    STEAM_CALLBACK(NetworkHandler, callbackMessageSessionRequest, SteamNetworkingMessagesSessionRequest_t);

private:
    bool multiplayerGame;
    uint64_t steamLobbyId;
    bool isLobbyHost;
    uint64_t lobbyHost;

    Game* game = nullptr;

    std::unordered_map<uint64_t, NetworkPlayer> networkPlayers;
    std::unordered_map<uint64_t, PlayerData> networkPlayerDatasSaved;

    // Client-specific
    static constexpr float CHUNK_REQUEST_OUTSTANDING_MAX_TIME = 2.0f;
    std::unordered_map<ChunkPosition, float> chunkRequestsOutstanding;

    // Steam callback results
    CCallResult<NetworkHandler, LobbyCreated_t> m_SteamCallResultCreateLobby;

};