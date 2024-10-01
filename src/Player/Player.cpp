#include "Player/Player.hpp"

Player::Player(sf::Vector2f position)
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

    equippedTool = -1;
    toolRotation = 0;
    usingTool = false;

    fishingRodCasted = false;
    swingingFishingRod = false;
    fishBitingLine = false;
}

void Player::update(float dt, sf::Vector2f mouseWorldPos, ChunkManager& chunkManager, int worldSize, bool& wrappedAroundWorld, sf::Vector2f& wrapPositionDelta)
{
    updateDirection(mouseWorldPos);
    updateAnimation(dt);

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

    // Wrap position around world
    float worldPixelSize = worldSize * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    if (collisionRect.x >= worldPixelSize) 
    {
        collisionRect.x -= worldPixelSize;
        wrapPositionDelta.x = -worldPixelSize;
        wrappedAroundWorld = true;
    }
    else if (collisionRect.x < 0)
    {
        collisionRect.x += worldPixelSize;
        wrapPositionDelta.x = worldPixelSize;
        wrappedAroundWorld = true;
    }
    if (collisionRect.y >= worldPixelSize)
    {
        collisionRect.y -= worldPixelSize;
        wrapPositionDelta.y = -worldPixelSize;
        wrappedAroundWorld = true;
    }
    else if (collisionRect.y < 0)
    {
        collisionRect.y += worldPixelSize;
        wrapPositionDelta.y = worldPixelSize;
        wrappedAroundWorld = true;
    }
    
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

void Player::setTool(ToolType toolType)
{
    equippedTool = toolType;

    fishingRodCasted = false;
}

ToolType Player::getTool()
{
    return equippedTool;
}

void Player::useTool()
{
    usingTool = true;
    // swingingTool = true;

    const ToolData& toolData = ToolDataLoader::getToolData(equippedTool);

    // Different tween values based on tool behaviour
    float destRotation = 0.0f;
    float tweenTime = 0.0f;
    switch (toolData.toolBehaviourType)
    {
        case ToolBehaviourType::Pickaxe:
            destRotation = 90.0f;
            tweenTime = 0.1f;
            break;
        case ToolBehaviourType::FishingRod:
            destRotation = -80.0f;
            tweenTime = 0.1f;
            break;
    }

    rotationTweenID = toolTweener.startTween(&toolRotation, toolRotation, destRotation, tweenTime, TweenTransition::Circ, TweenEasing::EaseInOut);
    toolTweener.addTweenToQueue(rotationTweenID, &toolRotation, destRotation, 0.0f, 0.15, TweenTransition::Expo, TweenEasing::EaseOut);
}

bool Player::isUsingTool()
{
    return usingTool;
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
    return tileDistance <= tileReach;
}

void Player::enterStructure()
{
    onWater = false;
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