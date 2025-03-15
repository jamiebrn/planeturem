#include "Network/NetworkHandler.hpp"
#include "Game.hpp"

NetworkHandler::NetworkHandler(Game* game)
{
    if (game == nullptr)
    {
        printf("ERROR: NetworkHandler game ptr set to null\n");
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
    SteamAPICall_t steamAPICall = SteamMatchmaking()->CreateLobby(ELobbyType::k_ELobbyTypeFriendsOnly, 8);
    m_SteamCallResultCreateLobby.Set(steamAPICall, this, &NetworkHandler::callbackLobbyCreated);
}

void NetworkHandler::callbackLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure)
{
    if (pCallback->m_ulSteamIDLobby == 0)
    {
        std::cout << "ERROR: Lobby creation failed\n";
        return;
    }

    std::cout << "NETWORK: Created lobby " << pCallback->m_ulSteamIDLobby << "\n";
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

    if (isLobbyHost)
    {
        SteamMatchmaking()->SetLobbyJoinable(steamLobbyId, false);

        // Alert clients of host leaving
        Packet packet;
        packet.type = PacketType::HostQuit;

        for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
        {
            SteamNetworkingIdentity identity;
            identity.SetSteamID64(iter->first);
            packet.sendToUser(identity, k_nSteamNetworkingSend_Reliable, 0);
            SteamNetworkingMessages()->CloseSessionWithUser(identity);
        }
    }
    else
    {
        SteamNetworkingIdentity hostIdentity;
        hostIdentity.SetSteamID64(lobbyHost);
        SteamNetworkingMessages()->CloseSessionWithUser(hostIdentity);
    }
    
    SteamMatchmaking()->LeaveLobby(steamLobbyId);
    isLobbyHost = false;
    multiplayerGame = false;
}

void NetworkHandler::sendWorldJoinReply(std::string playerName)
{
    std::cout << "NETWORK: Sending join reply to user " << SteamFriends()->GetFriendPersonaName(CSteamID(lobbyHost)) << "\n";

    PacketDataJoinReply packetData;
    packetData.playerName = playerName;
    
    Packet packet;
    packet.set(packetData);
    sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
}

bool NetworkHandler::isMultiplayerGame()
{
    return multiplayerGame;
}

bool NetworkHandler::isLobbyHostOrSolo()
{
    if (!multiplayerGame)
    {
        return true;
    }

    return isLobbyHost;
}

bool NetworkHandler::getIsLobbyHost()
{
    if (!multiplayerGame)
    {
        return false;
    }

    return isLobbyHost;
}

bool NetworkHandler::isClient()
{
    return (multiplayerGame && !isLobbyHost);
}

int NetworkHandler::getNetworkPlayerCount()
{
    return networkPlayers.size();
}

std::optional<uint64_t> NetworkHandler::getLobbyID()
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

std::vector<WorldObject*> NetworkHandler::getNetworkPlayersToDraw(const LocationState& locationState, sf::Vector2f playerPosition)
{
    std::vector<WorldObject*> networkPlayerObjects;

    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        // Not in same location as player
        if (iter->second.getPlayerData().locationState != locationState)
        {
            continue;
        }

        // Translate position to wrap around world correctly, if required
        if (locationState.isOnPlanet())
        {
            iter->second.applyWorldWrapTranslation(playerPosition, game->getChunkManager(locationState.getPlanetType()));
        }

        networkPlayerObjects.push_back(&iter->second);
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

    if (thisPlayerPlanetType.has_value())
    {
        if (thisPlayerPlanetType.value() >= 0)
        {
            planetTypeSet.insert(thisPlayerPlanetType.value());
        }
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

void NetworkHandler::registerNetworkPlayer(uint64_t id, bool notify)
{
    if (!multiplayerGame)
    {
        return;
    }

    // Alert connected players
    if (isLobbyHost)
    {
        Packet packet;
        packet.type = PacketType::PlayerJoined;
        packet.data.resize(sizeof(id));
        memcpy(packet.data.data(), &id, sizeof(id));
    
        for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
        {
            SteamNetworkingIdentity identity;
            identity.SetSteamID64(iter->first);
            packet.sendToUser(identity, k_nSteamNetworkingSend_Reliable, 0);
        }
    }

    if (notify)
    {
        InventoryGUI::pushItemPopup(ItemCount(0, 1), false, std::string(SteamFriends()->GetFriendPersonaName(id)) + " joined");
    }

    networkPlayers[id] = NetworkPlayer(sf::Vector2f(0, 0));
}

void NetworkHandler::deleteNetworkPlayer(uint64_t id)
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
            SteamNetworkingIdentity identity;
            identity.SetSteamID64(iter->first);
            packet.sendToUser(identity, k_nSteamNetworkingSend_Reliable, 0);
        }
    }

    networkPlayers.erase(id);

    InventoryGUI::pushItemPopup(ItemCount(0, 1), false, std::string(SteamFriends()->GetFriendPersonaName(id)) + " disconnected");
}

