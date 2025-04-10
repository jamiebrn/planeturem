#include "Player/Player.hpp"
#include "Player/Cursor.hpp"
#include "Game.hpp"

Player::Player(pl::Vector2f position, int maxHealth)
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

    canMove = true;

    this->maxHealth = (maxHealth == 0) ? INITIAL_MAX_HEALTH : maxHealth;
    health = this->maxHealth;
    healthRegenCooldownTimer = 0.0f;
    damageCooldownTimer = 0.0f;
    respawnTimer = 0.0f;
    healthConsumableTimer = 0.0f;

    playerYScaleMult = 1.0f;

    equippedTool = -1;
    toolRotation = 0;
    usingTool = false;

    armour = {-1, -1, -1};

    meleeSwingAnimation.create(7, 5, 13, 201, 35, 0.025f, false);
    meleeSwingAnimation.setFrame(6);

    useToolCooldown = 0.0f;

    fishingRodCasted = false;
    swingingFishingRod = false;
    fishBitingLine = false;

    inRocket = false;
}

void Player::update(float dt, pl::Vector2f mouseWorldPos, ChunkManager& chunkManager, ProjectileManager& projectileManager,
    bool& wrappedAroundWorld, pl::Vector2f& wrapPositionDelta)
{
    updateTimers(dt);

    if (inRocket || !isAlive())
    {
        return;
    }

    updateDirection(mouseWorldPos);
    updateAnimation(dt);
    updateToolRotation(mouseWorldPos);

    // Handle collision with world (tiles, object)

    updateMovement(dt, chunkManager);
    
    wrappedAroundWorld = testWorldWrap(chunkManager.getWorldSize(), wrapPositionDelta);

    // Update position using collision rect after collision has been handled
    position.x = collisionRect.x + collisionRect.width / 2.0f;
    position.y = collisionRect.y + collisionRect.height / 2.0f;

    // Test projectile collisions
    for (auto& projectilePair : projectileManager.getProjectiles())
    {
        if (testHitCollision(projectilePair.second))
        {
            projectilePair.second.onCollision();
        }
    }

    // Update fishing rod if required
    if (fishingRodCasted)
    {
        updateFishingRodCatch(dt);
    }

    // Update on water
    onWater = (chunkManager.getLoadedChunkTileType(getChunkInside(chunkManager.getWorldSize()), getChunkTileInside(chunkManager.getWorldSize())) == 0);
}

void Player::updateInRoom(float dt, pl::Vector2f mouseWorldPos, const Room& room)
{
    // updateTimers(dt);
    
    if (inRocket || !isAlive())
    {
        return;
    }
    
    updateDirection(mouseWorldPos);
    updateAnimation(dt);
    updateToolRotation(mouseWorldPos);

    updateMovementInRoom(dt, room);

    position.x = collisionRect.x + collisionRect.width / 2.0f;
    position.y = collisionRect.y + collisionRect.height / 2.0f;
}

