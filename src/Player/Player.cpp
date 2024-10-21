#include "Player/Player.hpp"
#include "Player/Cursor.hpp"

Player::Player(sf::Vector2f position, InventoryData* armourInventory)
    : WorldObject(position)
{
    collisionRect.width = 12.0f;
    collisionRect.height = 10.0f;

    collisionRect.x = position.x - collisionRect.width / 2.0f;
    collisionRect.y = position.y - collisionRect.height / 2.0f;

    drawLayer = 0;

    flippedTexture = false;

    idleAnimation.create(3, 16, 18, 0, 0, 0.3);
    runAnimation.create(5, 16, 18, 48, 0, 0.1);

    maxHealth = 5;
    health = maxHealth;
    damageCooldownTimer = 0.0f;

    equippedTool = -1;
    toolRotation = 0;
    usingTool = false;

    fishingRodCasted = false;
    swingingFishingRod = false;
    fishBitingLine = false;

    this->armourInventory = armourInventory;

    inRocket = false;
}

void Player::update(float dt, sf::Vector2f mouseWorldPos, ChunkManager& chunkManager, int worldSize, bool& wrappedAroundWorld, sf::Vector2f& wrapPositionDelta)
{
    if (inRocket)
        return;

    updateDirection(mouseWorldPos);
    updateAnimation(dt);
    updateToolRotation(mouseWorldPos);
    updateDamageCooldownTimer(dt);

    // Handle collision with world (tiles, object)

    float speedMult = 1.0f;
    if (DebugOptions::godMode)
    {
        speedMult = DebugOptions::godSpeedMultiplier;
    }

    // Test collision after x movement
    collisionRect.x += direction.x * speed * dt * speedMult;
    if (!DebugOptions::godMode)
    {
        chunkManager.collisionRectChunkStaticCollisionX(collisionRect, direction.x);
    }

    // Test collision after y movement
    collisionRect.y += direction.y * speed * dt * speedMult;
    if (!DebugOptions::godMode)
    {
        chunkManager.collisionRectChunkStaticCollisionY(collisionRect, direction.y);
    }
    
    wrappedAroundWorld = testWorldWrap(worldSize, wrapPositionDelta);

    // Update position using collision rect after collision has been handled
    position.x = collisionRect.x + collisionRect.width / 2.0f;
    position.y = collisionRect.y + collisionRect.height / 2.0f;

    // Update fishing rod if required
    if (fishingRodCasted)
    {
        updateFishingRodCatch(dt);
    }

    // Update on water
    onWater = (chunkManager.getLoadedChunkTileType(getChunkInside(worldSize), getChunkTileInside(worldSize)) == 0);
}

void Player::updateInStructure(float dt, sf::Vector2f mouseWorldPos, const Room& structureRoom)
{
    updateDirection(mouseWorldPos);
    updateAnimation(dt);
    updateToolRotation(mouseWorldPos);
    updateDamageCooldownTimer(dt);

    collisionRect.x += direction.x * speed * dt;
    structureRoom.handleStaticCollisionX(collisionRect, direction.x);

    collisionRect.y += direction.y * speed * dt;
    structureRoom.handleStaticCollisionY(collisionRect, direction.y);

    position.x = collisionRect.x + collisionRect.width / 2.0f;
    position.y = collisionRect.y + collisionRect.height / 2.0f;
}

void Player::updateDirection(sf::Vector2f mouseWorldPos)
{
    if (fishingRodCasted)
    {
        // Face towards bob
        flippedTexture = (position.x - fishingRodBobWorldPos.x) > 0;
    }
    else
    {
        // Face towards mouse cursor (overridden if moving)
        flippedTexture = (position.x - mouseWorldPos.x) > 0;
    }

    // Handle movement input
    direction.x = sf::Keyboard::isKeyPressed(sf::Keyboard::D) - sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    direction.y = sf::Keyboard::isKeyPressed(sf::Keyboard::S) - sf::Keyboard::isKeyPressed(sf::Keyboard::W);

    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0)
    {
        direction /= length;

        // Reel in fishing rod if moved
        swingingFishingRod = false;
        fishingRodCasted = false;

        if (direction.x != 0)
            flippedTexture = direction.x < 0;
    }
}

