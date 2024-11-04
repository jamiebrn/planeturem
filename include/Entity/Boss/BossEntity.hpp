#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

#include "Core/SpriteBatch.hpp"
#include "Core/Helper.hpp"

#include "Player/InventoryData.hpp"

#include "Object/WorldObject.hpp"

#include "Entity/Projectile/Projectile.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"

class Game;
class Player;

class BossEntity : public WorldObject
{
public:
    BossEntity() = default;

    virtual void update(Game& game, ProjectileManager& projectileManager, InventoryData& inventory, Player& player, float dt) = 0;

    virtual bool isAlive() = 0;

    virtual void handleWorldWrap(sf::Vector2f positionDelta) = 0;

    // virtual void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch) = 0;
    virtual void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, float dt, float gameTime, int worldSize, const sf::Color& color) const = 0;

    virtual void createLightSource(LightingEngine& lightingEngine, sf::Vector2f topLeftChunkPos) const = 0;

    virtual void drawStatsAtCursor(sf::RenderTarget& window, sf::Vector2f mouseScreenPos) = 0;

    virtual void testCollisionWithPlayer(Player& player) = 0;

    virtual void getWorldObjects(std::vector<WorldObject*>& worldObjects) = 0;

    // Test for despawn
    bool inPlayerRange(Player& player);

protected:
    static constexpr float STATS_DRAW_OFFSET_X = 24;
    static constexpr float STATS_DRAW_OFFSET_Y = 24;
    static constexpr int STATS_DRAW_SIZE = 24;

    float playerMaxRange = 1000.0f;
};