void Player::updateDirection(pl::Vector2f mouseWorldPos)
{
    if (!canMove)
    {
        return;
    }

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
    direction.x = InputManager::getActionAxisActivation(InputAction::WALK_LEFT, InputAction::WALK_RIGHT);
    direction.y = InputManager::getActionAxisActivation(InputAction::WALK_UP, InputAction::WALK_DOWN);

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


void Player::updateMovement(float dt, ChunkManager& chunkManager, bool isLocalPlayer)
{
    float speedMult = 1.0f;
    #if (!RELEASE_BUILD)
    if (DebugOptions::godMode && isLocalPlayer)
    {
        speedMult = DebugOptions::godSpeedMultiplier;
    }
    #endif

    // Test collision after x movement
    collisionRect.x += direction.x * speed * dt * speedMult;
    #if (!RELEASE_BUILD)
    if (!DebugOptions::godMode || !isLocalPlayer)
    #endif
    {
        chunkManager.collisionRectChunkStaticCollisionX(collisionRect, direction.x);
    }

    // Test collision after y movement
    collisionRect.y += direction.y * speed * dt * speedMult;
    #if (!RELEASE_BUILD)
    if (!DebugOptions::godMode || !isLocalPlayer)
    #endif
    {
        chunkManager.collisionRectChunkStaticCollisionY(collisionRect, direction.y);
    }
}

void Player::updateMovementInRoom(float dt, const Room& room, bool isLocalPlayer)
{
    float speedMult = 1.0f;
    #if (!RELEASE_BUILD)
    if (DebugOptions::godMode && isLocalPlayer)
    {
        speedMult = DebugOptions::godSpeedMultiplier;
    }
    #endif

    collisionRect.x += direction.x * speed * dt * speedMult;
    #if (!RELEASE_BUILD)
    if (!DebugOptions::godMode || !isLocalPlayer)
    #endif
    {
        room.handleStaticCollisionX(collisionRect, direction.x);
    }

    collisionRect.y += direction.y * speed * dt * speedMult;
    #if (!RELEASE_BUILD)
    if (!DebugOptions::godMode || !isLocalPlayer)
    #endif
    {
        room.handleStaticCollisionY(collisionRect, direction.y);
    }
}

void Player::updateAnimation(float dt)
{
    // Update animation
    if (!usingTool)
    {
        idleAnimation.update(dt);
    }
    runAnimation.update(dt);

    toolTweener.update(dt);

    playerYScaleMult = Helper::lerp(playerYScaleMult, 1.0f, PLAYER_Y_SCALE_LERP_WEIGHT * dt);

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

    if (equippedTool >= 0)
    {
        const ToolData& toolData = ToolDataLoader::getToolData(equippedTool);

        if (toolData.toolBehaviourType == ToolBehaviourType::MeleeWeapon)
        {
            meleeSwingAnimation.update(dt);
        }
    }
}

void Player::updateToolRotation(pl::Vector2f mouseWorldPos)
{
    // Rotate bow if tool is of type
    if (equippedTool < 0)
    {
        return;
    } 

    const ToolData& toolData = ToolDataLoader::getToolData(equippedTool);
    if (toolData.toolBehaviourType == ToolBehaviourType::BowWeapon || toolData.toolBehaviourType == ToolBehaviourType::MeleeWeapon)
    {
        if (flippedTexture)
        {
            toolRotation = 180.0f * std::atan2(mouseWorldPos.y - position.y, position.x - mouseWorldPos.x) / M_PI + 
                (usingTool ? toolRotation : 0);
        }
        else
        {
            toolRotation = 180.0f * std::atan2(mouseWorldPos.y - position.y, mouseWorldPos.x - position.x) / M_PI + 
                (usingTool ? toolRotation : 0);
        }
    }

    // Update melee collision rects
    if (toolData.toolBehaviourType == ToolBehaviourType::MeleeWeapon)
    {
        meleeHitRects.clear();
        float angle = std::atan2(mouseWorldPos.y - position.y + MELEE_SWING_Y_ORIGIN_OFFSET, mouseWorldPos.x - position.x);
        constexpr float angleStep = M_PI / 8.0f;
        constexpr float collisionSize = 7.0f;

        for (int i = -2; i <= 2; i++)
        {
            HitRect rect;
            rect.x = position.x + std::cos(angle + i * angleStep) * MELEE_SWING_RADIUS - collisionSize / 2.0f;
            rect.y = position.y + MELEE_SWING_Y_ORIGIN_OFFSET + std::sin(angle + i * angleStep) * MELEE_SWING_RADIUS - collisionSize / 2.0f;
            rect.width = collisionSize;
            rect.height = collisionSize;
            rect.damage = toolData.damage;
            meleeHitRects.push_back(rect);
        }

        meleeSwingAnimationRotation = angle;
    }
}

void Player::updateTimers(float dt)
{
    healthRegenCooldownTimer = std::max(healthRegenCooldownTimer - dt, 0.0f);

    if (healthRegenCooldownTimer <= 0 && health < maxHealth)
    {
        health = std::min(health + BASE_HEALTH_REGEN_RATE * dt, static_cast<float>(maxHealth));
    }

    damageCooldownTimer = std::max(damageCooldownTimer - dt, 0.0f);

    if (respawnTimer > 0)
    {
        respawnTimer -= dt;
        if (respawnTimer <= 0)
        {
            respawn();
        }
    }

    healthConsumableTimer = std::max(healthConsumableTimer - dt, 0.0f);

    useToolCooldown = std::max(useToolCooldown - dt, 0.0f);
}

bool Player::testWorldWrap(int worldSize, pl::Vector2f& wrapPositionDelta)
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

void Player::respawn()
{
    health = maxHealth;
    healthConsumableTimer = 0.0f;
}

void Player::updateFishingRodCatch(float dt)
{
    if (equippedTool < 0)
    {
        return;
    }

    fishingRodCastedTime += dt;
    if (fishingRodCastedTime >= 1)
    {
        fishingRodCastedTime = 0;
        fishBitingLine = false;

        const ToolData& fishingRodToolData = ToolDataLoader::getToolData(equippedTool);

        // Chance for fish to bite line
        int fishBiteChance = Helper::randFloat(0.0f, 5.0f / fishingRodToolData.fishingEfficiency);
        if (fishBiteChance < 1)
        {
            fishBitingLine = true;
        }
    }
}

void Player::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const pl::Color& color) const
{
    if (inRocket || !isAlive())
    {
        return;
    }

    pl::Vector2f playerScale((float)ResolutionHandler::getScale(), (float)ResolutionHandler::getScale());

    float waterYOffset = getWaterBobYOffset(worldSize, gameTime);

    if (flippedTexture)
        playerScale.x *= -1;

    float shadowScale = 1.0f;
    
    pl::Rect<int> animationRect;
    if (direction.x == 0 && direction.y == 0)
    {
        animationRect = idleAnimation.getTextureRect();
    }
    else
    {
        shadowScale = runningShadowScale[runAnimation.getFrame()];
        animationRect = runAnimation.getTextureRect();
    }

    pl::DrawData shadowDrawData;
    shadowDrawData.texture = TextureManager::getTexture(TextureType::Shadow);
    shadowDrawData.shader = Shaders::getShader(ShaderType::Default);
    shadowDrawData.position = camera.worldToScreenTransform(position + pl::Vector2f(0, waterYOffset));
    shadowDrawData.scale = playerScale * shadowScale;
    shadowDrawData.centerRatio = pl::Vector2f(0.5f, 0.85f);
    shadowDrawData.textureRect = pl::Rect<int>(0, 0, shadowDrawData.texture->getWidth(), shadowDrawData.texture->getHeight());

    spriteBatch.draw(window, shadowDrawData);
    
    // Draw melee swing behind player if required
    bool meleeSwingDrawn = false;
    if (equippedTool >= 0)
    {
        const ToolData& toolData = ToolDataLoader::getToolData(equippedTool);
        if (toolData.toolBehaviourType == ToolBehaviourType::MeleeWeapon && meleeSwingAnimationRotation < 0.0f)
        {
            drawMeleeSwing(window, spriteBatch, camera);
            meleeSwingDrawn = true;
        }
    }

    playerScale.y *= playerYScaleMult;

    pl::DrawData playerDrawData;
    playerDrawData.texture = TextureManager::getTexture(TextureType::Player);
    playerDrawData.shader = Shaders::getShader(ShaderType::Default);
    playerDrawData.position = camera.worldToScreenTransform(position + pl::Vector2f(0, waterYOffset));
    playerDrawData.scale = playerScale;
    playerDrawData.centerRatio = pl::Vector2f(0.5f, 1.0f);
    playerDrawData.textureRect = animationRect;
    
    if (damageCooldownTimer > 0.0f)
    {
        playerDrawData.shader = Shaders::getShader(ShaderType::Flash);
        playerDrawData.shader->setUniform1f("flash_amount", damageCooldownTimer / MAX_DAMAGE_COOLDOWN_TIMER);
    }

    spriteBatch.draw(window, playerDrawData);
    
    // Draw armour
    drawArmour(window, spriteBatch, camera, waterYOffset);

    // Draw equipped tool
    if (equippedTool >= 0)
    {
        const ToolData& toolData = ToolDataLoader::getToolData(equippedTool);

        pl::Vector2f toolPos = (camera.worldToScreenTransform(position + pl::Vector2f(0, waterYOffset)) +
            pl::Vector2f(playerScale.x * toolData.holdOffset.x, playerScale.y * toolData.holdOffset.y));

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

        // Draw melee swing if required
        if (toolData.toolBehaviourType == ToolBehaviourType::MeleeWeapon && !meleeSwingDrawn)
        {
            drawMeleeSwing(window, spriteBatch, camera);
        }

        pl::DrawData toolDrawData;
        toolDrawData.texture = TextureManager::getTexture(TextureType::Tools);
        toolDrawData.shader = Shaders::getShader(ShaderType::Default);
        toolDrawData.position = toolPos;
        toolDrawData.rotation = correctedToolRotation;
        toolDrawData.scale = playerScale;
        toolDrawData.centerRatio = pl::Vector2f(toolData.pivot.x, toolData.pivot.y + pivotYOffset);
        toolDrawData.textureRect = toolData.textureRects[textureRectIndex];

        spriteBatch.draw(window, toolDrawData);
        
        #if (!RELEASE_BUILD)
        // DEBUG
        if (DebugOptions::drawCollisionRects)
        {
            if (toolData.toolBehaviourType == ToolBehaviourType::MeleeWeapon)
            {
                spriteBatch.endDrawing(window);
                for (const CollisionRect& rect : meleeHitRects)
                {
                    rect.debugDraw(window, camera);
                }
            }
        }
        #endif
    }

    // Draw fishing line if casted rod
    if (fishingRodCasted)
    {
        drawFishingRodCast(window, spriteBatch, camera, gameTime, worldSize, waterYOffset);
    }

    #if (!RELEASE_BUILD)
    // DEBUG
    if (DebugOptions::drawCollisionRects)
    {
        collisionRect.debugDraw(window, camera);
    }
    #endif
}

