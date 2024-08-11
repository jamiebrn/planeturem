#include "Player/Player.hpp"

const sf::Vector2f Player::toolOffset = {-2, -3};

Player::Player(sf::Vector2f position)
    : WorldObject(position)
{
    collisionRect.width = 16.0f;
    collisionRect.height = 16.0f;

    collisionRect.x = position.x - collisionRect.width / 2.0f;
    collisionRect.y = position.y - collisionRect.height / 2.0f;

    drawLayer = 0;

    flippedTexture = false;

    idleAnimation.create(1, 16, 18, 0, 0, 0);
    runAnimation.create(5, 16, 18, 48, 0, 0.1);

    equippedTool = 0;
    toolRotation = 0;
    usingTool = false;
}

void Player::update(float dt, sf::Vector2f mouseWorldPos, ChunkManager& chunkManager)
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

    // std::cout << position.x << ", " << position.y << std::endl;
}

void Player::draw(sf::RenderTarget& window, float dt, const sf::Color& color)
{
    sf::Vector2f scale((float)ResolutionHandler::getScale(), (float)ResolutionHandler::getScale());

    TextureManager::drawTexture(window, {TextureType::Shadow, Camera::worldToScreenTransform(position), 0, scale, {0.5, 0.85}});

    if (flippedTexture)
        scale.x *= -1;
    
    sf::IntRect animationRect;
    if (direction.x == 0 && direction.y == 0)
        animationRect = idleAnimation.getTextureRect();
    else
        animationRect = runAnimation.getTextureRect();

    TextureManager::drawSubTexture(window, {TextureType::Player, Camera::worldToScreenTransform(position), 0, scale, {0.5, 1}}, animationRect);

    // Draw equipped tool
    const ToolData& toolData = ToolDataLoader::getToolData(equippedTool);

    sf::Vector2f toolPos = Camera::worldToScreenTransform(position) + sf::Vector2f(scale.x * toolOffset.x, scale.y * toolOffset.y);

    float pivotYOffset = (toolRotation / 90.0f) * 0.4;

    float correctedToolRotation = toolRotation;
    if (flippedTexture)
        correctedToolRotation = -toolRotation;

    TextureManager::drawSubTexture(window, {TextureType::Tools, toolPos, correctedToolRotation, scale, {toolData.pivot.x, toolData.pivot.y + pivotYOffset}}, toolData.textureRect);

    // DEBUG
    // collisionRect.debugDraw(window);
}

void Player::drawLightMask(sf::RenderTarget& lightTexture)
{
    sf::Vector2f scale((float)ResolutionHandler::getScale(), (float)ResolutionHandler::getScale());

    sf::IntRect lightMaskRect(0, 0, 80, 80);

    TextureManager::drawSubTexture(lightTexture, {TextureType::LightMask, Camera::worldToScreenTransform(position), 0, scale, {0.5, 0.5}}, lightMaskRect);
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
    float distance = std::sqrt(std::pow(worldPos.x - position.x, 2.0) + std::pow(worldPos.y - position.y, 2.0));
    float tileDistance = distance / TILE_SIZE_PIXELS_UNSCALED;
    return tileDistance <= tileReach;
}