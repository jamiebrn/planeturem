#pragma once

#include <vector>
#include <memory>
#include <string>

#include <SFML/Graphics.hpp>

#include "Core/Sounds.hpp"

#include "BossEntity.hpp"
#include "BossBenjaminCrow.hpp"

class BossManager
{
public:
    BossManager() = default;

    void createBoss(const std::string& name, sf::Vector2f position);

    void update(Game& game, ProjectileManager& projectileManager, InventoryData& inventory, sf::Vector2f playerPos, float dt);

    void handleWorldWrap(sf::Vector2f positionDelta);

    void stopBossMusic();

    void clearBosses();

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch);

    void drawStatsAtCursor(sf::RenderTarget& window, sf::Vector2f mouseScreenPos);

private:
    std::vector<std::unique_ptr<BossEntity>> bosses;

};