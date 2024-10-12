#pragma once

#include <algorithm>

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/SpriteBatch.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Shaders.hpp"
#include "Core/Camera.hpp"
#include "Core/Helper.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/CollisionCircle.hpp"

#include "Entity/Projectile/Projectile.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"

#include "BossEntity.hpp"

class BossBenjaminCrow : public BossEntity
{
public:
    BossBenjaminCrow(sf::Vector2f position);

    void update(Game& game, ProjectileManager& projectileManager, sf::Vector2f playerPos, float dt) override;

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch) override;

    void handleWorldWrap(sf::Vector2f positionDelta) override;

private:
    void takeDamage(int damage);
    void applyKnockback(Projectile& projectile);

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

    float flyingHeight;

    static constexpr float MOVE_SPEED = 90.0f;

    static constexpr float MAX_FLASH_TIME = 0.3f;
    float flashTime;

    BossBenjaminState behaviourState;

    AnimatedTexture idleAnim;
};