void Player::createLightSource(LightingEngine& lightingEngine, pl::Vector2f topLeftChunkPos) const
{
    // if (inRocket || !isAlive())
    //     return;
    
    // pl::Vector2f topLeftRelativePos = position - topLeftChunkPos;

    // int lightingTileX = std::floor(topLeftRelativePos.x / (TILE_SIZE_PIXELS_UNSCALED / TILE_LIGHTING_RESOLUTION));
    // int lightingTileY = std::floor(topLeftRelativePos.y / (TILE_SIZE_PIXELS_UNSCALED / TILE_LIGHTING_RESOLUTION));

    // // Add 6 sources around player pos
    // for (int x = 0; x < 2; x++)
    // {
    //     for (int y = -2; y <= 0; y++)
    //     {
    //         lightingEngine.addMovingLightSource(lightingTileX + x, lightingTileY + y, 0.85f);
    //     }
    // }

    // static constexpr float lightScale = 0.7f;
    // static const pl::Color lightColor(255, 220, 140);

    // pl::Vector2f scale((float)ResolutionHandler::getScale() * lightScale, (float)ResolutionHandler::getScale() * lightScale);

    // static const pl::Rect<int> lightMaskRect(0, 0, 256, 256);

    // TextureManager::drawSubTexture(lightTexture, {
    //     TextureType::LightMask, camera.worldToScreenTransform(position), 0, scale, {0.5, 0.5}, lightColor
    //     }, lightMaskRect, sf::BlendAdd);
}

