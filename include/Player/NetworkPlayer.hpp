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

#include "Network/PacketDataPlayerInfo.hpp"

#include "Player/Player.hpp"

class NetworkPlayer : public Player
{
public:
    NetworkPlayer() = default;
    NetworkPlayer(sf::Vector2f position, int maxHealth = 0);

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const sf::Color& color) const override;

    // Multiplayer

    // Player position is of player on this machine, not for this network player
    void setNetworkPlayerInfo(const PacketDataPlayerInfo& info, std::string steamName, sf::Vector2f playerPosition, const ChunkManager& chunkManager);

private:
    std::string networkPlayerName;

    PlanetType currentPlanetType;
    RoomType roomDestType;
    
};