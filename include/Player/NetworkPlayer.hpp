#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <vector>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Vector.hpp>

#include "Core/ResolutionHandler.hpp"
#include "Core/TextDraw.hpp"

#include "Data/typedefs.hpp"
#include "Types/GameState.hpp"

#include "Network/PacketData/PacketDataPlayer/PacketDataPlayerCharacterInfo.hpp"

#include "Player/Player.hpp"
#include "Player/PlayerData.hpp"
#include "World/ChunkViewRange.hpp"

#include "GameConstants.hpp"

class Game;
class ChunkManager;

class NetworkPlayer : public Player
{
public:
    NetworkPlayer() = default;
    NetworkPlayer(pl::Vector2f position, int maxHealth = 0);

    void updateNetworkPlayer(float dt, Game& game);

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const pl::Color& color) const override;

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera* camera, float dt, float gameTime, int worldSize,
        const pl::Color& color, bool drawName) const;

    // Multiplayer
    void setNetworkPlayerCharacterInfo(const PacketDataPlayerCharacterInfo& info);
    
    // Player position is of player on this machine, not for this network player
    // void applyWorldWrapTranslation(pl::Vector2f playerPosition, const ChunkManager& chunkManager);

    PlayerData& getPlayerData();
    void setPlayerData(const PlayerData& playerData);

    const ChunkViewRange& getChunkViewRange();

private:
    PlayerData playerData;

    ChunkViewRange chunkViewRange;
    
    pl::Vector2f positionNext;
    float toolRotationNext;

    // Before world wrap translation applied
    // pl::Vector2f fishingRodBobWorldPos;

};