void Player::drawFishingRodCast(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, float gameTime, int worldSize, float waterYOffset) const
{   
    // Draw bob
    pl::Vector2f bobPosition = fishingRodBobWorldPos + pl::Vector2f(0, WorldObject::getWaterBobYOffset(fishingRodBobWorldPos, worldSize, gameTime));
    
    // If fish is biting line, shake bob
    if (fishBitingLine)
    {
        bobPosition.x += (Helper::randInt(0, 2000) - 1000) / 1000.0f;
        bobPosition.y += (Helper::randInt(0, 2000) - 1000) / 1000.0f;
    }
    else
    {
        // Only draw bob if fish not on line
        pl::DrawData bobDrawData;
        bobDrawData.texture = TextureManager::getTexture(TextureType::Tools);
        bobDrawData.shader = Shaders::getShader(ShaderType::Default);
        bobDrawData.position = camera.worldToScreenTransform(bobPosition);
        bobDrawData.scale = pl::Vector2f(ResolutionHandler::getScale(), ResolutionHandler::getScale());
        bobDrawData.centerRatio = pl::Vector2f(0.5, 0.5);
        bobDrawData.textureRect = pl::Rect<int>(0, 112, 16, 16);
        
        spriteBatch.draw(window, bobDrawData);
    }
    
    spriteBatch.endDrawing(window);

    // Draw line
    // TODO: Make line constant thickness (not 1 pixel)
    pl::VertexArray line;
    line.setPrimitiveMode(pl::PrimitiveMode::Lines);

    // Calculate line origin position
    const ToolData& toolData = ToolDataLoader::getToolData(equippedTool);

    pl::Vector2f lineOrigin;
    int lineOffsetXMult = 1;
    if (flippedTexture)
    {
        lineOffsetXMult = -1;
    }

    pl::Vector2f rotatedLineOffset = Helper::rotateVector(toolData.fishingRodLineOffset, 3.14159 * toolRotation / 180.0f);

    lineOrigin.x = position.x + (toolData.holdOffset.x + rotatedLineOffset.x) * lineOffsetXMult;
    lineOrigin.y = position.y + waterYOffset + toolData.holdOffset.y + rotatedLineOffset.y;

    line.addVertex(pl::Vertex(camera.worldToScreenTransform(lineOrigin), pl::Color(255, 255, 255)));
    line.addVertex(pl::Vertex(camera.worldToScreenTransform(bobPosition), pl::Color(255, 255, 255)));

    window.draw(line, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);
}

