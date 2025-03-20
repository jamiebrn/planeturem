#pragma once

#include <extlib/steam/steam_api.h>

#include <unordered_map>
#include <vector>

#include "Core/Camera.hpp"

#include "Network/Packet.hpp"
#include "Network/IPacketData.hpp"
#include "Network/PacketData/PacketDataIncludes.hpp"

#include "GUI/InventoryGUI.hpp"
#include "World/ChunkPosition.hpp"
#include "World/ChunkViewRange.hpp"
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

    void update(float dt);

    void updateNetworkPlayers(float dt);

    void sendGameUpdates(float dt, const Camera& camera);
    void sendGameUpdatesToClients();
    void sendGameUpdatesToHost(const Camera& camera);

    void leaveLobby();
    
    EResult sendPacketToClients(const Packet& packet, int nSendFlags, int nRemoteChannel);
    EResult sendPacketToClient(uint64_t steamID, const Packet& packet, int nSendFlags, int nRemoteChannel);
    EResult sendPacketToHost(const Packet& packet, int nSendFlags, int nRemoteChannel);
    
    void requestChunksFromHost(PlanetType planetType, std::vector<ChunkPosition>& chunks);

    void queueSendPlayerData();
    void sendPlayerData();

    bool canSendStructureRequest();
    void structureRequestSent();

    bool isMultiplayerGame();
    bool isLobbyHostOrSolo();
    bool getIsLobbyHost();
    bool isClient();
    int getNetworkPlayerCount();
    std::optional<uint64_t> getLobbyID();

    NetworkPlayer* getNetworkPlayer(uint64_t id);
    std::unordered_map<uint64_t, NetworkPlayer>& getNetworkPlayers();

    std::vector<WorldObject*> getNetworkPlayersToDraw(const LocationState& locationState, sf::Vector2f playerPosition);

    std::vector<ChunkViewRange> getNetworkPlayersChunkViewRanges(PlanetType planetType);

    std::unordered_set<PlanetType> getPlayersPlanetTypeSet(std::optional<PlanetType> thisPlayerPlanetType);
    std::unordered_set<RoomType> getPlayersRoomDestTypeSet(std::optional<RoomType> thisPlayerRoomType);

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

    void sendPlayerDataToHost();
    void sendHostPlayerDataToClients();
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

    static constexpr float SERVER_UPDATE_TICK = 1 / 30.0f;
    float updateTick;

    std::unordered_map<uint64_t, NetworkPlayer> networkPlayers;
    std::unordered_map<uint64_t, PlayerData> networkPlayerDatasSaved;

    static constexpr float SEND_PLAYER_DATA_QUEUE_TIME = 1.0f;
    bool sendPlayerDataQueued;
    float sendPlayerDataQueueTime;

    static constexpr float STRUCTURE_ENTER_REQUEST_COOLDOWN = 0.5f;
    float structureEnterRequestCooldown;

    // Client-specific
    static constexpr float CHUNK_REQUEST_OUTSTANDING_MAX_TIME = 2.0f;
    std::unordered_map<ChunkPosition, float> chunkRequestsOutstanding;

    // Steam callback results
    CCallResult<NetworkHandler, LobbyCreated_t> m_SteamCallResultCreateLobby;

};