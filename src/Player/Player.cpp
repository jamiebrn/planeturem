#include "Player/Player.hpp"

Player::Player(sf::Vector2f position)
    : WorldObject(position)
{
    collisionRect.width = 16.0f;
    collisionRect.height = 16.0f;

    collisionRect.x = position.x - collisionRect.width / 2.0f;
    collisionRect.y = position.y - collisionRect.height / 2.0f;

    drawLayer = 0;
}

void Player::update(float dt)
{
    // Handle movement input
    sf::Vector2f direction;
    direction.x = sf::Keyboard::isKeyPressed(sf::Keyboard::D) - sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    direction.y = sf::Keyboard::isKeyPressed(sf::Keyboard::S) - sf::Keyboard::isKeyPressed(sf::Keyboard::W);

    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0)
        direction /= length;
    
    collisionRect.x += direction.x * 300.0f * dt;
    collisionRect.y += direction.y * 300.0f * dt;

    // Handle collision with world (tiles, object)
    std::vector<std::unique_ptr<CollisionRect>> worldCollisionRects = ChunkManager::getChunkCollisionRects();
    for (const std::unique_ptr<CollisionRect>& worldCollisionRect : worldCollisionRects)
    {
        collisionRect.handleCollision(*worldCollisionRect);
    }

    // Update position using collision rect after collision has been handled
    position.x = collisionRect.x + collisionRect.width / 2.0f;
    position.y = collisionRect.y + collisionRect.height / 2.0f;
}

void Player::draw(sf::RenderWindow& window, float dt, const sf::Color& color)
{
    TextureManager::drawTexture(window, {TextureType::Player, position + Camera::getIntegerDrawOffset(), 0, 3, {0.5, 0.7}});
}