void Player::drawMeleeSwing(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera) const
{
    pl::Vector2f swingWorldPos = pl::Vector2f(std::cos(meleeSwingAnimationRotation), std::sin(meleeSwingAnimationRotation)) * MELEE_SWING_RADIUS + position;
    swingWorldPos.y += MELEE_SWING_Y_ORIGIN_OFFSET;

    pl::DrawData meleeSwingDrawData;
    meleeSwingDrawData.texture = TextureManager::getTexture(TextureType::Tools);
    meleeSwingDrawData.shader = Shaders::getShader(ShaderType::Default);
    meleeSwingDrawData.centerRatio = pl::Vector2f(0.5f, 0.5f);
    meleeSwingDrawData.position = camera.worldToScreenTransform(swingWorldPos);
    meleeSwingDrawData.rotation = meleeSwingAnimationRotation / M_PI * 180.0f;
    meleeSwingDrawData.scale = pl::Vector2f((float)ResolutionHandler::getScale(), -(float)ResolutionHandler::getScale());
    meleeSwingDrawData.textureRect = meleeSwingAnimation.getTextureRect();

    if (flippedTexture)
    {
        meleeSwingDrawData.scale.y *= -1;
    }

    spriteBatch.draw(window, meleeSwingDrawData);
}

void Player::drawArmour(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, float waterYOffset) const
{
    float scale = ResolutionHandler::getScale();

    int xScaleMult = 1;
    if (flippedTexture)
    {
        xScaleMult = -1;
    }

    pl::Vector2f armourOrigin = position - pl::Vector2f(8 * xScaleMult, 0);

    pl::DrawData armourDrawData;
    armourDrawData.texture = TextureManager::getTexture(TextureType::Tools);
    armourDrawData.shader = Shaders::getShader(ShaderType::Default);
    armourDrawData.scale = pl::Vector2f(scale * xScaleMult, scale * playerYScaleMult);
    armourDrawData.useCentreAbsolute = true;

    if (damageCooldownTimer > 0.0f)
    {
        armourDrawData.shader = Shaders::getShader(ShaderType::Flash);
        armourDrawData.shader->setUniform1f("flash_amount", damageCooldownTimer / MAX_DAMAGE_COOLDOWN_TIMER);
    }

    // Draw headpiece, chest, and boots
    for (int i = 2; i >= 0; i--)
    {
        ArmourType armourType = armour[i];

        if (armourType < 0)
        {
            continue;
        }

        // Get data
        const ArmourData& armourData = ArmourDataLoader::getArmourData(armourType);

        // Draw armour piece
        int frame = idleAnimation.getFrame();
        if (direction.x != 0 || direction.y != 0)
        {
            frame = idleAnimation.getFrameCount() + runAnimation.getFrame();
        }

        const pl::Rect<int>& armourTextureRect = armourData.wearTextures[frame];

        armourDrawData.position = camera.worldToScreenTransform(armourOrigin + pl::Vector2f(armourData.wearTextureOffset.x * xScaleMult, waterYOffset));
        armourDrawData.centerRatio = pl::Vector2f(0, armourTextureRect.height - armourData.wearTextureOffset.y);
        armourDrawData.textureRect = armourTextureRect;

        spriteBatch.draw(window, armourDrawData);
    }
}