void NetworkHandler::callbackLobbyJoinRequested(GameLobbyJoinRequested_t* pCallback)
{
    SteamMatchmaking()->JoinLobby(pCallback->m_steamIDLobby);
    multiplayerGame = true;
}

void NetworkHandler::callbackLobbyEnter(LobbyEnter_t* pCallback)
{
    steamLobbyId = pCallback->m_ulSteamIDLobby;
    std::cout << "NETWORK: Joined lobby " << steamLobbyId << "\n";
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

            Packet packet;
            packet.set(packetData);

            EResult result = packet.sendToUser(userIdentity, k_nSteamNetworkingSend_Reliable, 0);
            if (result == EResult::k_EResultOK)
            {
                std::cout << "NETWORK: Sent join query successfully\n";
            }
            else if (result == EResult::k_EResultNoConnection)
            {
                std::cout << "ERROR: Could not send join query\n";
            }
        }
        else
        {
            deleteNetworkPlayer(pCallback->m_ulSteamIDUserChanged);
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
            if (pCallback->m_ulSteamIDUserChanged == SteamMatchmaking()->GetLobbyOwner(steamLobbyId).ConvertToUint64())
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
}

void NetworkHandler::receiveMessages()
{
    static const int MAX_MESSAGES = 10;

    SteamNetworkingMessage_t* messages[MAX_MESSAGES];
    int messageCount = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, messages, MAX_MESSAGES);

    for (int i = 0; i < messageCount; i++)
    {
        Packet packet;
        packet.deserialise((char*)messages[i]->GetData(), messages[i]->GetSize());

        processMessage(*messages[i], packet);

        messages[i]->Release();
    }
}

