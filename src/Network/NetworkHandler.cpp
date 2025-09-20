#include "Network/NetworkHandler.hpp"
#include "Game.hpp"
#include "GUI/ChatGUI.hpp"
#include "GUI/MainMenuGUI.hpp"
#include "IO/Log.hpp"
#include "steam/steamclientpublic.h"

NetworkHandler::NetworkHandler(Game* game)
{
    if (game == nullptr)
    {
        Log::push("ERROR: NetworkHandler game ptr set to null\n");
    }

    reset(game);
}

void NetworkHandler::reset(Game* game)
{
    this->game = game;
    multiplayerGame = false;
    steamLobbyId = 0;
    isLobbyHost = false;
    lobbyHost = 0;
    networkPlayers.clear();
    networkPlayerDatasSaved.clear();

    totalBytesSent = 0;
    totalBytesReceived = 0;
    totalBytesSentLast = 0;
    totalBytesReceivedLast = 0;
    byteRateSampleTime = 0.0f;
    byteSendRate = 0.0f;
    byteReceiveRate = 0.0f;
    
    updateTick = 0.0f;
    updateTickCount = 0;

    sendPlayerDataQueued = false;
    sendPlayerDataQueueTime = 0.0f;

    structureEnterRequestCooldown = 0.0f;
}

void NetworkHandler::startHostServer()
{
    if (multiplayerGame)
    {
        return;
    }

    networkPlayers.clear();
    SteamAPICall_t steamAPICall = SteamMatchmaking()->CreateLobby(ELobbyType::k_ELobbyTypeFriendsOnly, MAX_LOBBY_PLAYER_COUNT);
    m_SteamCallResultCreateLobby.Set(steamAPICall, this, &NetworkHandler::callbackLobbyCreated);
}

void NetworkHandler::callbackLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure)
{
    if (pCallback->m_ulSteamIDLobby == 0)
    {
        Log::push("ERROR: Lobby creation failed\n");
        return;
    }

    Log::push("NETWORK: Created lobby " + std::to_string(pCallback->m_ulSteamIDLobby) + "\n");
    isLobbyHost = true;
    lobbyHost = SteamUser()->GetSteamID().ConvertToUint64();
    multiplayerGame = true;
}

void NetworkHandler::leaveLobby()
{
    if (!multiplayerGame)
    {
        return;
    }

    CSteamID steamLobbyIDSteam;
    steamLobbyIDSteam.SetFromUint64(steamLobbyId);

    if (isLobbyHost)
    {
        SteamMatchmaking()->SetLobbyJoinable(steamLobbyIDSteam, false);

        // Alert clients of host leaving
        Packet packet;
        packet.type = PacketType::HostQuit;

        for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
        {
            SteamNetworkingIdentity identity;
            identity.SetSteamID64(iter->first);
            sendPacketToClient(iter->first, packet, k_nSteamNetworkingSend_Reliable, 0);
            SteamNetworkingMessages()->CloseSessionWithUser(identity);
        }
    }
    else
    {
        SteamNetworkingIdentity hostIdentity;
        hostIdentity.SetSteamID64(lobbyHost);
        SteamNetworkingMessages()->CloseSessionWithUser(hostIdentity);
    }
    
    SteamMatchmaking()->LeaveLobby(steamLobbyIDSteam);
    isLobbyHost = false;
    multiplayerGame = false;
}

void NetworkHandler::sendWorldJoinReply(std::string playerName, pl::Color bodyColor, pl::Color skinColor)
{
    CSteamID lobbyHostSteam;
    lobbyHostSteam.SetFromUint64(lobbyHost);
    Log::push("NETWORK: Sending join reply to user " + std::string(SteamFriends()->GetFriendPersonaName(lobbyHostSteam)) + "\n");

    PacketDataJoinReply packetData;
    packetData.playerName = playerName;
    packetData.bodyColor = bodyColor;
    packetData.skinColor = skinColor;
    packetData.pingLocation = getLocalPingLocation();

    Packet packet;
    packet.set(packetData);
    sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
}

bool NetworkHandler::isMultiplayerGame() const
{
    return multiplayerGame;
}

bool NetworkHandler::isLobbyHostOrSolo() const
{
    if (!multiplayerGame)
    {
        return true;
    }

    return isLobbyHost;
}

bool NetworkHandler::getIsLobbyHost() const
{
    if (!multiplayerGame)
    {
        return false;
    }

    return isLobbyHost;
}

bool NetworkHandler::isClient() const
{
    return (multiplayerGame && !isLobbyHost);
}

int NetworkHandler::getNetworkPlayerCount() const
{
    return networkPlayers.size();
}

std::optional<uint64_t> NetworkHandler::getLobbyID() const
{
    if (!multiplayerGame)
    {
        return std::nullopt;
    }
    return steamLobbyId;
}

NetworkPlayer* NetworkHandler::getNetworkPlayer(uint64_t id)
{
    if (!networkPlayers.contains(id))
    {
        return nullptr;
    }
    return &networkPlayers.at(id);
}

std::unordered_map<uint64_t, NetworkPlayer>& NetworkHandler::getNetworkPlayers()
{
    return networkPlayers;
}

std::vector<Player*> NetworkHandler::getPlayersAtLocation(const LocationState& locationState, Player* thisPlayer)
{
    std::vector<Player*> players;

    if (thisPlayer)
    {
        players.push_back(thisPlayer);
    }

    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        // Not in same location as player
        if (iter->second.getPlayerData().locationState != locationState)
        {
            continue;
        }

        players.push_back(&iter->second);
    }

    return players;
}

std::unordered_map<uint64_t, NetworkPlayer*> NetworkHandler::getNetworkPlayersAtLocation(const LocationState& locationState)
{
    std::unordered_map<uint64_t, NetworkPlayer*> networkPlayersAtLocation;
    
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        // Not in same location as player
        if (iter->second.getPlayerData().locationState != locationState)
        {
            continue;
        }
    
        networkPlayersAtLocation[iter->first] = &iter->second;
    }
    
    return networkPlayersAtLocation;
}

std::vector<WorldObject*> NetworkHandler::getNetworkPlayersToDraw(const Camera& camera, const LocationState& locationState, pl::Vector2f playerPosition, float gameTime)
{
    std::vector<WorldObject*> networkPlayerObjects;

    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        // Not in same location as player
        if (iter->second.getPlayerData().locationState != locationState)
        {
            continue;
        }

        int worldSize = 0;

        // Translate position to wrap around world correctly, if required
        if (locationState.isOnPlanet())
        {
            // iter->second.applyWorldWrapTranslation(playerPosition, game->getChunkManager(locationState.getPlanetType()));
            const PlanetGenData& planetGenData = PlanetGenDataLoader::getPlanetGenData(locationState.getPlanetType());
            worldSize = planetGenData.worldSize;
        }

        std::vector<WorldObject*> playerWorldObjects = iter->second.getDrawWorldObjects(camera, worldSize, gameTime);

        networkPlayerObjects.insert(networkPlayerObjects.end(), playerWorldObjects.begin(), playerWorldObjects.end());
    }

    return networkPlayerObjects;
}

std::vector<ChunkViewRange> NetworkHandler::getNetworkPlayersChunkViewRanges(PlanetType planetType)
{
    std::vector<ChunkViewRange> chunkViewRanges;

    LocationState locationState;
    locationState.setPlanetType(planetType);

    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        if (iter->second.getPlayerData().locationState != locationState)
        {
            continue;
        }

        chunkViewRanges.push_back(iter->second.getChunkViewRange());
    }

    return chunkViewRanges;
}

std::unordered_set<PlanetType> NetworkHandler::getPlayersPlanetTypeSet(std::optional<PlanetType> thisPlayerPlanetType)
{
    std::unordered_set<PlanetType> planetTypeSet;
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        if (!iter->second.getPlayerData().locationState.isOnPlanet())
        {
            continue;
        }

        planetTypeSet.insert(iter->second.getPlayerData().locationState.getPlanetType());
    }

    if (thisPlayerPlanetType.has_value() && thisPlayerPlanetType.value() >= 0)
    {
        planetTypeSet.insert(thisPlayerPlanetType.value());
    }

    return planetTypeSet;
}