void Player::updateAnimation(float dt)
{
    // Update animation
    idleAnimation.update(dt);
    runAnimation.update(dt);

    toolTweener.update(dt);

    // if (swingingTool)
    // {
    //     if (toolTweener.isTweenFinished(rotationTweenID))
    //     {
    //         rotationTweenID = toolTweener.startTween(&toolRotation, toolRotation, 0.0f, 0.15, TweenTransition::Expo, TweenEasing::EaseOut);
    //         swingingTool = false;
    //     }
    // }
    if (usingTool)
    {
        if (toolTweener.isTweenFinished(rotationTweenID))
        {
            usingTool = false;

            if (swingingFishingRod)
            {
                castFishingRod();
            }
        }
    }
}

void Player::updateToolRotation(sf::Vector2f mouseWorldPos)
{
    // Rotate bow if tool is of type
    if (equippedTool < 0)
    {
        return;
    } 

    const ToolData& toolData = ToolDataLoader::getToolData(equippedTool);
    if (toolData.toolBehaviourType == ToolBehaviourType::BowWeapon)
    {
        if (flippedTexture)
        {
            toolRotation = 180.0f * std::atan2(mouseWorldPos.y - position.y, position.x - mouseWorldPos.x) / M_PI;
        }
        else
        {
            toolRotation = 180.0f * std::atan2(mouseWorldPos.y - position.y, mouseWorldPos.x - position.x) / M_PI;
        }
    }
}

void Player::updateDamageCooldownTimer(float dt)
{
    damageCooldownTimer = std::max(damageCooldownTimer - dt, 0.0f);
}

bool Player::testWorldWrap(int worldSize, sf::Vector2f& wrapPositionDelta)
{
    bool wrapped = false;

    // Wrap position around world
    float worldPixelSize = worldSize * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    if (collisionRect.x >= worldPixelSize) 
    {
        collisionRect.x -= worldPixelSize;
        wrapPositionDelta.x = -worldPixelSize;
        wrapped = true;
    }
    else if (collisionRect.x < 0)
    {
        collisionRect.x += worldPixelSize;
        wrapPositionDelta.x = worldPixelSize;
        wrapped = true;
    }
    if (collisionRect.y >= worldPixelSize)
    {
        collisionRect.y -= worldPixelSize;
        wrapPositionDelta.y = -worldPixelSize;
        wrapped = true;
    }
    else if (collisionRect.y < 0)
    {
        collisionRect.y += worldPixelSize;
        wrapPositionDelta.y = worldPixelSize;
        wrapped = true;
    }

    return wrapped;
}

void Player::updateFishingRodCatch(float dt)
{
    fishingRodCastedTime += dt;
    if (fishingRodCastedTime >= 1)
    {
        fishingRodCastedTime = 0;
        fishBitingLine = false;

        // Chance for fish to bite line
        int fishBiteChance = Helper::randInt(0, 5);
        if (fishBiteChance == 0)
        {
            fishBitingLine = true;
        }
    }
}

