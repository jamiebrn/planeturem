#pragma once

#include <algorithm>
#include <vector>
#include <array>

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/SpriteBatch.hpp"
#include "Core/TextDraw.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Shaders.hpp"
#include "Core/Camera.hpp"
#include "Core/Tween.hpp"
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

// TODO: Ghost effect when dash

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

    void addDashGhostEffect();
    void updateDashGhostEffects(float dt);

private:
    enum class BossBenjaminState
    {
        Chase,
        FastChase,
        Dash,
        Killed
    };

    struct DashGhostEffect
    {
        sf::Vector2f position;
        int scaleX = 1;
        int stage;

        static constexpr float MAX_TIME = 0.3f;
        float timer;

        static constexpr int MAX_ALPHA = 140;
    };

private:
    sf::Vector2f position;

    static constexpr float VELOCITY_LERP_WEIGHT = 3.0f;
    sf::Vector2f velocity;
    sf::Vector2f direction;

    static constexpr float hitboxSize = 16.0f;
    CollisionCircle collision;

    static constexpr int MAX_HEALTH = 100;
    int health;
    bool dead;

    static constexpr int HEALTH_SECOND_STAGE_THRESHOLD = 40;
    int stage;

    float flyingHeight;

    static constexpr float DASH_TARGET_INITIATE_DISTANCE = 100.0f;
    static constexpr float DASH_TARGET_OVERSHOOT = 150.0f;
    static constexpr float DASH_TARGET_FINISH_DISTANCE = 20.0f;
    sf::Vector2f dashTargetPosition;

    static constexpr float MAX_DASH_COOLDOWN_TIMER = 1.0f;
    static constexpr float SECOND_STAGE_MAX_DASH_COOLDOWN_TIMER = 0.6f;
    float dashCooldownTimer;

    static constexpr float FAST_CHASE_DISTANCE_THRESHOLD = 300.0f;

    static constexpr float MOVE_SPEED = 180.0f;
    static constexpr float FAST_CHASE_MOVE_SPEED = 280.0f;
    static constexpr float DASH_MOVE_SPEED = 550.0f;
    static constexpr float SECOND_STAGE_SPEED_MULTIPLIER = 1.6f;
    float currentMoveSpeed;

    static constexpr float MAX_FLASH_TIME = 0.3f;
    float flashTime;

    std::vector<DashGhostEffect> dashGhostEffects;
    static constexpr float MAX_DASH_GHOST_TIMER = 0.05f;
    float dashGhostTimer;

    BossBenjaminState behaviourState;

    Tween<float> floatTween;
    static constexpr float TWEEN_DEAD_FALLING_TIME = 0.8f;
    TweenID fallingTweenID;

    std::array<AnimatedTexture, 2> idleAnims;
    static const std::array<sf::IntRect, 2> dashGhostTextureRects;
    static const sf::IntRect deadTextureRect;
    static const sf::IntRect shadowTextureRect;
};