std::unordered_set<RoomType> NetworkHandler::getPlayersRoomDestTypeSet(std::optional<RoomType> thisPlayerRoomType)
{
    std::unordered_set<RoomType> roomDestTypeSet;
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        if (!iter->second.getPlayerData().locationState.isInRoomDest())
        {
            continue;
        }

        roomDestTypeSet.insert(iter->second.getPlayerData().locationState.getRoomDestType());
    }

    if (thisPlayerRoomType.has_value())
    {
        if (thisPlayerRoomType.value() >= 0)
        {
            roomDestTypeSet.insert(thisPlayerRoomType.value());
        }
    }

    return roomDestTypeSet;
}

const std::string NetworkHandler::getPlayerName(uint64_t id)
{
    if (!networkPlayers.contains(id))
    {
        return "";
    }
    return networkPlayers[id].getPlayerData().name;
}

const std::string NetworkHandler::getPlayerPingLocation(uint64_t id)
{
    if (!networkPlayers.contains(id))
    {
        return "";
    }
    return networkPlayers[id].getPlayerData().pingLocation;
}

std::string NetworkHandler::getLocalPingLocation()
{
    SteamNetworkPingLocation_t pingLocation;
    if (!SteamNetworkingUtils()->GetLocalPingLocation(pingLocation))
    {
        return "";
    }
    
    char pingLocationStrBuffer[k_cchMaxSteamNetworkingPingLocationString];
    SteamNetworkingUtils()->ConvertPingLocationToString(pingLocation, pingLocationStrBuffer, k_cchMaxSteamNetworkingPingLocationString);
    return pingLocationStrBuffer;
}

const PlayerData* NetworkHandler::getSavedNetworkPlayerData(uint64_t id)
{
    if (!networkPlayerDatasSaved.contains(id))
    {
        return nullptr;
    }
    return &networkPlayerDatasSaved.at(id);
}

void NetworkHandler::setSavedNetworkPlayerData(uint64_t id, const PlayerData& networkPlayerData)
{
    networkPlayerDatasSaved[id] = networkPlayerData;
}

const std::unordered_map<uint64_t, PlayerData> NetworkHandler::getSavedNetworkPlayerDataMap()
{
    return networkPlayerDatasSaved;
}

void NetworkHandler::registerNetworkPlayer(uint64_t id, const std::string& name, const std::string& pingLocation, ChatGUI* chatGUI)
{
    if (!multiplayerGame)
    {
        return;
    }

    // Alert connected players
    if (isLobbyHost)
    {
        PacketDataPlayerJoined packetData;
        packetData.id = id;
        packetData.name = name;
        packetData.pingLocation = pingLocation;
        Packet packet;
        packet.set(packetData);

        for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
        {
            sendPacketToClient(iter->first, packet, k_nSteamNetworkingSend_Reliable, 0);
        }
    }

    if (chatGUI)
    {
        // InventoryGUI::pushItemPopup(ItemCount(0, 1), false, std::string(SteamFriends()->GetFriendPersonaName(id)) + " joined");
        PacketDataChatMessage chatMessage;
        chatMessage.userId = std::nullopt;
        chatMessage.message = name + " joined";
        chatGUI->addChatMessage(*this, chatMessage);
    }

    networkPlayers[id] = NetworkPlayer(pl::Vector2f(0, 0));
    networkPlayers[id].getPlayerData().pingLocation = pingLocation;
}

void NetworkHandler::deleteNetworkPlayer(uint64_t id, ChatGUI* chatGUI)
{
    if (!multiplayerGame)
    {
        return;
    }

    if (!networkPlayers.contains(id))
    {
        return;
    }
    
    // Alert connected players and save player data
    if (isLobbyHost)
    {
        networkPlayerDatasSaved[id] = networkPlayers[id].getPlayerData();

        Packet packet;
        packet.type = PacketType::PlayerDisconnected;
        packet.data.resize(sizeof(id));
        memcpy(packet.data.data(), &id, sizeof(id));
    
        for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
        {
            sendPacketToClient(iter->first, packet, k_nSteamNetworkingSend_Reliable, 0);
        }
    }

    if (chatGUI)
    {
        // InventoryGUI::pushItemPopup(ItemCount(0, 1), false, std::string(SteamFriends()->GetFriendPersonaName(id)) + " disconnected");
        PacketDataChatMessage chatMessage;
        chatMessage.userId = std::nullopt;
        chatMessage.message = networkPlayers.at(id).getPlayerData().name + " disconnected";
        chatGUI->addChatMessage(*this, chatMessage);
    }

    networkPlayers.erase(id);
}

void NetworkHandler::callbackLobbyJoinRequested(GameLobbyJoinRequested_t* pCallback)
{
    SteamMatchmaking()->JoinLobby(pCallback->m_steamIDLobby);
    multiplayerGame = true;
}

void NetworkHandler::callbackLobbyEnter(LobbyEnter_t* pCallback)
{
    steamLobbyId = pCallback->m_ulSteamIDLobby;
    Log::push("NETWORK: Joined lobby " + std::to_string(steamLobbyId) + "\n");
    multiplayerGame = true;
}

void NetworkHandler::callbackLobbyUpdated(LobbyChatUpdate_t* pCallback)
{
    SteamNetworkingIdentity userIdentity;
    userIdentity.SetSteamID64(pCallback->m_ulSteamIDUserChanged);
    
    if (isLobbyHost)
    {
        if (pCallback->m_rgfChatMemberStateChange & k_EChatMemberStateChangeEntered)
        {
            PacketDataJoinQuery packetData;
            packetData.requiresNameInput = !networkPlayerDatasSaved.contains(userIdentity.GetSteamID64());
            packetData.gameDataHash = game->getGameDataHash();

            Packet packet;
            packet.set(packetData);

            EResult result = sendPacketToClient(userIdentity.GetSteamID64(), packet, k_nSteamNetworkingSend_Reliable, 0);
            if (result == EResult::k_EResultOK)
            {
                Log::push("NETWORK: Sent join query successfully\n");
            }
            else if (result == EResult::k_EResultNoConnection)
            {
                Log::push("ERROR: Could not send join query\n");
            }
        }
        else
        {
            deleteNetworkPlayer(pCallback->m_ulSteamIDUserChanged, &game->getChatGUI());
        }
    }
    else
    {
        // Test for host disconnect
        if (multiplayerGame)
        {
            // Test while in game
            if (pCallback->m_ulSteamIDUserChanged == lobbyHost)
            {
                game->quitWorld();
            }
        }
        else
        {
            // Test while in lobby, but not in game (rare case)
            CSteamID steamLobbyIdSteam;
            steamLobbyIdSteam.SetFromUint64(steamLobbyId);
            if (pCallback->m_ulSteamIDUserChanged == SteamMatchmaking()->GetLobbyOwner(steamLobbyIdSteam).ConvertToUint64())
            {
                leaveLobby();
            }
        }
    }
}

void NetworkHandler::callbackMessageSessionRequest(SteamNetworkingMessagesSessionRequest_t* pCallback)
{
    SteamNetworkingMessages()->AcceptSessionWithUser(pCallback->m_identityRemote);
}

void NetworkHandler::update(float dt)
{
    if (sendPlayerDataQueued)
    {
        sendPlayerDataQueueTime = std::max(sendPlayerDataQueueTime - dt, 0.0f);
        if (sendPlayerDataQueueTime <= 0.0f)
        {
            sendPlayerData();
        }
    }

    structureEnterRequestCooldown = std::max(structureEnterRequestCooldown - dt, 0.0f);

    byteRateSampleTime += dt;
    if (byteRateSampleTime >= BYTE_RATE_SAMPLE_RATE)
    {
        byteRateSampleTime = 0.0f;
        byteSendRate = (totalBytesSent - totalBytesSentLast) / BYTE_RATE_SAMPLE_RATE;
        byteReceiveRate = (totalBytesReceived - totalBytesReceivedLast) / BYTE_RATE_SAMPLE_RATE;
        totalBytesSentLast = totalBytesSent;
        totalBytesReceivedLast = totalBytesReceived;
    }
}

