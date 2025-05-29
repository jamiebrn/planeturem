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

#include "GUI/HitMarkers.hpp"

#include "Entity/Projectile/Projectile.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"
#include "Entity/HitRect.hpp"

#include "Network/CompactFloat.hpp"

#include "BossEntity.hpp"

class BossSandSerpent : public BossEntity
{
public:
    BossSandSerpent();
    BossSandSerpent(pl::Vector2f playerPosition, Game& game);
    BossEntity* clone() const override;

    void update(Game& game, ProjectileManager& projectileManager, std::vector<Player*>& players, float dt, int worldSize) override;
    void updateNetwork(float dt, int worldSize) override;

    bool isAlive() override;

    // void handleWorldWrap(pl::Vector2f positionDelta) override;

    // void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch) override;
    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
        const pl::Color& color) const override;

    // inline void createLightSource(LightingEngine& lightingEngine, pl::Vector2f topLeftChunkPos) const override {}

    void getHoverStats(pl::Vector2f mouseWorldPos, std::vector<std::string>& hoverStats) override;

    void testCollisionWithPlayer(Player& player, int worldSize) override;

    void testProjectileCollision(Projectile& projectile, int worldSize) override;

    void testHitRectCollision(const std::vector<HitRect>& hitRects, int worldSize) override;

    void getWorldObjects(std::vector<WorldObject*>& worldObjects) override;

    template <class Archive>
    void save(Archive& ar) const
    {
        CompactFloat<uint8_t> headFlashTimeCompact(headFlashTime, 2);
        CompactFloat<uint8_t> bodyFlashTimeCompact(bodyFlashTime, 2);
        uint8_t animFrame = animations.at(behaviourState).getFrame();
        ar(cereal::base_class<BossEntity>(this), velocity.x, velocity.y, headHealth, bodyHealth, dead, behaviourState,
            headFlashTimeCompact, bodyFlashTimeCompact, headDirection, animFrame);
    }

    template <class Archive>
    void load(Archive& ar)
    {
        CompactFloat<uint8_t> headFlashTimeCompact;
        CompactFloat<uint8_t> bodyFlashTimeCompact;
        uint8_t animFrame;
        ar(cereal::base_class<BossEntity>(this), velocity.x, velocity.y, headHealth, bodyHealth, dead, behaviourState,
            headFlashTimeCompact, bodyFlashTimeCompact, headDirection, animFrame);
        
        headFlashTime = headFlashTimeCompact.getValue(2);
        bodyFlashTime = bodyFlashTimeCompact.getValue(2);
        animations[behaviourState].setFrame(animFrame);
    }

private:
    void initialise();

    void updateCollision();

    // void setPathfindStepIndex(int index);

    bool takeHeadDamage(int damage, pl::Vector2f damagePosition);
    void takeBodyDamage(int damage, pl::Vector2f damagePosition);
    void applyKnockback(Projectile& projectile);

private:
    enum class BossSandSerpentState : uint8_t
    {
        IdleStage1,
        ShootingStage1,
        MovingToPlayer,
        Leaving
    };

private:
    static constexpr int MAX_HEAD_HEALTH = 750;
    static constexpr int MAX_BODY_HEALTH = 300;
    int16_t headHealth;
    int16_t bodyHealth;
    bool dead;

    BossSandSerpentState behaviourState;

    pl::Vector2f velocity;

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
    // pl::Vector2f pathfindLastStepPosition;
    // pl::Vector2f pathfindStepTargetPosition;
    // int pathfindStepIndex;

    static constexpr float MAX_SHOOT_COOLDOWN_TIME = 4.0f;
    static constexpr float MAX_SHOOT_PROJECTILE_COOLDOWN_TIME = 0.15f;
    static constexpr float MAX_IDLE_COOLDOWN_TIME = 3.0f;
    float shootCooldownTime;
    float shootProjectileCooldownTime;
    float idleCooldownTime;

    std::unordered_map<BossSandSerpentState, AnimatedTexture> animations;

    static const std::array<pl::Rect<int>, 4> HEAD_FRAMES;
    static const pl::Rect<int> SHOOTING_HEAD_FRAME;
    AnimatedTextureMinimal headAnimation;
    // forward = 0, left = -1, right = 1
    uint8_t headDirection;
};

CEREAL_REGISTER_TYPE(BossSandSerpent);