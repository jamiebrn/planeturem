#pragma once

#include <vector>

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <vector>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/Tween.hpp"
#include "Core/Helper.hpp"
#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"
#include "Core/InputManager.hpp"
#include "Core/Camera.hpp"
#include "Object/WorldObject.hpp"
#include "World/ChunkManager.hpp"
#include "World/Room.hpp"
#include "World/LightingEngine.hpp"

#include "Player/InventoryData.hpp"

#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"
#include "Data/ArmourData.hpp"
#include "Data/ArmourDataLoader.hpp"

#include "Entity/HitRect.hpp"
#include "Entity/Projectile/Projectile.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"
#include "Entity/Boss/BossEntity.hpp"

#include "GUI/HitMarkers.hpp"

#include "Network/PacketData/PacketDataPlayer/PacketDataPlayerCharacterInfo.hpp"

#include "GameConstants.hpp"
#include "DebugOptions.hpp"

class Game;

class Player : public WorldObject
{
public:
    Player() = default;
    Player(pl::Vector2f position, int maxHealth = 0);

    void update(float dt, pl::Vector2f mouseWorldPos, ChunkManager& chunkManager, ProjectileManager& projectileManager,
        bool& wrappedAroundWorld, pl::Vector2f& wrapPositionDelta);
    void updateInRoom(float dt, pl::Vector2f mouseWorldPos, const Room& room);

    virtual void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const pl::Color& color) const override;
    void createLightSource(LightingEngine& lightingEngine, pl::Vector2f topLeftChunkPos) const override;

    void setTool(ToolType toolType);
    ToolType getTool();

    void useTool(ProjectileManager& projectileManager, InventoryData& inventory, pl::Vector2f mouseWorldPos, Game& game);
    bool isUsingTool();

    void startUseToolTimer();
    bool isUseToolTimerFinished();

    void setArmourFromInventory(const InventoryData& armourInventory);

    void setCanMove(bool value);

    // Damage
    bool testHitCollision(const Projectile& projectile);
    bool testHitCollision(const HitRect& hitRect);
    bool testHitCollision(const HitCircle& hitCircle);

    // Consumable
    bool useConsumable(const ConsumableData& consumable);

    // Fishing rod specific
    void swingFishingRod(pl::Vector2f mouseWorldPos, pl::Vector2<int> selectedWorldTile);
    pl::Vector2<int> reelInFishingRod();
    bool isFishBitingLine();

    // Structure specific
    void enterStructure();

    // Rocket state specific
    void enterRocket(pl::Vector2f positionOverride);
    void exitRocket();
    bool isInRocket();

    bool canReachPosition(pl::Vector2f worldPos);

    void setPosition(pl::Vector2f worldPos);

    const CollisionRect& getCollisionRect();

    bool isAlive() const;
    inline int getMaxHealth() const {return maxHealth;}
    inline float getHealth() const {return health;}
    inline float getHealthConsumableTimerMax() const {return healthConsumableTimerMax;}
    inline float getHealthConsumableTimer() const {return healthConsumableTimer;}

    // Multiplayer
    
    virtual PacketDataPlayerCharacterInfo getNetworkPlayerInfo(const Camera* camera, uint64_t steamID);


protected:
    void updateDirection(pl::Vector2f mouseWorldPos);
    void updateMovement(float dt, ChunkManager& chunkManager, bool isLocalPlayer = true);
    void updateMovementInRoom(float dt, const Room& room, bool isLocalPlayer = true);
    void updateAnimation(float dt);
    void updateToolRotation(pl::Vector2f mouseWorldPos);
    void updateTimers(float dt);

private:
    bool testWorldWrap(int worldSize, pl::Vector2f& wrapPositionDelta);

    bool takeDamage(float rawAmount);

    void respawn();

    void updateFishingRodCatch(float dt);
    void castFishingRod();

    void drawFishingRodCast(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, float gameTime, int worldSize, float waterYOffset) const;

    void drawMeleeSwing(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera) const;

    void drawArmour(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, float waterYOffset) const;

protected:
    CollisionRect collisionRect;
    pl::Vector2f direction;
    bool flippedTexture;

    AnimatedTexture idleAnimation;
    AnimatedTexture runAnimation;

    int tileReach = 4;
    float speed = 120.0f;

    bool canMove;

    static constexpr float BASE_HEALTH_REGEN_RATE = 3.0f;
    static constexpr float MAX_HEALTH_REGEN_COOLDOWN_TIMER = 5.0f;
    float healthRegenCooldownTimer;
    static constexpr int INITIAL_MAX_HEALTH = 150;
    int maxHealth;
    float health;
    static constexpr float MAX_DAMAGE_COOLDOWN_TIMER = 0.4f;
    float damageCooldownTimer;
    static constexpr float MAX_RESPAWN_TIMER = 10.0f;
    float respawnTimer;

    static constexpr float PLAYER_Y_SCALE_LERP_WEIGHT = 10.0f;
    float playerYScaleMult;

    ToolType equippedTool;
    // InventoryData* armourInventory = nullptr;
    std::array<ArmourType, 3> armour;

    // Tool animation
    float toolRotation;
    Tween<float> toolTweener;
    TweenID rotationTweenID;
    // bool swingingTool;
    bool usingTool;
    static constexpr float MELEE_SWING_Y_ORIGIN_OFFSET = -4.0f;
    static constexpr float MELEE_SWING_RADIUS = 13.0f;
    std::vector<HitRect> meleeHitRects;
    AnimatedTexture meleeSwingAnimation;
    float meleeSwingAnimationRotation;

    static constexpr float MAX_USE_TOOL_COOLDOWN = 0.3f;
    float useToolCooldown;

    // Fishing rod
    bool fishingRodCasted;
    bool swingingFishingRod;
    float fishingRodCastedTime;
    bool fishBitingLine;
    pl::Vector2f fishingRodBobWorldPos;
    pl::Vector2<int> fishingRodBobWorldTile;

    // Consumable
    float healthConsumableTimerMax;
    float healthConsumableTimer;

    // In rocket state
    bool inRocket;
    pl::Vector2f rocketExitPos;

    static constexpr std::array<float, 5> runningShadowScale = {1.0f, 0.8f, 0.7f, 0.8f, 0.9f};
    
};