void NetworkHandler::updateNetworkPlayers(float dt, const LocationState& locationState)
{
    for (auto& networkPlayerPair : networkPlayers)
    {
        if (networkPlayerPair.second.getPlayerData().locationState != locationState)
        {
            continue;
        }

        networkPlayerPair.second.updateNetworkPlayer(dt, *game);
    }
}

void NetworkHandler::receiveMessages(ChatGUI& chatGUI, MainMenuGUI& mainMenuGUI)
{
    static constexpr int MAX_MESSAGES = 10;

    SteamNetworkingMessage_t* messages[MAX_MESSAGES];

    while (true)
    {
        int messageCount = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, messages, MAX_MESSAGES);

        if (messageCount <= 0)
        {
            break;
        }
    
        for (int i = 0; i < messageCount; i++)
        {
            Packet packet;
            packet.deserialise((char*)messages[i]->GetData(), messages[i]->GetSize());
    
            processMessage(*messages[i], packet, chatGUI, mainMenuGUI);

            totalBytesReceived += messages[i]->GetSize();
            // Log::push("___DEBUG___: Received packet of size {} bytes, type {}\n", messages[i]->GetSize(), packet.type);
    
            messages[i]->Release();
        }
    }
}

void NetworkHandler::processMessage(const SteamNetworkingMessage_t& message, const Packet& packet, ChatGUI& chatGUI, MainMenuGUI& mainMenuGUI)
{
    // Process packet
    if (isLobbyHost)
    {
        processMessageAsHost(message, packet, chatGUI);
    }
    else
    {
        processMessageAsClient(message, packet, chatGUI, mainMenuGUI);
    }
    
    switch (packet.type)
    {
        case PacketType::PlayerCharacterInfo:
        {
            if (isLobbyHost)
            {
                if (!networkPlayers.contains(message.m_identityPeer.GetSteamID64()))
                {
                    // registerNetworkPlayer(message.m_identityPeer.GetSteamID64());
                    Log::push(("ERROR: Received player character info for unregistered player ID " + std::to_string(message.m_identityPeer.GetSteamID64()) + "\n").c_str());
                }
            }
    
            PacketDataPlayerCharacterInfo packetData;
            packetData.deserialise(packet.data);
            packetData.applyPingEstimate(getPlayerPingLocation(message.m_identityPeer.GetSteamID64()));
    
            if (networkPlayers.contains(packetData.userID))
            {
                // std::string playerName = SteamFriends()->GetFriendPersonaName(CSteamID(packetData.userID));
    
                // Translate player position to wrap around world, relative to player
                // pl::Vector2f playerPos = game->getChunkManager().translatePositionAroundWorld(pl::Vector2f(packetData.positionX, packetData.positionY),
                //     game->getPlayer().getPosition());
                // packetData.positionX = playerPos.x;
                // packetData.positionY = playerPos.y;
    
                networkPlayers[packetData.userID].setNetworkPlayerCharacterInfo(packetData);

                // Update with ping time
                // networkPlayers[packetData.userID].updateNetworkPlayer(packetData.pingTime, *game);
            }
            break;
        }
        case PacketType::PlayerData:
        {
            PacketDataPlayerData packetData;
            packetData.deserialise(packet.data);
            if (networkPlayers.contains(packetData.userID))
            {
                networkPlayers[packetData.userID].setPlayerData(packetData.playerData);
                Log::push(("NETWORK: Received player data for network player " + std::to_string(packetData.userID) + " (" +
                packetData.playerData.name + ")\n").c_str());
            }
            else
            {
                Log::push(("WARNING: Received player data for unregistered network player " + std::to_string(packetData.userID) + " (" +
                    packetData.playerData.name + ")\n").c_str());
            }

            // If host, save data and redistribute
            if (isLobbyHost)
            {
                networkPlayerDatasSaved[packetData.userID] = packetData.playerData;
                sendPlayerDataToClients(packetData);
            }
            break;
        }
        case PacketType::ChatMessage:
        {
            PacketDataChatMessage packetData;
            packetData.deserialise(packet.data);

            // Forward to clients if host
            if (isLobbyHost)
            {
                sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
            }

            chatGUI.addChatMessage(*this, packetData);
            break;
        }
        case PacketType::ObjectHit:
        {
            PacketDataObjectHit packetData;
            packetData.deserialise(packet.data);
            bool sentFromHost = !isLobbyHost;
            game->hitObject(packetData.objectHit.chunk, packetData.objectHit.tile, packetData.damage, packetData.planetType, sentFromHost, packetData.userId);
            break;
        }
        case PacketType::ObjectBuilt:
        {
            PacketDataObjectBuilt packetData;
            packetData.deserialise(packet.data);
            bool sentFromHost = !isLobbyHost;
            game->buildObject(packetData.objectReference.chunk, packetData.objectReference.tile, packetData.objectType, packetData.planetType,
                sentFromHost, packetData.userId.has_value(), packetData.userId);
            break;
        }
        case PacketType::LandPlaced:
        {
            PacketDataLandPlaced packetData;
            packetData.deserialise(packet.data);
            if (!game->isLocationStateInitialised(LocationState::createFromPlanetType(packetData.planetType)))
            {
                break;
            }

            // Provide NetworkHandler pointer when placing land only if we are host,
            // as client receiving LandPlaced packet does not require networking (assumes to be truth)
            NetworkHandler* networkHandlerParam = isLobbyHost ? this : nullptr;
            game->getChunkManager(packetData.planetType).placeLand(packetData.chunk, packetData.tile, networkHandlerParam);
            break;
        }
        case PacketType::ItemPickupsCreated:
        {
            PacketDataItemPickupsCreated packetData;
            packetData.deserialise(packet.data);

            if (!game->isLocationStateInitialised(packetData.locationState))
            {
                if (isLobbyHost)
                {
                    Log::push(("ERROR: Attempted to create item pickups from client on null planet type " +
                        std::to_string(packetData.locationState.getPlanetType()) + "\n").c_str());
                }
                break;
            }
            
            // Create item pickups sent
            for (auto& itemPickupPair : packetData.createdPickups)
            {
                Chunk* chunkPtr = game->getChunkManager(packetData.locationState.getPlanetType()).getChunk(itemPickupPair.first.chunk);
                if (!chunkPtr)
                {
                    Log::push("ERROR: Failed to create item pickup sent from host in null chunk (" + std::to_string(itemPickupPair.first.chunk.x) +
                        ", " + std::to_string(itemPickupPair.first.chunk.y) + ")\n");
                    continue;
                }
    
                // Denormalise pickup position from chunk-relative to world position
                itemPickupPair.second.setPosition(itemPickupPair.second.getPosition() + chunkPtr->getWorldPosition());
    
                chunkPtr->addItemPickup(itemPickupPair.second, itemPickupPair.first.id);
            }
    
            // If host, redistribute pickups created message to clients
            if (isLobbyHost)
            {
                sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
            }
            break;
        }
        case PacketType::ItemPickupCollected:
        {
            PacketDataItemPickupCollected packetData;
            packetData.deserialise(packet.data);
            
            // If host, redistribute to clients (except sending player)
            if (isLobbyHost)
            {
                sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0, {message.m_identityPeer.GetSteamID64()});
                
                if (!game->isLocationStateInitialised(packetData.locationState))
                {
                    Log::push(("ERROR: Attempted to delete item pickups from client on null planet type " +
                        std::to_string(packetData.locationState.getPlanetType()) + "\n").c_str());
                    break;
                }
    
                Chunk* chunkPtr = game->getChunkManager(packetData.locationState.getPlanetType()).getChunk(packetData.pickup.chunk);
                if (chunkPtr)
                {
                    ItemPickup* itemPickupPtr = chunkPtr->getItemPickup(packetData.pickup.id);
                    if (itemPickupPtr)
                    {
                        // Give item to client
                        PacketDataInventoryAddItem itemPacketData;
                        itemPacketData.itemType = itemPickupPtr->getItemType();
                        itemPacketData.amount = std::min(packetData.count, itemPickupPtr->getItemCount());
                        Packet itemPacket;
                        itemPacket.set(itemPacketData);
    
                        sendPacketToClient(message.m_identityPeer.GetSteamID64(), itemPacket, k_nSteamNetworkingSend_Reliable, 0);
                    }
                }
            }

            if (game->isLocationStateInitialised(packetData.locationState))
            {
                // Delete pickup from chunk manager, regardless of whether we are host or client
                game->getChunkManager(packetData.locationState.getPlanetType()).reduceItemPickupCount(packetData.pickup, packetData.count);
            }
            break;
        }
        case PacketType::InventoryAddItem:
        {
            PacketDataInventoryAddItem packetData;
            packetData.deserialise(packet.data);
            game->getInventory().addItem(packetData.itemType, packetData.amount, true);
            queueSendPlayerData();
            break;
        }
        case PacketType::ChestOpened:
        {
            PacketDataChestOpened packetData;
            packetData.deserialise(packet.data);
            if (isLobbyHost)
            {
                game->openChestForClient(packetData);
            }
            else
            {
                game->openChestFromHost(packetData);
            }
            break;
        }
        case PacketType::ChestClosed:
        {
            PacketDataChestClosed packetData;
            packetData.deserialise(packet.data);
            if (isLobbyHost)
            {
                // Redistribute to clients
                Packet packetToSend;
                packetToSend.set(packetData);
                sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
            }
    
            // Close chest for us (if using the chest, close inventory etc, or simply close animation if other user closed)
            game->closeChest(packetData.chestObject, packetData.locationState, true, packetData.userID);
            break;
        }
        case PacketType::LandmarkModified:
        {
            PacketDataLandmarkModified packetData;
            packetData.deserialise(packet.data);

            LocationState packetLocationState = LocationState::createFromPlanetType(packetData.planetType);

            if (!game->isLocationStateInitialised(packetLocationState))
            {
                if (isLobbyHost)
                {
                    // Should not be receiving uninitialised planet type landmark modified packets when host - all active planets should be loaded
                    Log::push("ERROR: Received landmark modified packet of uninitialised planet type {}\n", packetData.planetType);
                }
                break;
            }

            LandmarkObject* landmarkObject = game->getObjectFromLocation<LandmarkObject>(packetData.landmarkObjectReference, packetLocationState);

            if (!landmarkObject)
            {
                if (isLobbyHost)
                {
                    Log::push("ERROR: Received landmark modified packet for null object ({}, {}, {}, {})\n",
                        packetData.landmarkObjectReference.chunk.x, packetData.landmarkObjectReference.chunk.y,
                        packetData.landmarkObjectReference.tile.x, packetData.landmarkObjectReference.tile.y);
                }
                else
                {
                    // If client and landmark missing, request chunk from host as is likely we are out of date with world data
                    std::vector<ChunkPosition> chunksToRequest = {packetData.landmarkObjectReference.chunk};
                    requestChunksFromHost(packetData.planetType, chunksToRequest, true);
                }
                break;
            }

            landmarkObject->setLandmarkColour(packetData.newColorA, packetData.newColorB);

            game->getLandmarkManager(packetData.planetType).addLandmark(packetData.landmarkObjectReference);

            // Forward to clients
            if (isLobbyHost)
            {
                sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
            }
            break;
        }
        case PacketType::RocketInteraction:
        {
            PacketDataRocketInteraction packetData;
            packetData.deserialise(packet.data);

            if (!game->isLocationStateInitialised(packetData.locationState))
            {
                if (isLobbyHost)
                {
                    Log::push("ERROR: Received rocket interaction for null location\n");
                }
                break;
            }

            RocketObject* rocketObject = game->getObjectFromLocation<RocketObject>(packetData.rocketObjectReference, packetData.locationState);

            if (!rocketObject)
            {
                Log::push("ERROR: Received rocket interaction for null rocket\n");
                break;
            }

            switch (packetData.interactionType)
            {
                case PacketDataRocketInteraction::InteractionType::Enter:
                {
                    rocketObject->enter();
                    break;
                }
                case PacketDataRocketInteraction::InteractionType::Exit:
                {
                    rocketObject->exit();
                    break;
                }
                case PacketDataRocketInteraction::InteractionType::FlyUp:
                {
                    rocketObject->startFlyingUpwards(*game, packetData.locationState, nullptr);
                    break;
                }
                case PacketDataRocketInteraction::InteractionType::FlyDown:
                {
                    rocketObject->startFlyingDownwards(*game, packetData.locationState, nullptr, false);
                    break;
                }
            }

            // Redistribute to clients (except sender) if host
            if (isLobbyHost)
            {
                sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0, {message.m_identityPeer.GetSteamID64()});
            }
            break;
        }
        default:
            break;
    }
}

