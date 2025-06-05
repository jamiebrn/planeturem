#pragma once

#include <algorithm>
#include <vector>
#include <array>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/memory.hpp>
#include <extlib/cereal/types/base_class.hpp>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
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
#include "Player/Achievements.hpp"

#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"

#include "GUI/HitMarkers.hpp"

#include "Entity/Projectile/Projectile.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"
#include "Entity/HitRect.hpp"

#include "Network/CompactFloat.hpp"

#include "BossEntity.hpp"

class BossBenjaminCrow : public BossEntity
{
public:
    BossBenjaminCrow();
    BossBenjaminCrow(pl::Vector2f playerPosition);
    BossEntity* clone() const override;

    void update(Game& game, ChunkManager& chunkManager, ProjectileManager& projectileManager, std::vector<Player*>& players, float dt, int worldSize) override;
    void updateNetwork(Player& player, float dt, int worldSize) override;

    bool isAlive() override;

    // void handleWorldWrap(pl::Vector2f positionDelta) override;

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const pl::Color& color) const override;

    // inline void createLightSource(LightingEngine& lightingEngine, pl::Vector2f topLeftChunkPos) const override {}

    void getHoverStats(pl::Vector2f mouseWorldPos, std::vector<std::string>& hoverStats) override;

    void testCollisionWithPlayer(Player& player, int worldSize) override;

    void testProjectileCollision(Projectile& projectile, int worldSize) override;

    void getWorldObjects(std::vector<WorldObject*>& worldObjects) override;

    template <class Archive>
    void save(Archive& ar) const
    {
        CompactFloat<uint16_t> flyingHeightCompact(flyingHeight, 2);
        uint8_t animFrame = idleAnims[stage].getFrame();
        ar(cereal::base_class<BossEntity>(this), velocity.x, velocity.y, health, dead, stage, flyingHeightCompact, animFrame, flashTime, behaviourState, dashGhostEffects);
    }
    
    template <class Archive>
    void load(Archive& ar)
    {
        CompactFloat<uint16_t> flyingHeightCompact;
        uint8_t animFrame;
        ar(cereal::base_class<BossEntity>(this), velocity.x, velocity.y, health, dead, stage, flyingHeightCompact, animFrame, flashTime, behaviourState, dashGhostEffects);

        flyingHeight = flyingHeightCompact.getValue(2);
        idleAnims[stage].setFrame(animFrame);
    }

private:
    void initialise();

    void updateCollision();

    void takeDamage(int damage, pl::Vector2f damagePosition);
    void applyKnockback(Projectile& projectile);

    bool isProjectileColliding(Projectile& projectile, int worldSize);

    void addDashGhostEffect();
    void updateDashGhostEffects(float dt);

private:
    enum class BossBenjaminState : uint8_t
    {
        Chase,
        FastChase,
        Dash,
        Killed,

        FlyAway
    };

    struct DashGhostEffect
    {
        pl::Vector2f position;
        int8_t scaleX = 1;
        int8_t stage;

        static constexpr float MAX_TIME = 0.3f;
        float timer;

        static constexpr int MAX_ALPHA = 140;
        
        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(position.x, position.y, scaleX, stage, timer);
        }
    };

private:
    static constexpr float VELOCITY_LERP_WEIGHT = 3.0f;
    pl::Vector2f velocity;
    pl::Vector2f direction;

    static constexpr float HITBOX_SIZE = 16.0f;
    CollisionCircle collision;

    static constexpr int MAX_HEALTH = 450;
    int16_t health;
    bool dead;

    static constexpr int HEALTH_SECOND_STAGE_THRESHOLD = 250;
    int8_t stage;

    float flyingHeight;

    static constexpr float DASH_TARGET_INITIATE_DISTANCE = 100.0f;
    static constexpr float DASH_TARGET_OVERSHOOT = 150.0f;
    static constexpr float DASH_TARGET_FINISH_DISTANCE = 20.0f;
    pl::Vector2f dashTargetPosition;

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

    static const int DAMAGE_HITBOX_SIZE;
    static const std::array<int, 2> damageValues;

    BossBenjaminState behaviourState;

    Tween<float> floatTween;
    static constexpr float TWEEN_DEAD_FALLING_TIME = 0.8f;
    TweenID fallingTweenID;

    std::array<AnimatedTexture, 2> idleAnims;
    static const std::array<pl::Rect<int>, 2> dashGhostTextureRects;
    static const pl::Rect<int> deadTextureRect;
    static const pl::Rect<int> shadowTextureRect;
};

CEREAL_REGISTER_TYPE(BossBenjaminCrow);