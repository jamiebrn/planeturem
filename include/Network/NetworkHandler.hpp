#pragma once

#include <extlib/steam/steam_api.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Vector.hpp>

#include "Core/Camera.hpp"

#include "Network/Packet.hpp"
#include "Network/IPacketData.hpp"
#include "Network/PacketData/PacketDataIncludes.hpp"

#include "GUI/InventoryGUI.hpp"
#include "World/ChunkPosition.hpp"
#include "World/ChunkViewRange.hpp"
#include "Player/PlayerData.hpp"
#include "Player/NetworkPlayer.hpp"
#include "Player/Achievements.hpp"

class Game;
class ChatGUI;
class MainMenuGUI;

class NetworkHandler
{
public:
    NetworkHandler() = default;
    NetworkHandler(Game* game);
    void reset(Game* game);

    void startHostServer();
    
    void sendWorldJoinReply(std::string playerName, pl::Color bodyColor, pl::Color skinColor);

    void receiveMessages(ChatGUI& chatGUI, MainMenuGUI& mainMenuGUI);

    void update(float dt);

    void updateNetworkPlayers(float dt, const LocationState& locationState);

    void sendGameUpdates(float dt, const Camera& camera);
    void sendGameUpdatesToClients(float dt);
    void sendGameUpdatesToHost(const Camera& camera, float dt);

    void leaveLobby();

    EResult sendPacketToClientsAtLocation(const Packet& packet, int nSendFlags, int nRemoteChannel, const LocationState& locationState);

    EResult sendPacketToClients(const Packet& packet, int nSendFlags, int nRemoteChannel, std::unordered_set<uint64_t> exceptions = {});
    EResult sendPacketToClient(uint64_t steamID, const Packet& packet, int nSendFlags, int nRemoteChannel);
    EResult sendPacketToHost(const Packet& packet, int nSendFlags, int nRemoteChannel);

    // Sends packet to host if is client, or sends packet to all clients if is host
    EResult sendPacketToServer(const Packet& packet, int nSendFlags, int nRemoteChannel);

    void requestChunksFromHost(PlanetType planetType, std::vector<ChunkPosition>& chunks, bool forceRequest = false);

    void queueSendPlayerData();
    void sendPlayerData();

    bool canSendStructureRequest();
    void structureRequestSent();

    bool isMultiplayerGame() const;
    bool isLobbyHostOrSolo() const;
    bool getIsLobbyHost() const;
    bool isClient() const;
    int getNetworkPlayerCount() const;
    std::optional<uint64_t> getLobbyID() const;

    NetworkPlayer* getNetworkPlayer(uint64_t id);
    std::unordered_map<uint64_t, NetworkPlayer>& getNetworkPlayers();

    std::vector<Player*> getPlayersAtLocation(const LocationState& locationState, Player* thisPlayer);
    std::unordered_map<uint64_t, NetworkPlayer*> getNetworkPlayersAtLocation(const LocationState& locationState);

    std::vector<WorldObject*> getNetworkPlayersToDraw(const Camera& camera, const LocationState& locationState, pl::Vector2f playerPosition, float gameTime);

    std::vector<ChunkViewRange> getNetworkPlayersChunkViewRanges(PlanetType planetType);

    std::unordered_set<PlanetType> getPlayersPlanetTypeSet(std::optional<PlanetType> thisPlayerPlanetType);
    std::unordered_set<RoomType> getPlayersRoomDestTypeSet(std::optional<RoomType> thisPlayerRoomType);

    const std::string getPlayerName(uint64_t id);
    const std::string getPlayerPingLocation(uint64_t id);
    std::string getLocalPingLocation();

    const PlayerData* getSavedNetworkPlayerData(uint64_t id);
    void setSavedNetworkPlayerData(uint64_t id, const PlayerData& networkPlayerData);
    const std::unordered_map<uint64_t, PlayerData> getSavedNetworkPlayerDataMap();

    int getTotalBytesSent() const;
    int getTotalBytesReceived() const;
    float getByteSendRate(float dt) const;
    float getByteReceiveRate(float dt) const;

private:
    void processMessage(const SteamNetworkingMessage_t& message, const Packet& packet, ChatGUI& chatGUI, MainMenuGUI& mainMenuGUI);
    void processMessageAsHost(const SteamNetworkingMessage_t& message, const Packet& packet, ChatGUI& chatGUI);
    void processMessageAsClient(const SteamNetworkingMessage_t& message, const Packet& packet, ChatGUI& chatGUI, MainMenuGUI& mainMenuGUI);
    
    void callbackLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);
    
    void registerNetworkPlayer(uint64_t id, const std::string& name, const std::string& pingLocation, ChatGUI* chatGUI);
    void deleteNetworkPlayer(uint64_t id, ChatGUI* chatGUI);

    void handleChunkDatasFromHost(const PacketDataChunkDatas& chunkDatas);
    void handleChunkModifiedAlertsFromHost(const PacketDataChunkModifiedAlerts& chunkModifiedAlerts);

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

    int totalBytesSent;
    int totalBytesReceived;
    int totalBytesSentLast;
    int totalBytesReceivedLast;
    static constexpr float BYTE_RATE_SAMPLE_RATE = 0.2f;
    float byteRateSampleTime;
    float byteSendRate;
    float byteReceiveRate;

    static constexpr int MAX_LOBBY_PLAYER_COUNT = 8;

    float updateTick;
    int updateTickCount;
    static constexpr int MAX_UPDATE_TICK_COUNT = 2;
    static constexpr int NON_PLAYER_UPDATE_TICK = 1;

    std::unordered_map<uint64_t, NetworkPlayer> networkPlayers;
    std::unordered_map<uint64_t, PlayerData> networkPlayerDatasSaved;

    std::vector<PacketDataPlanetTravelRequest> planetTravelRequests;
    std::vector<PacketDataRoomTravelRequest> roomTravelRequests;

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