void NetworkHandler::processMessageAsHost(const SteamNetworkingMessage_t& message, const Packet& packet, ChatGUI& chatGUI)
{
    switch (packet.type)
    {
        case PacketType::JoinReply:
        {
            const char* steamName = SteamFriends()->GetFriendPersonaName(message.m_identityPeer.GetSteamID());
            Log::push("NETWORK: Player joined: " + std::string(steamName) + " (" + std::to_string(message.m_identityPeer.GetSteamID64()) + ")\n");

            PacketDataJoinReply packetDataJoinReply;
            packetDataJoinReply.deserialise(packet.data);
            
            // Send world info
            PacketDataJoinInfo packetData;
            packetData.seed = game->getPlanetSeed();
            packetData.gameTime = game->getGameTime();
            packetData.time = game->getDayCycleManager().getCurrentTime();
            packetData.day = game->getDayCycleManager().getCurrentDay();
            // packetData.planetName = PlanetGenDataLoader::getPlanetGenData(game->getChunkManager().getPlanetType()).name;
    
            // packetData.chestDataPool = game->getChestDataPool();

            bool newPlayer = false;

            // Initialise new player data
            if (!networkPlayerDatasSaved.contains(message.m_identityPeer.GetSteamID64()))
            {
                // Player data does not exist - initialise
                
                // Get name from join reply packet
                
                networkPlayerDatasSaved[message.m_identityPeer.GetSteamID64()] = PlayerData();

                PlayerData& playerData = networkPlayerDatasSaved[message.m_identityPeer.GetSteamID64()];

                playerData.name = packetDataJoinReply.playerName;
                playerData.bodyColor = packetDataJoinReply.bodyColor;
                playerData.skinColor = packetDataJoinReply.skinColor;

                playerData.locationState.setPlanetType(PlanetGenDataLoader::getPlanetTypeFromName("Earthlike"));

                playerData.inventory = InventoryData(32);
                playerData.inventory.giveStartingItems();
                playerData.armourInventory = InventoryData(3);

                newPlayer = true;
            }
            else
            {
                // Data exists - use stored name
                packetDataJoinReply.playerName = networkPlayerDatasSaved[message.m_identityPeer.GetSteamID64()].name;
            }

            // Load planet if required
            PlayerData& playerData = networkPlayerDatasSaved[message.m_identityPeer.GetSteamID64()];
            if (playerData.locationState.isOnPlanet())
            {    
                game->loadPlanet(playerData.locationState.getPlanetType());

                if (playerData.locationState.isInStructure())
                {
                    packetData.inStructureRoomType = game->
                        getStructureRoomPool(playerData.locationState.getPlanetType()).getRoom(playerData.locationState.getInStructureID()).getRoomType();
                }
                else if (newPlayer)
                {
                    // Find spawn for player
                    ChunkPosition playerSpawnChunk = game->getChunkManager(playerData.locationState.getPlanetType()).findValidSpawnChunk(2);
                    playerData.position.x = playerSpawnChunk.x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
                    playerData.position.y = playerSpawnChunk.y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
                }
            }
            else if (playerData.locationState.isInRoomDest())
            {
                game->loadRoomDest(playerData.locationState.getRoomDestType());
            }

            // Send player data
            packetData.playerData = playerData;

            PlayerData hostPlayerData = game->createPlayerData();
            hostPlayerData.pingLocation = getLocalPingLocation();
            packetData.currentPlayerDatas[SteamUser()->GetSteamID().ConvertToUint64()] = hostPlayerData;

            for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
            {
                packetData.currentPlayerDatas[iter->first] = iter->second.getPlayerData();
            }
            
            // Send landmark and worldmap data if required
            if (packetData.playerData.locationState.isOnPlanet())
            {
                packetData.landmarks = PacketDataLandmarks();
                packetData.landmarks->planetType = packetData.playerData.locationState.getPlanetType();
                packetData.landmarks->landmarkManager = game->getLandmarkManager(packetData.landmarks->planetType);

                packetData.worldMap.setMapTextureData(game->getChunkManager(packetData.playerData.locationState.getPlanetType()).getWorldMap().getMapTextureData());
            }

            registerNetworkPlayer(message.m_identityPeer.GetSteamID64(), packetDataJoinReply.playerName, packetDataJoinReply.pingLocation, &chatGUI);
            
            Packet packetToSend;
            packetToSend.set(packetData, true);
            sendPacketToClient(message.m_identityPeer.GetSteamID64(), packetToSend, k_nSteamNetworkingSend_Reliable, 0);
            break;
        }
        case PacketType::ItemPickupsCreateRequest:
        {
            PacketDataItemPickupsCreateRequest packetData;
            packetData.deserialise(packet.data);

            PacketDataItemPickupsCreated pickupsCreatedPacketData;
            pickupsCreatedPacketData.locationState = packetData.locationState;

            // Create item pickups requested
            for (auto& request : packetData.pickupRequests)
            {
                Chunk* chunkPtr = game->getChunkManager(pickupsCreatedPacketData.locationState.getPlanetType()).getChunk(request.chunk);
                if (!chunkPtr)
                {
                    Log::push("ERROR: Failed to create item pickup requested from client in null chunk (" + std::to_string(request.chunk.x) +
                        ", " + std::to_string(request.chunk.y) + ")\n");
                    continue;
                }

                // Denormalise pickup position from chunk-relative to world position
                ItemPickup pickup(request.positionRelative + chunkPtr->getWorldPosition(), request.itemType, game->getGameTime(), request.count);

                uint64_t itemPickupID = chunkPtr->addItemPickup(pickup);
                ItemPickup* itemPickupPtr = chunkPtr->getItemPickup(itemPickupID);
                if (!itemPickupPtr)
                {
                    Log::push("ERROR: Failed to create null item pickup requested from client in chunk (" + std::to_string(request.chunk.x) +
                        ", " + std::to_string(request.chunk.y) + ")\n");
                    continue;
                }

                // Normalise to chunk-relative before sending to clients
                ItemPickup itemPickup = *itemPickupPtr;
                itemPickup.setPosition(itemPickup.getPosition() - chunkPtr->getWorldPosition());

                pickupsCreatedPacketData.createdPickups.push_back({ItemPickupReference{request.chunk, itemPickupID}, itemPickup});
            }

            Packet pickupsCreatedPacket;
            pickupsCreatedPacket.set(pickupsCreatedPacketData, true);

            // Alert clients of pickups created
            sendPacketToClients(pickupsCreatedPacket, k_nSteamNetworkingSend_Reliable, 0);
            break;
        }
        case PacketType::MeleeRequest:
        {
            PacketDataMeleeRequest packetData;
            packetData.deserialise(packet.data);
            game->testMeleeCollision(LocationState::createFromPlanetType(packetData.planetType), packetData.hitRects, packetData.hitOrigin);
            break;
        }
        case PacketType::ChunkRequests:
        {
            PacketDataChunkRequests packetData;
            packetData.deserialise(packet.data);
            game->handleChunkRequestsFromClient(packetData, message.m_identityPeer);
            break;
        }
        case PacketType::ChestDataModified:
        {
            PacketDataChestDataModified packetData;
            packetData.deserialise(packet.data);
            game->getChestDataPool(packetData.locationState).overwriteChestData(packetData.chestID, packetData.chestData);
            Log::push(("NETWORK: Received chest data from " + std::string(SteamFriends()->GetFriendPersonaName(message.m_identityPeer.GetSteamID())) + "\n").c_str());
            break;
        }
        case PacketType::ProjectileCreateRequest:
        {
            PacketDataProjectileCreateRequest packetData;
            packetData.deserialise(packet.data);
            packetData.applyPingEstimate(getPlayerPingLocation(message.m_identityPeer.GetSteamID64()));

            const ToolData& weaponData = ToolDataLoader::getToolData(packetData.weaponType);
            Projectile projectile(packetData.projectile.getPosition(), packetData.projectile.getVelocity(),
                packetData.projectile.getType(), weaponData.projectileDamageMult, HitLayer::Entity);

            game->getProjectileManager(packetData.planetType).addProjectile(projectile, packetData.weaponType);
            break;
        }
        case PacketType::BossSpawnCheck:
        {
            PacketDataBossSpawnCheck packetData;
            packetData.deserialise(packet.data);

            if (!game->isLocationStateInitialised(LocationState::createFromPlanetType(packetData.planetType)))
            {
                Log::push("ERROR: Received boss spawn check for uninitialised location\n");
                break;
            }

            if (!game->canPlayerSpawnBoss(packetData.planetType, packetData.bossSpawnItem, *getNetworkPlayer(message.m_identityPeer.GetSteamID64())))
            {
                break;
            }

            // Player can spawn boss - send check to ensure still has item so they can request boss spawn
            PacketDataBossSpawnCheckReply packetDataReply;
            packetDataReply.planetType = packetData.planetType;
            packetDataReply.bossSpawnItem = packetData.bossSpawnItem;

            Packet packetReply(packetDataReply);
            sendPacketToClient(message.m_identityPeer.GetSteamID64(), packetDataReply, k_nSteamNetworkingSend_Reliable, 0);
            break;
        }
        case PacketType::BossSpawnRequest:
        {
            PacketDataBossSpawnCheck packetData;
            packetData.deserialise(packet.data);

            if (!game->isLocationStateInitialised(LocationState::createFromPlanetType(packetData.planetType)))
            {
                Log::push("ERROR: Received boss spawn REQUEST for uninitialised location\n");
                break;
            }

            game->attemptSpawnBoss(packetData.planetType, packetData.bossSpawnItem, *getNetworkPlayer(message.m_identityPeer.GetSteamID64()));
            break;
        }
        case PacketType::RocketEnterRequest:
        {
            PacketDataRocketEnterRequest packetData;
            packetData.deserialise(packet.data);

            if (!game->isLocationStateInitialised(packetData.locationState))
            {
                Log::push("ERROR: Received rocket enter request for uninitialised location\n");
                break;
            }

            RocketObject* rocketObject = game->getObjectFromLocation<RocketObject>(packetData.rocketObjectReference, packetData.locationState);
            if (!rocketObject)
            {
                Log::push("ERROR: Received rocket enter request for null rocket\n");
            }

            // Accept enter request if rocket is not already entered
            if (!rocketObject->isEntered())
            {
                PacketDataRocketEnterReply rocketEnterReply;
                rocketEnterReply.locationState = packetData.locationState;
                rocketEnterReply.rocketObjectReference = packetData.rocketObjectReference;
                Packet packetRocketEnterReply(rocketEnterReply);
                sendPacketToClient(message.m_identityPeer.GetSteamID64(), packetRocketEnterReply, k_nSteamNetworkingSend_Reliable, 0);
            }
            break;
        }
        case PacketType::PlanetTravelRequest:
        {
            PacketDataPlanetTravelRequest packetData;
            packetData.deserialise(packet.data);
            packetData.userId = message.m_identityPeer.GetSteamID64();
            planetTravelRequests.push_back(packetData);
            break;
        }
        case PacketType::RoomTravelRequest:
        {
            PacketDataRoomTravelRequest packetData;
            packetData.deserialise(packet.data);
            packetData.userId = message.m_identityPeer.GetSteamID64();
            roomTravelRequests.push_back(packetData);
            break;
        }
        case PacketType::StructureEnterRequest:
        {
            PacketDataStructureEnterRequest packetData;
            packetData.deserialise(packet.data);

            PacketDataStructureEnterReply packetDataReply;
            std::optional<uint32_t> structureID = game->initialiseStructureOrGet(packetData.planetType, packetData.chunkPos,
                &packetDataReply.structureEntrancePos, &packetDataReply.roomType);
            if (structureID.has_value())
            {
                Log::push(("NETWORK: Sending structure enter reply to " +
                    std::string(SteamFriends()->GetFriendPersonaName(message.m_identityPeer.GetSteamID())) + "\n").c_str());

                packetDataReply.structureID = structureID.value();
                packetDataReply.planetType = packetData.planetType;
                packetDataReply.chunkPos = packetData.chunkPos;
                Packet replyPacket;
                replyPacket.set(packetDataReply);
                sendPacketToClient(message.m_identityPeer.GetSteamID64(), replyPacket, k_nSteamNetworkingSend_Reliable, 0);
            }
            break;
        }
        default:
            break;
    }
}