void Player::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) const
{
    if (inRocket)
        return;

    spriteBatch.endDrawing(window);

    sf::Vector2f playerScale((float)ResolutionHandler::getScale(), (float)ResolutionHandler::getScale());

    float waterYOffset = getWaterBobYOffset(worldSize, gameTime);

    if (flippedTexture)
        playerScale.x *= -1;

    float shadowScale = 1.0f;
    
    sf::IntRect animationRect;
    if (direction.x == 0 && direction.y == 0)
    {
        animationRect = idleAnimation.getTextureRect();
    }
    else
    {
        shadowScale = runningShadowScale[runAnimation.getFrame()];
        animationRect = runAnimation.getTextureRect();
    }

    TextureManager::drawTexture(window, {
        TextureType::Shadow, Camera::worldToScreenTransform(position + sf::Vector2f(0, waterYOffset)), 0, playerScale * shadowScale, {0.5, 0.85}
        });

    TextureManager::drawSubTexture(window, {TextureType::Player, Camera::worldToScreenTransform(position + sf::Vector2f(0, waterYOffset)), 0, playerScale, {0.5, 1}}, 
        animationRect);
    
    // Draw armour
    drawArmour(window, waterYOffset);

    // Draw equipped tool
    if (equippedTool >= 0)
    {
        const ToolData& toolData = ToolDataLoader::getToolData(equippedTool);

        sf::Vector2f toolPos = (Camera::worldToScreenTransform(position + sf::Vector2f(0, waterYOffset)) +
            sf::Vector2f(playerScale.x * toolData.holdOffset.x, playerScale.y * toolData.holdOffset.y));

        float pivotYOffset = 0.0f;
        
        // Add pickaxe swing pivot
        if (toolData.toolBehaviourType == ToolBehaviourType::Pickaxe)
        {
            pivotYOffset = (toolRotation / 90.0f) * 0.4;
        }

        float correctedToolRotation = toolRotation;
        if (flippedTexture)
            correctedToolRotation = -toolRotation;
        
        int textureRectIndex = 0;
        if (toolData.toolBehaviourType == ToolBehaviourType::FishingRod)
        {
            if (fishingRodCasted)
            {
                textureRectIndex = 1;
            }
        }

        TextureManager::drawSubTexture(window, {
            TextureType::Tools, toolPos, correctedToolRotation, playerScale, {toolData.pivot.x, toolData.pivot.y + pivotYOffset
            }}, toolData.textureRects[textureRectIndex]);
    }

    // Draw fishing line if casted rod
    if (fishingRodCasted)
    {
        drawFishingRodCast(window, gameTime, worldSize, waterYOffset);
    }

    // DEBUG
    if (DebugOptions::drawCollisionRects)
    {
        collisionRect.debugDraw(window);
    }
}

void Player::drawLightMask(sf::RenderTarget& lightTexture)
{
    if (inRocket)
        return;

    static constexpr float lightScale = 0.7f;
    static const sf::Color lightColor(255, 220, 140);

    sf::Vector2f scale((float)ResolutionHandler::getScale() * lightScale, (float)ResolutionHandler::getScale() * lightScale);

    sf::IntRect lightMaskRect(0, 0, 256, 256);

    TextureManager::drawSubTexture(lightTexture, {
        TextureType::LightMask, Camera::worldToScreenTransform(position), 0, scale, {0.5, 0.5}, lightColor
        }, lightMaskRect, sf::BlendAdd);
}

void Player::drawFishingRodCast(sf::RenderTarget& window, float gameTime, int worldSize, float waterYOffset) const
{
    // Draw bob
    sf::Vector2f bobPosition = fishingRodBobWorldPos + sf::Vector2f(0, WorldObject::getWaterBobYOffset(fishingRodBobWorldPos, worldSize, gameTime));

    // If fish is biting line, shake bob
    if (fishBitingLine)
    {
        bobPosition.x += (Helper::randInt(0, 2000) - 1000) / 1000.0f;
        bobPosition.y += (Helper::randInt(0, 2000) - 1000) / 1000.0f;
    }
    else
    {
        // Only draw bob if fish not on line
        TextureDrawData bobDrawData;
        bobDrawData.position = Camera::worldToScreenTransform(bobPosition);
        bobDrawData.type = TextureType::Tools;
        bobDrawData.scale = sf::Vector2f(ResolutionHandler::getScale(), ResolutionHandler::getScale());
        bobDrawData.centerRatio = sf::Vector2f(0.5, 0.5);

        TextureManager::drawSubTexture(window, bobDrawData, sf::IntRect(0, 112, 16, 16));
    }

    // Draw line
    sf::VertexArray line;
    line.setPrimitiveType(sf::Lines);

    // Calculate line origin position
    const ToolData& toolData = ToolDataLoader::getToolData(equippedTool);

    sf::Vector2f lineOrigin;
    int lineOffsetXMult = 1;
    if (flippedTexture)
    {
        lineOffsetXMult = -1;
    }

    sf::Vector2f rotatedLineOffset = Helper::rotateVector(toolData.fishingRodLineOffset, 3.14159 * toolRotation / 180.0f);

    lineOrigin.x = position.x + (toolData.holdOffset.x + rotatedLineOffset.x) * lineOffsetXMult;
    lineOrigin.y = position.y + waterYOffset + toolData.holdOffset.y + rotatedLineOffset.y;

    line.append(sf::Vertex(Camera::worldToScreenTransform(lineOrigin), sf::Color(255, 255, 255)));
    line.append(sf::Vertex(Camera::worldToScreenTransform(bobPosition), sf::Color(255, 255, 255)));

    window.draw(line);
}