void Player::setTool(ToolType toolType)
{
    equippedTool = toolType;

    toolRotation = 0.0f;

    reelInFishingRod();

    if (usingTool)
    {
        toolTweener.stopTween(rotationTweenID);
    }
}

ToolType Player::getTool()
{
    return equippedTool;
}

void Player::useTool(ProjectileManager& projectileManager, InventoryData& inventory, pl::Vector2f mouseWorldPos, Game& game)
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
            ProjectileType projectileType = inventory.getValidProjectileNearestToEnd(equippedTool);
            if (projectileType < 0)
            {
                return;
            }

            // Take projectile from inventory
            const ProjectileData& projectileData = ToolDataLoader::getProjectileData(projectileType);
            inventory.takeItem(projectileData.itemType, 1);

            // Calculate projectile position and angle
            float angle = 180.0f * std::atan2(mouseWorldPos.y - position.y, mouseWorldPos.x - position.x) / M_PI;

            pl::Vector2f spawnPos = static_cast<pl::Vector2f>(toolData.holdOffset) + Helper::rotateVector(static_cast<pl::Vector2f>(toolData.shootOffset), toolRotation);

            if (flippedTexture)
            {
                spawnPos.x *= -1;
            }

            spawnPos += position;

            // Create projectile
            Projectile projectile(spawnPos, angle, projectileType,
                toolData.projectileDamageMult, toolData.shootPower);

            // Add projectile to manager
            projectileManager.addProjectile(projectile);
            break;
        }
        case ToolBehaviourType::MeleeWeapon:
        {
            usingTool = true;

            static constexpr float destRotation = 90.0f;
            static constexpr float tweenTime = 0.1f;

            rotationTweenID = toolTweener.startTween(&toolRotation, 0.0f, destRotation, tweenTime, TweenTransition::Circ, TweenEasing::EaseInOut);
            toolTweener.addTweenToQueue(rotationTweenID, &toolRotation, destRotation, 0.0f, 0.15, TweenTransition::Expo, TweenEasing::EaseOut);

            game.testMeleeCollision(meleeHitRects);

            meleeSwingAnimation.setFrame(0);
            break;
        }
    }

    playerYScaleMult = 0.8f;
    idleAnimation.setFrame(0);
}

bool Player::isUsingTool()
{
    return usingTool;
}

void Player::startUseToolTimer()
{
    useToolCooldown = MAX_USE_TOOL_COOLDOWN;
}

bool Player::isUseToolTimerFinished()
{
    return (useToolCooldown <= 0.0f);
}

void Player::setArmourFromInventory(const InventoryData& armourInventory)
{
    for (int i = 0; i < std::min(3, armourInventory.getSize()); i++)
    {
        const std::optional<ItemCount>& armourItemSlot = armourInventory.getData().at(i);
        if (armourItemSlot.has_value())
        {
            const ItemData& itemData = ItemDataLoader::getItemData(armourItemSlot->first);
            armour[i] = itemData.armourType;
        }
        else
        {
            armour[i] = -1;
        }
    }
}

void Player::setCanMove(bool value)
{
    canMove = value;
    direction = pl::Vector2f(0, 0);
}

bool Player::testHitCollision(const Projectile& projectile)
{
    HitCircle hitCircle(projectile.getCollisionCircle());
    hitCircle.damage = projectile.getDamage();
    return testHitCollision(hitCircle);
}

bool Player::testHitCollision(const HitRect& hitRect)
{
    if (!isAlive())
    {
        return false;
    }

    if (!collisionRect.isColliding(hitRect))
    {
        return false;
    }

    return takeDamage(hitRect.damage);
}