void NetworkHandler::processMessageAsClient(const SteamNetworkingMessage_t& message, const Packet& packet, ChatGUI& chatGUI, MainMenuGUI& mainMenuGUI)
{
    switch (packet.type)
    {
        case PacketType::JoinQuery:
        {
            PacketDataJoinQuery packetData;
            packetData.deserialise(packet.data);

            if (packetData.gameDataHash != game->getGameDataHash())
            {
                leaveLobby();
                mainMenuGUI.setErrorMessage("Failed to join game (game data mismatch)");
                break;
            }
            
            lobbyHost = message.m_identityPeer.GetSteamID64();
            isLobbyHost = false;

            game->joinedLobby(packetData.requiresNameInput);
            break;
        }
        case PacketType::JoinInfo:
        {
            // Deserialise packet data
            PacketDataJoinInfo packetData;
            packetData.deserialise(packet.data);

            // Set lobby host
            lobbyHost = message.m_identityPeer.GetSteamID64();
            isLobbyHost = false;
            
            multiplayerGame = true;

            networkPlayers.clear();
            networkPlayerDatasSaved.clear();
            for (const auto& networkPlayerDataPair : packetData.currentPlayerDatas)
            {
                CSteamID steamID;
                steamID.SetFromUint64(networkPlayerDataPair.first);

                registerNetworkPlayer(networkPlayerDataPair.first, networkPlayerDataPair.second.name, networkPlayerDataPair.second.pingLocation, nullptr);
                Log::push("NETWORK: Registered existing player " + std::string(SteamFriends()->GetFriendPersonaName(steamID)) + "\n");
                networkPlayers[networkPlayerDataPair.first].setPlayerData(networkPlayerDataPair.second);
            }

            // Load into world
            game->joinWorld(packetData);
            break;
        }
        case PacketType::PlayerJoined:
        {
            PacketDataPlayerJoined packetData;
            packetData.deserialise(packet.data);
            registerNetworkPlayer(packetData.id, packetData.name, packetData.pingLocation, &chatGUI);
            break;
        }
        case PacketType::PlayerDisconnected:
        {
            uint64_t id;
            memcpy(&id, packet.data.data(), sizeof(id));
            deleteNetworkPlayer(id, &chatGUI);
            break;
        }
        case PacketType::HostQuit:
        {
            game->quitWorld();
            break;
        }
        case PacketType::ServerInfo:
        {
            PacketDataServerInfo serverInfo;
            serverInfo.deserialise(packet.data);
            serverInfo.applyPingEstimate(getPlayerPingLocation(message.m_identityPeer.GetSteamID64()));

            game->setGameTime(serverInfo.gameTime);
            game->getDayCycleManager(true).setCurrentDay(serverInfo.day);
            game->getDayCycleManager(true).setCurrentTime(serverInfo.time);
            break;
        }
        case PacketType::ObjectDestroyed:
        {
            PacketDataObjectDestroyed packetData;
            packetData.deserialise(packet.data);
            game->destroyObjectFromHost(packetData.objectReference.chunk, packetData.objectReference.tile, packetData.planetType);
            break;
        }
        case PacketType::ChunkDatas:
        {
            PacketDataChunkDatas packetData;
            packetData.deserialise(packet.data);
            if (game->getLocationState().getPlanetType() != packetData.planetType)
            {
                Log::push("ERROR: Received chunk data for incorrect planet type {}\n", packetData.planetType);
                break;
            }
            handleChunkDatasFromHost(packetData);
            break;
        }
        case PacketType::ChunkModifiedAlerts:
        {
            PacketDataChunkModifiedAlerts packetData;
            packetData.deserialise(packet.data);
            if (game->getLocationState().getPlanetType() != packetData.planetType)
            {
                Log::push("ERROR: Received chunk modified alert for incorrect planet type {}\n", packetData.planetType);
                break;
            }
            handleChunkModifiedAlertsFromHost(packetData);
            break;
        }
        case PacketType::Entities:
        {
            PacketDataEntities packetData;
            packetData.deserialise(packet.data);
            packetData.applyPingEstimate(getPlayerPingLocation(message.m_identityPeer.GetSteamID64()));
            if (game->getLocationState().getPlanetType() != packetData.planetType)
            {
                Log::push("ERROR: Received entity data for incorrect planet type {}\n", packetData.planetType);
                break;
            }
            game->getChunkManager().loadEntityPacketDatas(packetData);
            break;
        }
        case PacketType::Projectiles:
        {
            PacketDataProjectiles packetData;
            packetData.deserialise(packet.data);
            packetData.applyPingEstimate(getPlayerPingLocation(message.m_identityPeer.GetSteamID64()));
            if (game->getLocationState().getPlanetType() != packetData.planetType)
            {
                Log::push("ERROR: Received projectile data for incorrect planet type {}\n", packetData.planetType);
                break;
            }
            game->getProjectileManager(packetData.planetType).getProjectiles() = packetData.projectileManager.getProjectiles();
            break;
        }
        case PacketType::Bosses:
        {
            PacketDataBosses packetData;
            packetData.deserialise(packet.data);
            packetData.applyPingEstimate(getPlayerPingLocation(message.m_identityPeer.GetSteamID64()));
            if (game->getLocationState().getPlanetType() != packetData.planetType)
            {
                Log::push("ERROR: Received boss data for incorrect planet type {}\n", packetData.planetType);
                break;
            }
            game->getBossManager(packetData.planetType) = packetData.bossManager;
            break;
        }
        case PacketType::BossSpawnCheckReply:
        {
            PacketDataBossSpawnCheckReply packetData;
            packetData.deserialise(packet.data);
            if (!game->attemptUseBossSpawnFromHost(packetData.planetType, packetData.bossSpawnItem))
            {
                break;
            }

            PacketDataBossSpawnRequest packetDataReply;
            packetDataReply.planetType = packetData.planetType;
            packetDataReply.bossSpawnItem = packetData.bossSpawnItem;
            Packet packetReply(packetDataReply);
            sendPacketToHost(packetReply, k_nSteamNetworkingSend_Reliable, 0);
            break;
        }
        case PacketType::RocketEnterReply:
        {
            PacketDataRocketEnterReply packetData;
            packetData.deserialise(packet.data);

            if (!game->isLocationStateInitialised(packetData.locationState))
            {
                Log::push("ERROR: Received rocket enter reply for uninitialised location\n");
                break;
            }

            game->enterRocketFromReference(packetData.rocketObjectReference, true);
            break;
        }
        case PacketType::Particle:
        {
            PacketDataParticle packetData;
            packetData.deserialise(packet.data);

            if (game->getLocationState() != LocationState::createFromPlanetType(packetData.planetType))
            {
                break;
            }

            game->getParticleSystem().addParticle(packetData.particle, LocationState::createFromPlanetType(packetData.planetType), nullptr);
            break;
        }
        case PacketType::MapChunkDiscovered:
        {
            PacketDataMapChunkDiscovered packetData;
            packetData.deserialise(packet.data);

            if (game->getLocationState() != LocationState::createFromPlanetType(packetData.planetType))
            {
                break;
            }

            if (game->getChunkManager(packetData.planetType).getWorldMap().isChunkDiscovered(packetData.worldMapSection.chunkPosition))
            {
                break;
            }

            game->getChunkManager(packetData.planetType).getWorldMap().setChunkMapSection(packetData.worldMapSection);
            break;
        }
        case PacketType::PlanetTravelReply:
        {
            PacketDataPlanetTravelReply packetData;
            packetData.deserialise(packet.data);
            game->travelToPlanetFromHost(packetData);
            break;
        }
        case PacketType::RoomTravelReply:
        {
            PacketDataRoomTravelReply packetData;
            packetData.deserialise(packet.data);
            game->travelToRoomDestinationFromHost(packetData);
            break;
        }
        case PacketType::StructureEnterReply:
        {
            PacketDataStructureEnterReply packetData;
            packetData.deserialise(packet.data);
            game->enterStructureFromHost(packetData.planetType, packetData.chunkPos, packetData.structureID, packetData.structureEntrancePos, packetData.roomType);
            break;
        }
        default:
            break;
    }
}

