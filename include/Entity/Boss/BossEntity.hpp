#pragma once

#include <SFML/Graphics.hpp>

#include "Core/SpriteBatch.hpp"
#include "Player/InventoryData.hpp"

#include "Entity/Projectile/Projectile.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"

class Game;

class BossEntity
{
public:
    BossEntity() = default;

    virtual void update(Game& game, ProjectileManager& projectileManager, InventoryData& inventory, sf::Vector2f playerPos, float dt) = 0;

    virtual bool isAlive() = 0;

    virtual void handleWorldWrap(sf::Vector2f positionDelta) = 0;

    virtual void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch) = 0;

    virtual void drawStatsAtCursor(sf::RenderTarget& window, sf::Vector2f mouseScreenPos) = 0;

protected:
    static constexpr float STATS_DRAW_OFFSET_X = 24;
    static constexpr float STATS_DRAW_OFFSET_Y = 24;
    static constexpr int STATS_DRAW_SIZE = 24;
};