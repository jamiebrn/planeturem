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
#include "Core/Sounds.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/CollisionCircle.hpp"

#include "Player/InventoryData.hpp"
#include "Player/Player.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"

#include "World/ChunkManager.hpp"
#include "World/PathfindingEngine.hpp"

#include "GUI/InventoryGUI.hpp"
#include "GUI/HitMarkers.hpp"

#include "Entity/Projectile/Projectile.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"
#include "Entity/HitRect.hpp"

#include "BossEntity.hpp"

class BossSandSerpent : public BossEntity
{
public:
    BossSandSerpent(sf::Vector2f playerPosition, Game& game);

    void update(Game& game, ProjectileManager& enemyProjectileManager, Player& player, float dt) override;

    bool isAlive() override;

    void handleWorldWrap(sf::Vector2f positionDelta) override;

    // void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch) override;
    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
        const sf::Color& color) const override;

    inline void createLightSource(LightingEngine& lightingEngine, sf::Vector2f topLeftChunkPos) const override {}

    void getHoverStats(sf::Vector2f mouseWorldPos, std::vector<std::string>& hoverStats) override;

    void testCollisionWithPlayer(Player& player) override;

    void testProjectileCollision(Projectile& projectile) override;

    void testHitRectCollision(const std::vector<HitRect>& hitRects) override;

    void getWorldObjects(std::vector<WorldObject*>& worldObjects) override;

private:
    void updateCollision();

    // void setPathfindStepIndex(int index);

    bool takeHeadDamage(int damage, sf::Vector2f damagePosition);
    void takeBodyDamage(int damage, sf::Vector2f damagePosition);
    void applyKnockback(Projectile& projectile);

private:
    enum class BossSandSerpentState
    {
        IdleStage1,
        ShootingStage1,
        MovingToPlayer,
        Leaving
    };

private:
    static constexpr int MAX_HEAD_HEALTH = 750;
    static constexpr int MAX_BODY_HEALTH = 300;
    int headHealth;
    int bodyHealth;
    bool dead;

    BossSandSerpentState behaviourState;

    static constexpr int HEAD_HITBOX_RADIUS = 17;
    static constexpr int BODY_HITBOX_WIDTH = 56;
    static constexpr int BODY_HITBOX_HEIGHT = 32;
    CollisionCircle headCollision;
    CollisionRect bodyCollision;

    static constexpr float MAX_FLASH_TIME = 0.3f;
    float headFlashTime;
    float bodyFlashTime;

    static constexpr int START_MOVE_PLAYER_DISTANCE = 300;
    PathFollower pathFollower;
    // std::vector<PathfindGridCoordinate> pathfindStepSequence;
    // sf::Vector2f pathfindLastStepPosition;
    // sf::Vector2f pathfindStepTargetPosition;
    // int pathfindStepIndex;

    static constexpr float MAX_SHOOT_COOLDOWN_TIME = 4.0f;
    static constexpr float MAX_SHOOT_PROJECTILE_COOLDOWN_TIME = 0.15f;
    static constexpr float MAX_IDLE_COOLDOWN_TIME = 3.0f;
    float shootCooldownTime;
    float shootProjectileCooldownTime;
    float idleCooldownTime;

    std::unordered_map<BossSandSerpentState, AnimatedTexture> animations;

    static const std::array<sf::IntRect, 4> HEAD_FRAMES;
    static const sf::IntRect SHOOTING_HEAD_FRAME;
    AnimatedTextureMinimal headAnimation;
    // forward = 0, left = -1, right = 1
    int headDirection;
};