void NetworkHandler::sendGameUpdates(float dt, const Camera& camera)
{
    updateTick += dt;
    if (updateTick < SERVER_UPDATE_TICK)
    {
        return;
    }

    // Update tick
    updateTick = 0.0f;
    updateTickCount = (updateTickCount + 1) % MAX_UPDATE_TICK_COUNT;

    if (isLobbyHost)
    {
        sendGameUpdatesToClients(dt);
    }
    else
    {
        sendGameUpdatesToHost(camera, dt);
    }
}

void NetworkHandler::sendGameUpdatesToClients(float dt)
{
    if (!isLobbyHost)
    {
        return;
    }

    uint64_t steamID = SteamUser()->GetSteamID().ConvertToUint64();

    std::unordered_map<uint64_t, Packet> playerInfoPackets;
    
    // Set own player info
    playerInfoPackets[steamID] = Packet();
    PacketDataPlayerCharacterInfo localCharacterPacketData = game->getPlayer().getNetworkPlayerInfo(nullptr, steamID, dt);
    playerInfoPackets[steamID].set(localCharacterPacketData, true);

    // Get player infos
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        playerInfoPackets[iter->first] = Packet();
        PacketDataPlayerCharacterInfo playerCharacterPacketData = iter->second.getNetworkPlayerInfo(nullptr, iter->first, dt);
        playerInfoPackets[iter->first].set(playerCharacterPacketData, true);
    }

    // Send player info
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        for (auto subIter = networkPlayers.begin(); subIter != networkPlayers.end(); subIter++)
        {
            // Don't send player their own info
            if (iter == subIter)
            {
                continue;
            }

            sendPacketToClient(iter->first, playerInfoPackets[subIter->first], k_nSteamNetworkingSend_Unreliable, 0);
        }
        
        // Send host player data
        sendPacketToClient(iter->first, playerInfoPackets[steamID], k_nSteamNetworkingSend_Unreliable, 0);
    }

    // Only send non-player info on certain ticks to save bandwidth
    if (updateTickCount != NON_PLAYER_UPDATE_TICK)
    {
        return;
    }

    // Send server info
    PacketDataServerInfo serverInfoData;
    serverInfoData.gameTime = game->getGameTime();
    serverInfoData.day = game->getDayCycleManager().getCurrentDay();
    serverInfoData.time = game->getDayCycleManager().getCurrentTime();
    Packet serverInfoPacket;
    serverInfoPacket.set(serverInfoData);

    for (auto& client : networkPlayers)
    {
        sendPacketToClient(client.first, serverInfoPacket, k_nSteamNetworkingSend_Unreliable, 0);
    }

    // Send entity datas to each client as required
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        if (!iter->second.getPlayerData().locationState.isOnPlanet())
        {
            continue;
        }
        
        PlanetType playerPlanetType = iter->second.getPlayerData().locationState.getPlanetType();
        
        PacketDataEntities packetData = game->getChunkManager(playerPlanetType).getEntityPacketDatas(iter->second.getChunkViewRange());
        
        Packet packet;
        packet.set(packetData, true);
        
        // Log::push(("NETWORK: Sending entity data (of {} entities) to " + getPlayerName(iter->first) + " " + packet.getSizeStr() + "\n").c_str(), packetData.entities.size());
        
        sendPacketToClient(iter->first, packet, k_nSteamNetworkingSend_Reliable, 0);
    }
    
    // Send projectile data to each client
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        if (!iter->second.getPlayerData().locationState.isOnPlanet())
        {
            continue;
        }

        PlanetType playerPlanetType = iter->second.getPlayerData().locationState.getPlanetType();

        PacketDataProjectiles packetData;
        packetData.planetType = playerPlanetType;
        packetData.projectileManager = game->getProjectileManager(playerPlanetType);

        Packet packet;
        packet.set(packetData, true);

        // Log::push("NETWORK: Sending projectile data of size {}\n", packet.getSizeStr().c_str());

        sendPacketToClient(iter->first, packet, k_nSteamNetworkingSend_Reliable, 0);
    }

    // Send boss data to each client
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        if (!iter->second.getPlayerData().locationState.isOnPlanet())
        {
            continue;
        }

        PlanetType playerPlanetType = iter->second.getPlayerData().locationState.getPlanetType();

        PacketDataBosses packetData;
        packetData.planetType = playerPlanetType;
        packetData.bossManager = game->getBossManager(playerPlanetType);

        Packet packet;
        packet.set(packetData, true);

        // Log::push("NETWORK: Sending boss data of size {}\n", packet.getSizeStr().c_str());

        sendPacketToClient(iter->first, packet, k_nSteamNetworkingSend_Reliable, 0);
    }

    // Check for travel requests
    for (auto iter = planetTravelRequests.begin(); iter != planetTravelRequests.end();)
    {
        if (game->setupPlanetTravel(iter->planetType, networkPlayers[iter->userId].getPlayerData().locationState,
            iter->rocketUsedReference, iter->userId))
        {
            iter = planetTravelRequests.erase(iter);
            continue;
        }
        iter++;
    }

    for (auto iter = roomTravelRequests.begin(); iter != roomTravelRequests.end();)
    {
        if (game->travelToRoomDestinationForClient(iter->roomType, networkPlayers[iter->userId].getPlayerData().locationState,
            iter->rocketUsedReference, iter->userId))
        {
            iter = roomTravelRequests.erase(iter);
            continue;
        }
        iter++;
    }
}

