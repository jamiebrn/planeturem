#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <array>
#include <memory>
#include <optional>

#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/Tween.hpp"
#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"
#include "Core/SpriteBatch.hpp"
#include "Object/WorldObject.hpp"
#include "World/ChunkManager.hpp"
#include "World/Room.hpp"

#include "Player/InventoryData.hpp"

#include "Data/typedefs.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"
#include "Data/ArmourData.hpp"
#include "Data/ArmourDataLoader.hpp"

#include "Entity/HitRect.hpp"
#include "Entity/Projectile/Projectile.hpp"
#include "Entity/Projectile/ProjectileManager.hpp"
#include "Entity/Boss/BossEntity.hpp"

#include "GameConstants.hpp"
#include "DebugOptions.hpp"

class Player : public WorldObject
{
public:
    Player(sf::Vector2f position, InventoryData* armourInventory);

    void update(float dt, sf::Vector2f mouseWorldPos, ChunkManager& chunkManager, int worldSize, bool& wrappedAroundWorld, sf::Vector2f& wrapPositionDelta);
    void updateInStructure(float dt, sf::Vector2f mouseWorldPos, const Room& structureRoom);

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) const override;
    void drawLightMask(sf::RenderTarget& lightTexture) const override;

    void setTool(ToolType toolType);
    ToolType getTool();

    void useTool(ProjectileManager& projectileManager, InventoryData& inventory, sf::Vector2f mouseWorldPos);
    bool isUsingTool();

    // Damage
    void testHitCollision(const HitRect& hitRect);

    // Fishing rod specific
    void swingFishingRod(sf::Vector2f mouseWorldPos, sf::Vector2i selectedWorldTile);
    sf::Vector2i reelInFishingRod();
    bool isFishBitingLine();

    // Structure specific
    void enterStructure();

    // Rocket state specific
    void enterRocket(sf::Vector2f positionOverride);
    void exitRocket();
    bool isInRocket();

    bool canReachPosition(sf::Vector2f worldPos);

    void setPosition(sf::Vector2f worldPos);

    const CollisionRect& getCollisionRect();

    bool isAlive() const;
    inline int getMaxHealth() const {return maxHealth;}
    inline int getHealth() const {return health;}

private:
    void updateDirection(sf::Vector2f mouseWorldPos);
    void updateAnimation(float dt);
    void updateToolRotation(sf::Vector2f mouseWorldPos);
    void updateTimers(float dt);

    bool testWorldWrap(int worldSize, sf::Vector2f& wrapPositionDelta);

    void respawn();

    void updateFishingRodCatch(float dt);
    void castFishingRod();

    void drawFishingRodCast(sf::RenderTarget& window, float gameTime, int worldSize, float waterYOffset) const;

    void drawArmour(sf::RenderTarget& window, float waterYOffset) const;

private:
    CollisionRect collisionRect;
    sf::Vector2f direction;
    bool flippedTexture;

    AnimatedTexture idleAnimation;
    AnimatedTexture runAnimation;

    int tileReach = 4;
    float speed = 120.0f;

    int maxHealth;
    int health;
    static constexpr float MAX_DAMAGE_COOLDOWN_TIMER = 0.4f;
    float damageCooldownTimer;
    static constexpr float MAX_RESPAWN_TIMER = 10.0f;
    float respawnTimer;

    ToolType equippedTool;
    InventoryData* armourInventory;

    // Tool animation
    float toolRotation;
    Tween<float> toolTweener;
    TweenID rotationTweenID;
    // bool swingingTool;
    bool usingTool;

    // Fishing rod
    bool fishingRodCasted;
    bool swingingFishingRod;
    float fishingRodCastedTime;
    bool fishBitingLine;
    sf::Vector2f fishingRodBobWorldPos;
    sf::Vector2i fishingRodBobWorldTile;

    // In rocket state
    bool inRocket;
    sf::Vector2f rocketExitPos;

    static constexpr std::array<float, 5> runningShadowScale = {1.0f, 0.8f, 0.7f, 0.8f, 0.9f};
    
};