bool Player::testHitCollision(const HitCircle& hitCircle)
{
    if (!isAlive())
    {
        return false;
    }

    if (!collisionRect.isColliding(hitCircle))
    {
        return false;
    }

    return takeDamage(hitCircle.damage);
}

bool Player::takeDamage(float rawAmount)
{
    // Calculate defence to modify damage
    if (damageCooldownTimer <= 0)
    {
        int defence = PlayerStats::calculateDefence(armour);

        int damageAmount = std::max(std::round(rawAmount * (1.0f - defence / 70.0f)), 0.0f);
        
        health -= damageAmount;

        HitMarkers::addHitMarker(position, damageAmount, pl::Color(208, 15, 30));

        healthRegenCooldownTimer = MAX_HEALTH_REGEN_COOLDOWN_TIMER;
        damageCooldownTimer = MAX_DAMAGE_COOLDOWN_TIMER;
    }

    if (!isAlive())
    {
        respawnTimer = MAX_RESPAWN_TIMER;
    }

    return true;
}

bool Player::useConsumable(const ConsumableData& consumable)
{
    bool usedConsumable = false;

    if (consumable.healthIncrease > 0 && health < maxHealth && healthConsumableTimer <= 0)
    {
        int healthIncrease = std::min(static_cast<float>(consumable.healthIncrease), maxHealth - health);
        health += healthIncrease;;
        HitMarkers::addHitMarker(position, healthIncrease, pl::Color(30, 188, 28));

        healthConsumableTimerMax = consumable.cooldownTime;
        healthConsumableTimer = healthConsumableTimerMax;

        usedConsumable = true;
    }

    if (consumable.permanentHealthIncrease > 0)
    {
        maxHealth += consumable.permanentHealthIncrease;
        HitMarkers::addHitMarker(position, consumable.permanentHealthIncrease, pl::Color(35, 144, 99));

        usedConsumable = true;
    }

    return usedConsumable;
}

void Player::swingFishingRod(pl::Vector2f mouseWorldPos, pl::Vector2<int> selectedWorldTile)
{
    swingingFishingRod = true;
    fishingRodCasted = false;

    fishingRodBobWorldPos.x = (std::floor(mouseWorldPos.x / TILE_SIZE_PIXELS_UNSCALED) + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
    fishingRodBobWorldPos.y = (std::floor(mouseWorldPos.y / TILE_SIZE_PIXELS_UNSCALED) + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

    fishingRodBobWorldTile = selectedWorldTile;
}

pl::Vector2<int> Player::reelInFishingRod()
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

bool Player::canReachPosition(pl::Vector2f worldPos)
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

void Player::enterRocket(pl::Vector2f positionOverride)
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

void Player::setPosition(pl::Vector2f worldPos)
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

bool Player::isAlive() const
{
    return (health > 0);
}

// Multiplayer

PacketDataPlayerCharacterInfo Player::getNetworkPlayerInfo(const Camera* camera, uint64_t steamID)
{
    PacketDataPlayerCharacterInfo info;
    info.position = position;
    info.direction = direction;
    info.speed = speed;

    if (direction == pl::Vector2f(0, 0))
    {
        info.animationFrame = idleAnimation.getFrame();
        info.animationFrameTick = idleAnimation.getFrameTick();
    }
    else
    {
        info.animationFrame = runAnimation.getFrame();
        info.animationFrameTick = runAnimation.getFrameTick();
    }
    
    info.flipped = flippedTexture;
    info.yScaleMult = playerYScaleMult;

    info.onWater = onWater;

    info.inRocket = inRocket;

    info.toolType = equippedTool;
    // info.toolRotation = toolRotation;
    info.fishingRodCasted = fishingRodCasted;
    info.fishBitingLine = fishBitingLine;
    info.fishingRodBobWorldTile = fishingRodBobWorldTile;

    info.usingTool = usingTool;

    if (usingTool)
    {
        info.toolRotTweenID = rotationTweenID;
        info.toolTweenData = toolTweener.getTweenData(rotationTweenID);
    }

    info.armour = armour;

    if (camera)
    {
        info.chunkViewRange = camera->getChunkViewRange();
    }

    info.userID = steamID;

    return info;
}