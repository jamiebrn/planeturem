#include "Player/Player.hpp"

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
}

void Player::update(float dt, ChunkManager& chunkManager)
{
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

    float speed = 120.0f * ResolutionHandler::getScale();


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

    // std::cout << position.x << ", " << position.y << std::endl;
}

void Player::draw(sf::RenderWindow& window, float dt, const sf::Color& color)
{
    sf::Vector2f scale((float)ResolutionHandler::getScale(), (float)ResolutionHandler::getScale());

    if (flippedTexture)
        scale.x *= -1;
    
    sf::IntRect animationRect;
    if (direction.x == 0 && direction.y == 0)
        animationRect = idleAnimation.getTextureRect();
    else
        animationRect = runAnimation.getTextureRect();

    TextureManager::drawSubTexture(window, {TextureType::Player, position + Camera::getIntegerDrawOffset(), 0, scale, {0.5, 1}}, animationRect);
}