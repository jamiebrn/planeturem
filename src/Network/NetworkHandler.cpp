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
        std::cout << "Lobby creation failed\n";
        return;
    }

    std::cout << "Created lobby " << pCallback->m_ulSteamIDLobby << "\n";
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

std::unordered_map<uint64_t, Player>& NetworkHandler::getNetworkPlayers()
{
    return networkPlayers;
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

    networkPlayers[id] = Player(sf::Vector2f(0, 0));
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
    
    networkPlayers.erase(id);

    // Alert connected players
    if (isLobbyHost)
    {
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
    std::cout << "Joined lobby " << steamLobbyId << "\n";
    multiplayerGame = true;
}

void NetworkHandler::callbackLobbyUpdated(LobbyChatUpdate_t* pCallback)
{
    SteamNetworkingIdentity userIdentity;
    userIdentity.SetSteamID64(pCallback->m_ulSteamIDUserChanged);
    
    if (isLobbyHost)
    {
        Packet packet;
        
        if (pCallback->m_rgfChatMemberStateChange & k_EChatMemberStateChangeEntered)
        {
            packet.type = PacketType::JoinQuery;
            EResult result = packet.sendToUser(userIdentity, k_nSteamNetworkingSend_Reliable, 0);
            if (result == EResult::k_EResultOK)
            {
                std::cout << "Sent join query successfully\n";
            }
            else if (result == EResult::k_EResultNoConnection)
            {
                std::cout << "Could not send join query\n";
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

void NetworkHandler::receiveMessages()
{
    static const int MAX_MESSAGES = 10;

    SteamNetworkingMessage_t* messages[MAX_MESSAGES];
    int messageCount = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, messages, MAX_MESSAGES);

    for (int i = 0; i < messageCount; i++)
    {
        Packet packet;
        packet.deserialise((char*)messages[i]->GetData(), messages[i]->GetSize());

        // Process packet
        if (packet.type == PacketType::JoinReply && isLobbyHost)
        {
            const char* steamName = SteamFriends()->GetFriendPersonaName(messages[i]->m_identityPeer.GetSteamID());
            std::cout << "Player joined: " << steamName << " (" << messages[i]->m_identityPeer.GetSteamID64() << ")\n";
            
            // Send world info
            PacketDataJoinInfo packetData;
            packetData.seed = game->getChunkManager().getSeed();
            packetData.gameTime = game->getGameTime();
            packetData.time = game->getDayCycleManager().getCurrentTime();
            packetData.day = game->getDayCycleManager().getCurrentDay();
            packetData.planetName = PlanetGenDataLoader::getPlanetGenData(game->getChunkManager().getPlanetType()).name;

            packetData.chestDataPool = game->getChestDataPool();

            // Find spawn for player
            ChunkPosition playerSpawnChunk = game->getChunkManager().findValidSpawnChunk(2);
            packetData.spawnPosition.x = playerSpawnChunk.x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
            packetData.spawnPosition.y = playerSpawnChunk.y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
            
            packetData.currentPlayers.push_back(SteamUser()->GetSteamID().ConvertToUint64());
            for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
            {
                packetData.currentPlayers.push_back(iter->first);
            }

            registerNetworkPlayer(messages[i]->m_identityPeer.GetSteamID64());
            
            Packet packetToSend;
            packetToSend.set(packetData, true);
            packetToSend.sendToUser(messages[i]->m_identityPeer, k_nSteamNetworkingSend_Reliable, 0);
        }
        else if (packet.type == PacketType::JoinQuery)
        {
            Packet packetToSend;
            packetToSend.type = PacketType::JoinReply;
            
            std::cout << "Sending join reply to user " << messages[i]->m_identityPeer.GetSteamID64() << "\n";
            
            packetToSend.sendToUser(messages[i]->m_identityPeer, k_nSteamNetworkingSend_Reliable, 0);
        }
        else if (packet.type == PacketType::JoinInfo)
        {
            // Deserialise packet data
            PacketDataJoinInfo packetData;
            packetData.deserialise(packet.data);

            // Set lobby host
            lobbyHost = messages[i]->m_identityPeer.GetSteamID64();
            isLobbyHost = false;
            
            multiplayerGame = true;

            networkPlayers.clear();
            for (uint64_t player : packetData.currentPlayers)
            {
                registerNetworkPlayer(player, false);
                std::cout << "Registered existing player " << SteamFriends()->GetFriendPersonaName(CSteamID(player)) << "\n";
            }

            // Load into world
            game->joinWorld(packetData);
        }
        else if (packet.type == PacketType::PlayerJoined && !isLobbyHost)
        {
            uint64_t id;
            memcpy(&id, packet.data.data(), sizeof(id));
            registerNetworkPlayer(id);
        }
        else if (packet.type == PacketType::PlayerDisconnected && !isLobbyHost)
        {    
            uint64_t id;
            memcpy(&id, packet.data.data(), sizeof(id));
            deleteNetworkPlayer(id);
        }
        else if (packet.type == PacketType::HostQuit && !isLobbyHost)
        {
            game->quitWorld();
        }
        else if (packet.type == PacketType::WorldInfo && !isLobbyHost)
        {
            PacketDataWorldInfo worldInfo;
            worldInfo.deserialise(packet.data);
            worldInfo.applyPingEstimate();

            game->setGameTime(worldInfo.gameTime);
            game->getDayCycleManager(true).setCurrentDay(worldInfo.day);
            game->getDayCycleManager(true).setCurrentTime(worldInfo.time);
        }
        else if (packet.type == PacketType::PlayerInfo)
        {
            if (isLobbyHost)
            {
                if (!networkPlayers.contains(messages[i]->m_identityPeer.GetSteamID64()))
                {
                    registerNetworkPlayer(messages[i]->m_identityPeer.GetSteamID64());
                }
            }

            PacketDataPlayerInfo packetData;
            packetData.deserialise(packet.data);

            if (networkPlayers.contains(packetData.userID))
            {
                std::string playerName = SteamFriends()->GetFriendPersonaName(CSteamID(packetData.userID));

                // Translate player position to wrap around world, relative to player
                sf::Vector2f playerPos = game->getChunkManager().translatePositionAroundWorld(sf::Vector2f(packetData.positionX, packetData.positionY),
                    game->getPlayer().getPosition());
                packetData.positionX = playerPos.x;
                packetData.positionY = playerPos.y;

                networkPlayers[packetData.userID].setNetworkPlayerInfo(packetData, playerName, game->getPlayer().getPosition(), game->getChunkManager());
            }
        }
        else if (packet.type == PacketType::ObjectHit)
        {
            PacketDataObjectHit packetData;
            packetData.deserialise(packet.data);
            bool sentFromHost = !isLobbyHost;
            game->hitObject(packetData.objectHit.chunk, packetData.objectHit.tile, packetData.damage, sentFromHost, packetData.userId);
        }
        else if (packet.type == PacketType::ObjectBuilt)
        {
            PacketDataObjectBuilt packetData;
            packetData.deserialise(packet.data);
            bool sentFromHost = !isLobbyHost;
            game->buildObject(packetData.objectReference.chunk, packetData.objectReference.tile, packetData.objectType, sentFromHost);
        }
        else if (packet.type == PacketType::ObjectDestroyed && !isLobbyHost)
        {
            PacketDataObjectDestroyed packetData;
            packetData.deserialise(packet.data);
            game->getChunkManager().deleteObject(packetData.objectReference.chunk, packetData.objectReference.tile);
        }
        else if (packet.type == PacketType::ItemPickupsCreated)
        {
            PacketDataItemPickupsCreated packetData;
            packetData.deserialise(packet.data);
            
            // Create item pickups sent
            for (auto& itemPickupPair : packetData.createdPickups)
            {
                Chunk* chunkPtr = game->getChunkManager().getChunk(itemPickupPair.first.chunk);
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
        }
        else if (packet.type == PacketType::ItemPickupDeleted)
        {
            PacketDataItemPickupDeleted packetData;
            packetData.deserialise(packet.data);
            
            // If host, redistribute to clients
            if (isLobbyHost)
            {
                sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);

                Chunk* chunkPtr = game->getChunkManager().getChunk(packetData.pickupDeleted.chunk);
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

                        itemPacket.sendToUser(messages[i]->m_identityPeer, k_nSteamNetworkingSend_Reliable, 0);
                    }
                }
            }

            // Delete pickup from chunk manager, regardless of whether we are host or client
            game->getChunkManager().deleteItemPickup(packetData.pickupDeleted);
        }
        else if (packet.type == PacketType::ItemPickupsCreateRequest && isLobbyHost)
        {
            PacketDataItemPickupsCreateRequest packetData;
            packetData.deserialise(packet.data);

            PacketDataItemPickupsCreated pickupsCreatedPacketData;

            // Create item pickups requested
            for (auto& request : packetData.pickupRequests)
            {
                Chunk* chunkPtr = game->getChunkManager().getChunk(request.chunk);
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
        }
        else if (packet.type == PacketType::InventoryAddItem)
        {
            PacketDataInventoryAddItem packetData;
            packetData.deserialise(packet.data);
            game->getInventory().addItem(packetData.itemType, packetData.amount, true);
        }
        else if (packet.type == PacketType::ChunkRequests && isLobbyHost)
        {
            PacketDataChunkRequests packetData;
            packetData.deserialise(packet.data);
            game->handleChunkRequestsFromClient(packetData, messages[i]->m_identityPeer);
        }
        else if (packet.type == PacketType::ChunkDatas && !isLobbyHost)
        {
            PacketDataChunkDatas packetData;
            packetData.deserialise(packet.data);
            handleChunkDatasFromHost(packetData);
        }
        else if (packet.type == PacketType::ChestOpened)
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
        }
        else if (packet.type == PacketType::ChestClosed)
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
            game->closeChest(packetData.chestObject, true, packetData.userID);
        }
        else if (packet.type == PacketType::ChestDataModified && isLobbyHost)
        {
            PacketDataChestDataModified packetData;
            packetData.deserialise(packet.data);
            game->getChestDataPool().overwriteChestData(packetData.chestID, packetData.chestData);
            printf(("Received chest data from " + std::string(SteamFriends()->GetFriendPersonaName(messages[i]->m_identityPeer.GetSteamID())) + "\n").c_str());
        }

        messages[i]->Release();
    }
}

void NetworkHandler::sendGameUpdates()
{

}

void NetworkHandler::sendGameUpdatesToClients()
{
    if (!isLobbyHost)
    {
        return;
    }

    uint64_t steamID = SteamUser()->GetSteamID().ConvertToUint64();

    // Send world info
    PacketDataWorldInfo worldInfoData;
    worldInfoData.gameTime = game->getGameTime();
    worldInfoData.day = game->getDayCycleManager().getCurrentDay();
    worldInfoData.time = game->getDayCycleManager().getCurrentTime();
    worldInfoData.setHostPingLocation();
    Packet worldInfoPacket;
    worldInfoPacket.set(worldInfoData);

    for (auto& client : networkPlayers)
    {
        SteamNetworkingIdentity clientIdentity;
        clientIdentity.SetSteamID64(client.first);
        worldInfoPacket.sendToUser(clientIdentity, k_nSteamNetworkingSend_Unreliable, 0);
    }

    std::unordered_map<uint64_t, Packet> playerInfoPackets;
    playerInfoPackets[steamID] = Packet();
    playerInfoPackets[steamID].set(game->getPlayer().getNetworkPlayerInfo(steamID));

    // Get player infos
    for (auto iter = networkPlayers.begin(); iter != networkPlayers.end(); iter++)
    {
        playerInfoPackets[iter->first] = Packet();
        playerInfoPackets[iter->first].set(iter->second.getNetworkPlayerInfo(iter->first));
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

void NetworkHandler::sendGameUpdatesToHost()
{
    uint64_t steamID = SteamUser()->GetSteamID().ConvertToUint64();

    Packet packet;
    packet.set(game->getPlayer().getNetworkPlayerInfo(steamID));

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

void NetworkHandler::requestChunksFromHost(std::vector<ChunkPosition>& chunks)
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
    
    printf(("Requesting " + std::to_string(chunks.size()) + " chunks from host\n").c_str());

    PacketDataChunkRequests packetData;
    packetData.chunkRequests = chunks;
    Packet packet;
    packet.set(packetData);
    sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
}

void NetworkHandler::handleChunkDatasFromHost(const PacketDataChunkDatas& chunkDatas)
{
    for (const auto& chunkData : chunkDatas.chunkDatas)
    {
        game->handleChunkDataFromHost(chunkData);

        if (chunkRequestsOutstanding.contains(chunkData.chunkPosition))
        {
            chunkRequestsOutstanding.erase(chunkData.chunkPosition);
        }
    }
}