void NetworkHandler::sendGameUpdatesToHost(const Camera& camera, float dt)
{
    uint64_t steamID = SteamUser()->GetSteamID().ConvertToUint64();

    PacketDataPlayerCharacterInfo playerInfoPacketData = game->getPlayer().getNetworkPlayerInfo(&camera, steamID, dt);

    Packet packet;
    packet.set(playerInfoPacketData);

    sendPacketToHost(packet, k_nSteamNetworkingSend_Unreliable, 0);
}

EResult NetworkHandler::sendPacketToClientsAtLocation(const Packet& packet, int nSendFlags, int nRemoteChannel, const LocationState& locationState)
{
    if (!isLobbyHost)
    {
        return EResult::k_EResultAccessDenied;
    }

    EResult result = EResult::k_EResultOK;

    for (const auto& networkPlayerPair : getNetworkPlayersAtLocation(locationState))
    {
        EResult sendResult = sendPacketToClient(networkPlayerPair.first, packet, nSendFlags, nRemoteChannel);

        if (sendResult != EResult::k_EResultOK)
        {
            result = sendResult;
        }
    }

    return result;
}

EResult NetworkHandler::sendPacketToClients(const Packet& packet, int nSendFlags, int nRemoteChannel, std::unordered_set<uint64_t> exceptions)
{
    if (!isLobbyHost)
    {
        return EResult::k_EResultAccessDenied;
    }

    EResult result = EResult::k_EResultOK;
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        if (exceptions.contains(iter->first))
        {
            continue;
        }

        EResult sendResult = sendPacketToClient(iter->first, packet, nSendFlags, nRemoteChannel);

        if (sendResult != EResult::k_EResultOK)
        {
            result = sendResult;
        }
    }

    return result;
}

