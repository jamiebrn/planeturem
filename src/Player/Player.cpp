#include "Player/Player.hpp"

const sf::Vector2f Player::toolOffset = {-2, -3};

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
}

void Player::update(float dt, sf::Vector2f mouseWorldPos, ChunkManager& chunkManager, int worldSize, bool& wrappedAroundWorld, sf::Vector2f& wrapPositionDelta)
{
    // Face towards mouse cursor (overridden if moving)
    flippedTexture = (position.x - mouseWorldPos.x) > 0;

    // Handle movement input
    direction.x = sf::Keyboard::isKeyPressed(sf::Keyboard::D) - sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    direction.y = sf::Keyboard::isKeyPressed(sf::Keyboard::S) - sf::Keyboard::isKeyPressed(sf::Keyboard::W);

    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0)
    {
        direction /= length;

        if (direction.x != 0)
            flippedTexture = direction.x < 0;
    }

    float speed = 120.0f;


    // Handle collision with world (tiles, object)

    // Test collision after x movement
    collisionRect.x += direction.x * speed * dt;
    chunkManager.collisionRectChunkStaticCollisionX(collisionRect, direction.x);

    // Test collision after y movement
    collisionRect.y += direction.y * speed * dt;
    chunkManager.collisionRectChunkStaticCollisionY(collisionRect, direction.y);

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


    // Update animation
    idleAnimation.update(dt);
    runAnimation.update(dt);

    toolTweener.update(dt);

    if (swingingTool)
    {
        if (toolTweener.isTweenFinished(rotationTweenID))
        {
            rotationTweenID = toolTweener.startTween(&toolRotation, 90.0f, 0.0f, 0.15, TweenTransition::Expo, TweenEasing::EaseOut);
            swingingTool = false;
        }
    }
    else if (usingTool)
    {
        if (toolTweener.isTweenFinished(rotationTweenID)) usingTool = false;
    }

    // Update on water
    onWater = (chunkManager.getChunkTileType(getChunkInside(worldSize), getChunkTileInside(worldSize)) == 0);

    // Update prompts
    for (auto prompt = prompts.begin(); prompt != prompts.end();)
    {
        prompt->second -= dt;
        if (prompt->second <= 0)
        {
            prompt = prompts.erase(prompt);
            continue;
        }
        prompt++;
    }

    // std::cout << position.x << ", " << position.y << std::endl;
}

void Player::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color)
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
            sf::Vector2f(playerScale.x * toolOffset.x, playerScale.y * toolOffset.y));

        float pivotYOffset = (toolRotation / 90.0f) * 0.4;

        float correctedToolRotation = toolRotation;
        if (flippedTexture)
            correctedToolRotation = -toolRotation;

        TextureManager::drawSubTexture(window, {
            TextureType::Tools, toolPos, correctedToolRotation, playerScale, {toolData.pivot.x, toolData.pivot.y + pivotYOffset
            }}, toolData.textureRect);
    }

    float scale = ResolutionHandler::getScale();

    // Draw prompts
    for (int i = prompts.size() - 1; i >= 0; i--)
    {
        auto& prompt = prompts[i];
        int yOffset = (prompts.size() - 1 - i) * 12.0f * scale;
        TextDraw::drawText(window, {
            prompt.first,
            Camera::worldToScreenTransform(position) + sf::Vector2f(0, -10.0f * scale - yOffset),
            sf::Color(255, 255, 255),
            static_cast<unsigned int>(8.0f * scale),
            sf::Color(0, 0, 0),
            0,
            true,
            true
            });
    }

    // DEBUG

    #if DEBUG_DRAW
    collisionRect.debugDraw(window);
    #endif
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

void Player::setTool(ToolType toolType)
{
    equippedTool = toolType;
}

ToolType Player::getTool()
{
    return equippedTool;
}

void Player::useTool()
{
    usingTool = true;
    swingingTool = true;
    rotationTweenID = toolTweener.startTween(&toolRotation, toolRotation, 90.0f, 0.1, TweenTransition::Circ, TweenEasing::EaseInOut);
}

bool Player::isUsingTool()
{
    return usingTool;
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

void Player::createPrompt(std::string text)
{
    prompts.push_back({text, 5.0f});
}