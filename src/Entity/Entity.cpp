#include "Entity/Entity.hpp"

Entity::Entity(sf::Vector2f position, unsigned int entityType)
    : WorldObject(position)
{
    this->entityType = entityType;

    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);

    health = entityData.health;

    float velocityAngle = rand() % 360;
    velocity.x = std::cos(velocityAngle * 2 * M_PI / 180) * 70.0f;
    velocity.y = std::sin(velocityAngle * 2 * M_PI / 180) * 70.0f;

    collisionRect.width = 16.0f;
    collisionRect.height = 16.0f;

    collisionRect.x = position.x - collisionRect.width / 2.0f;
    collisionRect.y = position.y - collisionRect.height / 2.0f;

    drawLayer = 0;
}

void Entity::update(float dt, std::vector<CollisionRect*>& worldCollisionRects)
{
    // Handle collision with world (tiles, object)

    // Test collision after x movement
    collisionRect.x += velocity.x * dt;
    for (CollisionRect* worldCollisionRect : worldCollisionRects)
    {
        collisionRect.handleStaticCollisionX(*worldCollisionRect, velocity.x);
    }

    // Test collision after y movement
    collisionRect.y += velocity.y * dt;
    for (CollisionRect* worldCollisionRect : worldCollisionRects)
    {
        collisionRect.handleStaticCollisionY(*worldCollisionRect, velocity.y);
    }

    // Update position using collision rect after collision has been handled
    position.x = collisionRect.x + collisionRect.width / 2.0f;
    position.y = collisionRect.y + collisionRect.height / 2.0f;
}

void Entity::draw(sf::RenderWindow& window, float dt, const sf::Color& color)
{
    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);

    sf::Vector2f scale(ResolutionHandler::getScale(), ResolutionHandler::getScale());

    if (velocity.x < 0)
        scale.x *= -1;

    TextureManager::drawSubTexture(window, {TextureType::Entities, position + Camera::getIntegerDrawOffset(), 0, 
        scale, entityData.textureOrigin, color}, entityData.textureRect);
}