EResult NetworkHandler::sendPacketToClient(uint64_t steamID, const Packet& packet, int nSendFlags, int nRemoteChannel)
{
    if (!multiplayerGame)
    {
        return EResult::k_EResultAccessDenied;
    }
    
    if (!isLobbyHost)
    {
        return EResult::k_EResultAccessDenied;
    }

    totalBytesSent += packet.getSize();

    SteamNetworkingIdentity identity;
    identity.SetSteamID64(steamID);

    return packet.sendToUser(identity, nSendFlags, nRemoteChannel);
}

EResult NetworkHandler::sendPacketToHost(const Packet& packet, int nSendFlags, int nRemoteChannel)
{
    if (!multiplayerGame)
    {
        return EResult::k_EResultAccessDenied;
    }
    
    if (isLobbyHost)
    {
        return EResult::k_EResultAccessDenied;
    }

    totalBytesSent += packet.getSize();

    SteamNetworkingIdentity hostIdentity;
    hostIdentity.SetSteamID64(lobbyHost);

    return packet.sendToUser(hostIdentity, nSendFlags, nRemoteChannel);
}

EResult NetworkHandler::sendPacketToServer(const Packet& packet, int nSendFlags, int nRemoteChannel)
{
    if (!multiplayerGame)
    {
        return EResult::k_EResultAccessDenied;
    }
    
    EResult result = EResult::k_EResultOK;

    if (isLobbyHost)
    {
        result = sendPacketToClients(packet, nSendFlags, nRemoteChannel);
    }
    else
    {
        result = sendPacketToHost(packet, nSendFlags, nRemoteChannel);
    }

    return result;
}

void NetworkHandler::requestChunksFromHost(PlanetType planetType, std::vector<ChunkPosition>& chunks, bool forceRequest)
{
    if (!multiplayerGame || isLobbyHost)
    {
        return;
    }

    for (auto iter = chunks.begin(); iter != chunks.end();)
    {
        if (!chunkRequestsOutstanding.contains(*iter))
        {
            chunkRequestsOutstanding[*iter] = game->getGameTime();
        }
        else if (game->getGameTime() - chunkRequestsOutstanding.at(*iter) >= CHUNK_REQUEST_OUTSTANDING_MAX_TIME)
        {
            // Reset time and request again
            chunkRequestsOutstanding[*iter] = game->getGameTime();
        }
        else if (!forceRequest)
        {
            // Chunk is still being requested - do not request again (yet)
            iter = chunks.erase(iter);
            continue;
        }

        iter++;
    }

    // Do not request 0 chunks
    if (chunks.size() <= 0)
    {
        return;
    }
    
    Log::push(("NETWORK: Requesting " + std::to_string(chunks.size()) + " chunks from host for planet type " + std::to_string(planetType) + "\n").c_str());

    PacketDataChunkRequests packetData;
    packetData.planetType = planetType;
    packetData.chunkRequests = chunks;
    Packet packet;
    packet.set(packetData);
    sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
}

void NetworkHandler::queueSendPlayerData()
{
    if (!multiplayerGame)
    {
        return;
    }
    
    sendPlayerDataQueued = true;
    sendPlayerDataQueueTime = SEND_PLAYER_DATA_QUEUE_TIME;
}

void NetworkHandler::sendPlayerDataToHost()
{
    if (!isClient())
    {
        return;
    }

    PacketDataPlayerData packetData;
    packetData.userID = SteamUser()->GetSteamID().ConvertToUint64();
    packetData.playerData = game->createPlayerData();

    Packet packet;
    packet.set(packetData, true);

    Log::push(("NETWORK: Sending player data to host " + packet.getSizeStr() + "\n").c_str());

    sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
}

void NetworkHandler::sendHostPlayerDataToClients()
{
    if (!getIsLobbyHost())
    {
        return;
    }

    PacketDataPlayerData packetData;
    packetData.userID = SteamUser()->GetSteamID().ConvertToUint64();
    packetData.playerData = game->createPlayerData();

    sendPlayerDataToClients(packetData);
}

void NetworkHandler::handleChunkDatasFromHost(const PacketDataChunkDatas& chunkDatas)
{
    game->handleChunkDataFromHost(chunkDatas);

    for (const auto& chunkData : chunkDatas.chunkDatas)
    {
        if (chunkRequestsOutstanding.contains(chunkData.chunkPosition))
        {
            chunkRequestsOutstanding.erase(chunkData.chunkPosition);
        }
    }
}

void NetworkHandler::handleChunkModifiedAlertsFromHost(const PacketDataChunkModifiedAlerts& chunkModifiedAlerts)
{
    std::vector<ChunkPosition> chunksToRequest;

    // Only request chunks from host if already generated (as modification to chunk does not concern us if we do not already have it in memory)
    for (ChunkPosition chunkPosition : chunkModifiedAlerts.chunkRequests)
    {
        if (game->getChunkManager().isChunkGenerated(chunkPosition))
        {
            chunksToRequest.push_back(chunkPosition);
        }
    }

    // Force request (ignore cooldown as host has alerted modification of chunks)
    requestChunksFromHost(chunkModifiedAlerts.planetType, chunksToRequest, true);
}

void NetworkHandler::sendPlayerData()
{
    if (getIsLobbyHost())
    {
        sendHostPlayerDataToClients();
    }
    else if (isClient())
    {
        sendPlayerDataToHost();
    }
    
    sendPlayerDataQueued = false;
}

void NetworkHandler::sendPlayerDataToClients(const PacketDataPlayerData& playerDataPacket)
{
    Packet packet;
    packet.set(playerDataPacket, true);

    Log::push(("NETWORK: Sending player data to clients " + packet.getSizeStr() + "\n").c_str());

    // Relay to clients (except this playerdata's respective client)
    for (auto client = networkPlayers.begin(); client != networkPlayers.end(); client++)
    {
        if (client->first == playerDataPacket.userID)
        {
            continue;
        }

        sendPacketToClient(client->first, packet, k_nSteamNetworkingSend_Reliable, 0);
    }
}

bool NetworkHandler::canSendStructureRequest()
{
    return (structureEnterRequestCooldown <= 0.0f);
}

void NetworkHandler::structureRequestSent()
{
    structureEnterRequestCooldown = STRUCTURE_ENTER_REQUEST_COOLDOWN;
}

int NetworkHandler::getTotalBytesSent() const
{
    return totalBytesSent;
}

int NetworkHandler::getTotalBytesReceived() const
{
    return totalBytesReceived;
}

float NetworkHandler::getByteSendRate(float dt) const
{
    return byteSendRate;
}

float NetworkHandler::getByteReceiveRate(float dt) const
{
    return byteReceiveRate;
}