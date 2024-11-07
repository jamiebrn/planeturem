#pragma once

#include <vector>
#include <array>
#include <memory>
#include <string>
#include <optional>

#include <SFML/Graphics.hpp>

#include "Core/Sounds.hpp"

#include "BossEntity.hpp"
#include "BossBenjaminCrow.hpp"
#include "BossSandSerpent.hpp"

class Player;
class Game;

class BossManager
{
public:
    BossManager() = default;

    void createBoss(const std::string& name, sf::Vector2f playerPosition, Game& game);

    void update(Game& game, ProjectileManager& projectileManager, InventoryData& inventory, Player& player, float dt);

    void handleWorldWrap(sf::Vector2f positionDelta);

    void stopBossMusic();

    bool isPlayingMusicBossMusic();

    void clearBosses();

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch);

    void drawStatsAtCursor(sf::RenderTarget& window, sf::Vector2f mouseScreenPos);

    void getBossWorldObjects(std::vector<WorldObject*>& worldObjects);

private:
    std::vector<std::unique_ptr<BossEntity>> bosses;

    static constexpr float STATS_DRAW_OFFSET_X = 24;
    static constexpr float STATS_DRAW_OFFSET_Y = 24;
    static constexpr int STATS_DRAW_SIZE = 24;
    static constexpr int STATS_DRAW_PADDING = 3;

};