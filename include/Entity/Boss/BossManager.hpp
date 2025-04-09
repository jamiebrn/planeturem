#pragma once

#include <vector>
#include <array>
#include <memory>
#include <string>
#include <optional>
#include <unordered_set>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Sounds.hpp"
#include "Core/Camera.hpp"

#include "Player/Cursor.hpp"

#include "BossEntity.hpp"

class Player;
class Game;
class ChunkManager;

class BossManager
{
public:
    BossManager() = default;

    bool createBoss(const std::string& name, sf::Vector2f playerPosition, Game& game);

    void update(Game& game, ProjectileManager& projectileManager, ChunkManager& chunkManager, Player& player, float dt, float gameTime);

    void testHitRectCollision(const std::vector<HitRect>& hitRects);

    void handleWorldWrap(sf::Vector2f positionDelta);

    void stopBossMusic();

    bool isPlayingMusicBossMusic();

    void clearBosses();

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch);

    void drawStatsAtCursor(sf::RenderTarget& window, const Camera& camera, sf::Vector2f mouseScreenPos);

    void getBossWorldObjects(std::vector<WorldObject*>& worldObjects);

private:
    std::vector<std::unique_ptr<BossEntity>> bosses;
    std::unordered_set<std::string> bossAliveNames;

    static constexpr float STATS_DRAW_OFFSET_X = 24;
    static constexpr float STATS_DRAW_OFFSET_Y = 24;
    static constexpr int STATS_DRAW_SIZE = 24;
    static constexpr int STATS_DRAW_PADDING = 3;
    static constexpr int STATS_DRAW_OUTLINE_THICKNESS = 2;

};