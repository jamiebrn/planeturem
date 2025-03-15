#pragma once

#include <vector>

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <vector>

#include <SFML/Graphics.hpp>

#include "Core/ResolutionHandler.hpp"
#include "Core/TextDraw.hpp"

#include "Data/typedefs.hpp"
#include "Types/GameState.hpp"

#include "Network/PacketData/PacketDataPlayer/PacketDataPlayerCharacterInfo.hpp"

#include "Player/Player.hpp"
#include "Player/PlayerData.hpp"
#include "World/ChunkViewRange.hpp"

class ChunkManager;

class NetworkPlayer : public Player
{
public:
    NetworkPlayer() = default;
    NetworkPlayer(sf::Vector2f position, int maxHealth = 0);

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const sf::Color& color) const override;

    // Multiplayer

    void setNetworkPlayerCharacterInfo(const PacketDataPlayerCharacterInfo& info);
    
    // Player position is of player on this machine, not for this network player
    void applyWorldWrapTranslation(sf::Vector2f playerPosition, const ChunkManager& chunkManager);

    PlayerData& getPlayerData();
    void setPlayerData(const PlayerData& playerData);

    const ChunkViewRange& getChunkViewRange();

private:
    PlayerData playerData;

    ChunkViewRange chunkViewRange;

    // Before world wrap translation applied
    sf::Vector2f fishingRodBobWorldPosUnwrapped;

};