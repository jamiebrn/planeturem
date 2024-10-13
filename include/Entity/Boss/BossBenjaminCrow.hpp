#pragma once

#include <algorithm>
#include <vector>

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/SpriteBatch.hpp"
#include "Core/TextDraw.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Shaders.hpp"
#include "Core/Camera.hpp"
#include "Core/Helper.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/CollisionCircle.hpp"

#include "Player/InventoryData.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"

#include "GUI/InventoryGUI.hpp"

#include "Entity/Projectile/Projectile.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"

#include "BossEntity.hpp"

class BossBenjaminCrow : public BossEntity
{
public:
    BossBenjaminCrow(sf::Vector2f position);

    void update(Game& game, ProjectileManager& projectileManager, InventoryData& inventory, sf::Vector2f playerPos, float dt) override;

    bool isAlive() override;

    void handleWorldWrap(sf::Vector2f positionDelta) override;

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch) override;

    void drawStatsAtCursor(sf::RenderTarget& window, sf::Vector2f mouseScreenPos) override;

private:
    void updateCollision();

    void takeDamage(int damage, InventoryData& inventory);
    void applyKnockback(Projectile& projectile);

    void giveItemDrops(InventoryData& inventory);

    bool isProjectileColliding(Projectile& projectile);

private:
    enum class BossBenjaminState
    {
        Idle,
        Dash
    };

private:
    sf::Vector2f position;

    static constexpr float VELOCITY_LERP_WEIGHT = 3.0f;
    sf::Vector2f velocity;
    sf::Vector2f direction;

    static constexpr float hitboxSize = 16.0f;
    CollisionCircle collision;

    static constexpr int MAX_HEALTH = 50;
    int health;

    float flyingHeight;

    static constexpr float MOVE_SPEED = 90.0f;

    static constexpr float MAX_FLASH_TIME = 0.3f;
    float flashTime;

    BossBenjaminState behaviourState;

    AnimatedTexture idleAnim;
};