void Player::drawArmour(sf::RenderTarget& window, float waterYOffset) const
{
    float scale = ResolutionHandler::getScale();

    int xScaleMult = 1;
    if (flippedTexture)
    {
        xScaleMult = -1;
    }

    sf::Vector2f armourOrigin = position - sf::Vector2f(8 * xScaleMult, 16);

    // Draw headpiece, chest, and boots
    for (int i = 0; i < 3; i++)
    {
        const std::optional<ItemCount> itemSlot = armourInventory->getItemSlotData(i);

        if (!itemSlot.has_value())
        {
            continue;
        }

        // Get data
        const ItemData& armourItemData = ItemDataLoader::getItemData(itemSlot->first);
        if (armourItemData.armourType < 0)
        {
            continue;
        }

        const ArmourData& armourData = ArmourDataLoader::getArmourData(armourItemData.armourType);

        // Draw armour piece
        int frame = idleAnimation.getFrame();
        if (direction.x != 0 || direction.y != 0)
        {
            frame = idleAnimation.getFrameCount() + runAnimation.getFrame();
        }

        const sf::IntRect& armourTextureRect = armourData.wearTextures[frame];

        TextureDrawData drawData;
        drawData.type = TextureType::Tools;
        drawData.position = Camera::worldToScreenTransform(armourOrigin + sf::Vector2f(armourData.wearTextureOffset.x * xScaleMult, armourData.wearTextureOffset.y + waterYOffset));
        drawData.scale = sf::Vector2f(scale * xScaleMult, scale);
        
        TextureManager::drawSubTexture(window, drawData, armourTextureRect);
    }
}

void Player::setTool(ToolType toolType)
{
    equippedTool = toolType;

    toolRotation = 0.0f;

    fishingRodCasted = false;
}

ToolType Player::getTool()
{
    return equippedTool;
}

void Player::useTool(ProjectileManager& projectileManager, InventoryData& inventory, sf::Vector2f mouseWorldPos)
{
    // swingingTool = true;

    const ToolData& toolData = ToolDataLoader::getToolData(equippedTool);

    switch (toolData.toolBehaviourType)
    {
        case ToolBehaviourType::Pickaxe:
        {
            usingTool = true;

            static constexpr float destRotation = 90.0f;
            static constexpr float tweenTime = 0.1f;

            rotationTweenID = toolTweener.startTween(&toolRotation, toolRotation, destRotation, tweenTime, TweenTransition::Circ, TweenEasing::EaseInOut);
            toolTweener.addTweenToQueue(rotationTweenID, &toolRotation, destRotation, 0.0f, 0.15, TweenTransition::Expo, TweenEasing::EaseOut);
            break;
        }
        case ToolBehaviourType::FishingRod:
        {
            usingTool = true;

            static constexpr float destRotation = -80.0f;
            static constexpr float tweenTime = 0.1f;

            rotationTweenID = toolTweener.startTween(&toolRotation, toolRotation, destRotation, tweenTime, TweenTransition::Circ, TweenEasing::EaseInOut);
            toolTweener.addTweenToQueue(rotationTweenID, &toolRotation, destRotation, 0.0f, 0.15, TweenTransition::Expo, TweenEasing::EaseOut);
            break;
        }
        case ToolBehaviourType::BowWeapon:
        {
            // Ensure enough projectiles in inventory for weapon
            if (inventory.getProjectileCountForWeapon(equippedTool) <= 0)
            {
                return;
            }

            // Take projectile from inventory
            const ProjectileData& projectileData = ToolDataLoader::getProjectileData(toolData.projectileShootTypes[0]);
            inventory.takeItem(projectileData.itemType, 1);

            // Calculate projectile position and angle
            float angle = 180.0f * std::atan2(mouseWorldPos.y - position.y, mouseWorldPos.x - position.x) / M_PI;

            sf::Vector2f spawnPos = static_cast<sf::Vector2f>(toolData.holdOffset);

            if (flippedTexture)
            {
                spawnPos.x *= -1;
            }

            spawnPos += position;

            // Create projectile
            // TODO: Use best projectile in inventory
            std::unique_ptr<Projectile> projectile = std::make_unique<Projectile>(spawnPos, angle, toolData.projectileShootTypes[0]);

            // Add projectile to manager
            projectileManager.addProjectile(std::move(projectile));
            break;
        }
    }

}

