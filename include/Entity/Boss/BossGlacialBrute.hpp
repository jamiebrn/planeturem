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

#include "BossEntity.hpp"

class BossGlacialBrute : public BossEntity
{
public:
    BossGlacialBrute();
    BossGlacialBrute(pl::Vector2f playerPosition, Game& game, ChunkManager& chunkManager);
    BossEntity* clone() const override;

    void update(Game& game, ChunkManager& chunkManager, ProjectileManager& projectileManager, std::vector<Player*>& players, float dt, int worldSize) override;
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
        CompactFloat<uint8_t> flashTimeCompact(flashTime, 2);

        CompactFloat<uint8_t> directionXCompact(direction.x, 2);
        CompactFloat<uint8_t> directionYCompact(direction.y, 2);
        CompactFloat<uint16_t> speed(velocity.getLength(), 2);
        
        uint8_t animFrame = walkAnimation.getFrame();
        ar(cereal::base_class<BossEntity>(this), speed, directionXCompact, directionYCompact, health, behaviourState, flashTimeCompact, animFrame);
    }
    
    template <class Archive>
    void load(Archive& ar)
    {
        CompactFloat<uint8_t> flashTimeCompact;

        CompactFloat<uint8_t> directionXCompact;
        CompactFloat<uint8_t> directionYCompact;
        CompactFloat<uint16_t> speed;

        uint8_t animFrame;
        ar(cereal::base_class<BossEntity>(this), speed, directionXCompact, directionYCompact, health, behaviourState, flashTimeCompact, animFrame);
        
        flashTime = flashTimeCompact.getValue(2);

        direction.x = directionXCompact.getValue(2);
        direction.y = directionYCompact.getValue(2);
        velocity = direction * speed.getValue(2);

        walkAnimation.setFrame(animFrame);
    }

private:
    void initialise();

    void damage(int amount, pl::Vector2f damageSource);

    void updateCollision();

    void throwSnowball(ProjectileManager& projectileManager, Player& player, int worldSize);

private:
    enum class BossGlacialBruteState : uint8_t
    {
        WalkingToPlayer,
        LeavingPlayer,
        ThrowSnowball
    };

private:
    BossGlacialBruteState behaviourState;

    pl::Vector2f velocity;
    pl::Vector2f direction;

    AnimatedTexture walkAnimation;

    static const pl::Rect<int> shadowTextureRect;

    static constexpr int MAX_HEALTH = 3500;
    int16_t health;

    static constexpr float MAX_FLASH_TIME = 0.3f;
    float flashTime = 0.0f;

    static constexpr float SNOWBALL_THROW_DISTANCE_THRESHOLD = 300.0f;
    static constexpr float MAX_SNOWBALL_THROW_COOLDOWN = 2.0f;
    static constexpr float MAX_SNOWBALL_CHARGE_TIME = 0.8f;
    static constexpr float MIN_SNOWBALL_CHARGE_TIME = 0.2f;
    float throwSnowballCooldown = 0.0f;
    float throwSnowballTimer = 0.0f;

    static constexpr float LEAVE_SPEED_MULT = 2.4f;

    static constexpr float BODY_DAMAGE = 200.0f;

    CollisionRect hitCollision;

    PathFollower pathFollower;
};

CEREAL_REGISTER_TYPE(BossGlacialBrute);