void NetworkHandler::processMessage(const SteamNetworkingMessage_t& message, const Packet& packet)
{
    // Process packet
    if (isLobbyHost)
    {
        processMessageAsHost(message, packet);
    }
    else
    {
        processMessageAsClient(message, packet);
    }

    
    switch (packet.type)
    {
        case PacketType::PlayerCharacterInfo:
        {
            if (isLobbyHost)
            {
                if (!networkPlayers.contains(message.m_identityPeer.GetSteamID64()))
                {
                    registerNetworkPlayer(message.m_identityPeer.GetSteamID64());
                }
            }
    
            PacketDataPlayerCharacterInfo packetData;
            packetData.deserialise(packet.data);
    
            if (networkPlayers.contains(packetData.userID))
            {
                // std::string playerName = SteamFriends()->GetFriendPersonaName(CSteamID(packetData.userID));
    
                // Translate player position to wrap around world, relative to player
                // sf::Vector2f playerPos = game->getChunkManager().translatePositionAroundWorld(sf::Vector2f(packetData.positionX, packetData.positionY),
                //     game->getPlayer().getPosition());
                // packetData.positionX = playerPos.x;
                // packetData.positionY = playerPos.y;
    
                networkPlayers[packetData.userID].setNetworkPlayerCharacterInfo(packetData);
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
                printf(("NETWORK: Received player data for network player " + std::to_string(packetData.userID) + "(" +
                packetData.playerData.name + ")\n").c_str());
            }
            else
            {
                printf(("WARNING: Received player data for unregistered network player " + std::to_string(packetData.userID) + "(" +
                    packetData.playerData.name + ")\n").c_str());
            }

            // If host, save data and redistribute
            if (getIsLobbyHost())
            {
                networkPlayerDatasSaved[packetData.userID] = packetData.playerData;
                sendPlayerDataToClients(packetData);
            }
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
            game->buildObject(packetData.objectReference.chunk, packetData.objectReference.tile, packetData.objectType, packetData.planetType, sentFromHost);
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
                    printf(("ERROR: Attempted to create item pickups from client on null planet type " +
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
                    std::cout << "ERROR: Failed to create item pickup sent from host in null chunk (" << itemPickupPair.first.chunk.x <<
                    ", " << itemPickupPair.first.chunk.y << ")\n";
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
        case PacketType::ItemPickupDeleted:
        {
            PacketDataItemPickupDeleted packetData;
            packetData.deserialise(packet.data);
            
            // If host, redistribute to clients
            if (isLobbyHost)
            {
                sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
                
                if (!game->isLocationStateInitialised(packetData.locationState))
                {
                    printf(("ERROR: Attempted to delete item pickups from client on null planet type " +
                        std::to_string(packetData.locationState.getPlanetType()) + "\n").c_str());
                    break;
                }
    
                Chunk* chunkPtr = game->getChunkManager(packetData.locationState.getPlanetType()).getChunk(packetData.pickupDeleted.chunk);
                if (chunkPtr)
                {
                    ItemPickup* itemPickupPtr = chunkPtr->getItemPickup(packetData.pickupDeleted.id);
                    if (itemPickupPtr)
                    {
                        // Give item to client
                        PacketDataInventoryAddItem itemPacketData;
                        itemPacketData.itemType = itemPickupPtr->getItemType();
                        itemPacketData.amount = 1;
                        Packet itemPacket;
                        itemPacket.set(itemPacketData);
    
                        itemPacket.sendToUser(message.m_identityPeer, k_nSteamNetworkingSend_Reliable, 0);
                    }
                }
            }

            if (game->isLocationStateInitialised(packetData.locationState))
            {
                // Delete pickup from chunk manager, regardless of whether we are host or client
                game->getChunkManager(packetData.locationState.getPlanetType()).deleteItemPickup(packetData.pickupDeleted);
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
    }
}

void NetworkHandler::processMessageAsHost(const SteamNetworkingMessage_t& message, const Packet& packet)
{
    switch (packet.type)
    {
        case PacketType::JoinReply:
        {
            const char* steamName = SteamFriends()->GetFriendPersonaName(message.m_identityPeer.GetSteamID());
            std::cout << "NETWORK: Player joined: " << steamName << " (" << message.m_identityPeer.GetSteamID64() << ")\n";
            
            // Send world info
            PacketDataJoinInfo packetData;
            packetData.seed = game->getPlanetSeed();
            packetData.gameTime = game->getGameTime();
            packetData.time = game->getDayCycleManager().getCurrentTime();
            packetData.day = game->getDayCycleManager().getCurrentDay();
            // packetData.planetName = PlanetGenDataLoader::getPlanetGenData(game->getChunkManager().getPlanetType()).name;
    
            // packetData.chestDataPool = game->getChestDataPool();

            // Initialise new player data
            if (!networkPlayerDatasSaved.contains(message.m_identityPeer.GetSteamID64()))
            {
                // Player data does not exist - initialise
                
                // Get name from join reply packet
                PacketDataJoinReply packetData;
                packetData.deserialise(packet.data);
                networkPlayerDatasSaved[message.m_identityPeer.GetSteamID64()] = PlayerData();

                PlayerData& playerData = networkPlayerDatasSaved[message.m_identityPeer.GetSteamID64()];

                playerData.name = packetData.playerName;

                playerData.locationState.setPlanetType(PlanetGenDataLoader::getPlanetTypeFromName("Earthlike"));

                playerData.inventory = InventoryData(32);
                playerData.inventory.giveStartingItems();
                playerData.armourInventory = InventoryData(3);
                
                // Find spawn for player
                ChunkPosition playerSpawnChunk = game->getChunkManager(playerData.locationState.getPlanetType()).findValidSpawnChunk(2);
                playerData.position.x = playerSpawnChunk.x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
                playerData.position.y = playerSpawnChunk.y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
            }

            // Send player data
            packetData.playerData = networkPlayerDatasSaved.at(message.m_identityPeer.GetSteamID64());
            
            packetData.currentPlayerDatas[SteamUser()->GetSteamID().ConvertToUint64()] = game->createPlayerData();
            for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
            {
                packetData.currentPlayerDatas[iter->first] = iter->second.getPlayerData();
            }
    
            registerNetworkPlayer(message.m_identityPeer.GetSteamID64());
            
            Packet packetToSend;
            packetToSend.set(packetData, true);
            packetToSend.sendToUser(message.m_identityPeer, k_nSteamNetworkingSend_Reliable, 0);
            break;
        }
        case PacketType::ItemPickupsCreateRequest:
        {
            PacketDataItemPickupsCreateRequest packetData;
            packetData.deserialise(packet.data);

            PacketDataItemPickupsCreated pickupsCreatedPacketData;

            // Create item pickups requested
            for (auto& request : packetData.pickupRequests)
            {
                Chunk* chunkPtr = game->getChunkManager(pickupsCreatedPacketData.locationState.getPlanetType()).getChunk(request.chunk);
                if (!chunkPtr)
                {
                    std::cout << "ERROR: Failed to create item pickup requested from client in null chunk (" << request.chunk.x <<
                        ", " << request.chunk.y << ")\n";
                    continue;
                }

                // Denormalise pickup position from chunk-relative to world position
                ItemPickup pickup(request.positionRelative + chunkPtr->getWorldPosition(), request.itemType, game->getGameTime());

                uint64_t itemPickupID = chunkPtr->addItemPickup(pickup);
                ItemPickup* itemPickupPtr = chunkPtr->getItemPickup(itemPickupID);
                if (!itemPickupPtr)
                {
                    std::cout << "ERROR: Failed to create null item pickup requested from client in chunk (" << request.chunk.x <<
                    ", " << request.chunk.y << ")\n";
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
            game->getChestDataPool().overwriteChestData(packetData.chestID, packetData.chestData);
            printf(("NETWORK: Received chest data from " + std::string(SteamFriends()->GetFriendPersonaName(message.m_identityPeer.GetSteamID())) + "\n").c_str());
            break;
        }
        case PacketType::PlanetTravelRequest:
        {
            PacketDataPlanetTravelRequest packetData;
            packetData.deserialise(packet.data);
            game->setupPlanetTravel(packetData.planetType, message.m_identityPeer.GetSteamID64());
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
                printf(("NETWORK: Sending structure enter reply to " +
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
    }
}

void NetworkHandler::processMessageAsClient(const SteamNetworkingMessage_t& message, const Packet& packet)
{
    switch (packet.type)
    {
        case PacketType::JoinQuery:
        {
            PacketDataJoinQuery packetData;
            packetData.deserialise(packet.data);
            
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
                registerNetworkPlayer(networkPlayerDataPair.first, false);
                std::cout << "NETWORK: Registered existing player " << SteamFriends()->GetFriendPersonaName(CSteamID(networkPlayerDataPair.first)) << "\n";
                networkPlayers[networkPlayerDataPair.first].setPlayerData(networkPlayerDataPair.second);
            }

            // Load into world
            game->joinWorld(packetData);
            break;
        }
        case PacketType::PlayerJoined:
        {
            uint64_t id;
            memcpy(&id, packet.data.data(), sizeof(id));
            registerNetworkPlayer(id);
            break;
        }
        case PacketType::PlayerDisconnected:
        {
            uint64_t id;
            memcpy(&id, packet.data.data(), sizeof(id));
            deleteNetworkPlayer(id);
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
            serverInfo.applyPingEstimate();

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
            handleChunkDatasFromHost(packetData);
            break;
        }
        case PacketType::PlanetTravelReply:
        {
            PacketDataPlanetTravelReply packetData;
            packetData.deserialise(packet.data);
            game->travelToPlanetFromHost(packetData);
            break;
        }
        case PacketType::StructureEnterReply:
        {
            PacketDataStructureEnterReply packetData;
            packetData.deserialise(packet.data);
            game->enterStructureFromHost(packetData.planetType, packetData.chunkPos, packetData.structureID, packetData.structureEntrancePos, packetData.roomType);
            break;
        }
    }
}

void NetworkHandler::sendGameUpdates(const Camera& camera)
{
    if (isLobbyHost)
    {
        sendGameUpdatesToClients();
    }
    else
    {
        sendGameUpdatesToHost(camera);
    }
}

void NetworkHandler::sendGameUpdatesToClients()
{
    if (!isLobbyHost)
    {
        return;
    }

    uint64_t steamID = SteamUser()->GetSteamID().ConvertToUint64();

    // Send server info
    PacketDataServerInfo serverInfoData;
    serverInfoData.gameTime = game->getGameTime();
    serverInfoData.day = game->getDayCycleManager().getCurrentDay();
    serverInfoData.time = game->getDayCycleManager().getCurrentTime();
    serverInfoData.setHostPingLocation();
    Packet serverInfoPacket;
    serverInfoPacket.set(serverInfoData);

    for (auto& client : networkPlayers)
    {
        SteamNetworkingIdentity clientIdentity;
        clientIdentity.SetSteamID64(client.first);
        serverInfoPacket.sendToUser(clientIdentity, k_nSteamNetworkingSend_Unreliable, 0);
    }

    std::unordered_map<uint64_t, Packet> playerInfoPackets;
    playerInfoPackets[steamID] = Packet();
    playerInfoPackets[steamID].set(game->getPlayer().getNetworkPlayerInfo(nullptr, steamID));

    // Get player infos
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        playerInfoPackets[iter->first] = Packet();
        playerInfoPackets[iter->first].set(iter->second.getNetworkPlayerInfo(nullptr, iter->first));
    }

    // Send player info
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        SteamNetworkingIdentity identity;
        identity.SetSteamID64(iter->first);

        for (auto subIter = networkPlayers.begin(); subIter != networkPlayers.end(); subIter++)
        {
            // Don't send player their own info
            if (iter == subIter)
            {
                continue;
            }

            playerInfoPackets[subIter->first].sendToUser(identity, k_nSteamNetworkingSend_Unreliable, 0);
        }
        
        // Send host player data
        playerInfoPackets[steamID].sendToUser(identity, k_nSteamNetworkingSend_Unreliable, 0);
    }
}

void NetworkHandler::sendGameUpdatesToHost(const Camera& camera)
{
    uint64_t steamID = SteamUser()->GetSteamID().ConvertToUint64();

    Packet packet;
    packet.set(game->getPlayer().getNetworkPlayerInfo(&camera, steamID));

    SteamNetworkingIdentity hostIdentity;
    hostIdentity.SetSteamID64(lobbyHost);

    packet.sendToUser(hostIdentity, k_nSteamNetworkingSend_Unreliable, 0);
}

EResult NetworkHandler::sendPacketToClients(const Packet& packet, int nSendFlags, int nRemoteChannel)
{
    if (!isLobbyHost)
    {
        return EResult::k_EResultAccessDenied;
    }

    EResult result = EResult::k_EResultOK;
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        SteamNetworkingIdentity identity;
        identity.SetSteamID64(iter->first);
        EResult sendResult = packet.sendToUser(identity, nSendFlags, nRemoteChannel);

        if (sendResult != EResult::k_EResultOK)
        {
            result = sendResult;
        }
    }

    return result;
}

EResult NetworkHandler::sendPacketToClient(uint64_t steamID, const Packet& packet, int nSendFlags, int nRemoteChannel)
{
    if (!isLobbyHost)
    {
        return EResult::k_EResultAccessDenied;
    }

    SteamNetworkingIdentity identity;
    identity.SetSteamID64(steamID);
    return packet.sendToUser(identity, nSendFlags, nRemoteChannel);
}

EResult NetworkHandler::sendPacketToHost(const Packet& packet, int nSendFlags, int nRemoteChannel)
{
    if (isLobbyHost)
    {
        return EResult::k_EResultAccessDenied;
    }

    SteamNetworkingIdentity hostIdentity;
    hostIdentity.SetSteamID64(lobbyHost);

    return packet.sendToUser(hostIdentity, nSendFlags, nRemoteChannel);
}

void NetworkHandler::requestChunksFromHost(PlanetType planetType, std::vector<ChunkPosition>& chunks)
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
        else
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
    
    printf(("NETWORK: Requesting " + std::to_string(chunks.size()) + " chunks from host for planet type " + std::to_string(planetType) + "\n").c_str());

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

    printf(("NETWORK: Sending player data to host " + packet.getSizeStr() + "\n").c_str());

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

    printf(("NETWORK: Sending player data to clients " + packet.getSizeStr() + "\n").c_str());

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