bool Player::isUsingTool()
{
    return usingTool;
}

void Player::testHitCollision(const HitRect& hitRect)
{
    if (damageCooldownTimer > 0)
    {
        return;
    }

    if (!collisionRect.isColliding(hitRect))
    {
        return;
    }

    // Collision
    health -= hitRect.damage;

    damageCooldownTimer = MAX_DAMAGE_COOLDOWN_TIMER;
}

void Player::swingFishingRod(sf::Vector2f mouseWorldPos, sf::Vector2i selectedWorldTile)
{
    swingingFishingRod = true;
    fishingRodCasted = false;

    fishingRodBobWorldPos.x = (std::floor(mouseWorldPos.x / TILE_SIZE_PIXELS_UNSCALED) + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
    fishingRodBobWorldPos.y = (std::floor(mouseWorldPos.y / TILE_SIZE_PIXELS_UNSCALED) + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

    fishingRodBobWorldTile = selectedWorldTile;
}

sf::Vector2i Player::reelInFishingRod()
{
    fishingRodCasted = false;
    fishBitingLine = false;

    return fishingRodBobWorldTile;
}

bool Player::isFishBitingLine()
{
    return fishBitingLine;
}

void Player::castFishingRod()
{
    swingingFishingRod = false;
    fishingRodCasted = true;

    fishBitingLine = false;
    fishingRodCastedTime = 0;
}

bool Player::canReachPosition(sf::Vector2f worldPos)
{
    // Calculate centre of tile based on world pos
    worldPos.x = (std::floor(worldPos.x / TILE_SIZE_PIXELS_UNSCALED) + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
    worldPos.y = (std::floor(worldPos.y / TILE_SIZE_PIXELS_UNSCALED) + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

    float distance = std::sqrt(std::pow(worldPos.x - position.x, 2.0) + std::pow(worldPos.y - position.y, 2.0));
    float tileDistance = distance / TILE_SIZE_PIXELS_UNSCALED;
    return (tileDistance <= tileReach);
}

void Player::enterStructure()
{
    onWater = false;
}

void Player::enterRocket(sf::Vector2f positionOverride)
{
    if (inRocket)
        return;

    inRocket = true;
    rocketExitPos = position;
    position = positionOverride;

    // Stop actions if required
    reelInFishingRod();
}

void Player::exitRocket()
{
    if (!inRocket)
        return;
    
    inRocket = false;
    position = rocketExitPos;
}

bool Player::isInRocket()
{
    return inRocket;
}

void Player::setPosition(sf::Vector2f worldPos)
{
    collisionRect.x = worldPos.x - collisionRect.width / 2.0f;
    collisionRect.y = worldPos.y - collisionRect.height / 2.0f;

    position.x = collisionRect.x + collisionRect.width / 2.0f;
    position.y = collisionRect.y + collisionRect.height / 2.0f;
}

const CollisionRect& Player::getCollisionRect()
{
    return collisionRect;
}

bool Player::isAlive()
{
    return (health > 0);
}