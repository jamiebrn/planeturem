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

    collisionRect.width = 48.0f;
    collisionRect.height = 48.0f;

    collisionRect.x = position.x - collisionRect.width / 2.0f;
    collisionRect.y = position.y - collisionRect.height / 2.0f;

    drawLayer = 0;
}

void Entity::update(float dt, ChunkManager& chunkManager)
{
    // Handle collision with world (tiles, object)

    // Test collision after x movement
    collisionRect.x += velocity.x * dt;
    if (chunkManager.collisionRectChunkStaticCollisionX(collisionRect, velocity.x))
        velocity.x *= -1;

    // Test collision after y movement
    collisionRect.y += velocity.y * dt;
    if (chunkManager.collisionRectChunkStaticCollisionY(collisionRect, velocity.y))
        velocity.y *= -1;

    // Update position using collision rect after collision has been handled
    position.x = collisionRect.x + collisionRect.width / 2.0f;
    position.y = collisionRect.y + collisionRect.height / 2.0f;
}

void Entity::draw(sf::RenderWindow& window, float dt, const sf::Color& color)
{
    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);

    sf::Vector2f scale(ResolutionHandler::getScale(), ResolutionHandler::getScale());

    // Draw shadow
    TextureManager::drawTexture(window, {TextureType::Shadow, position + Camera::getIntegerDrawOffset(), 0, scale, {0.5, 0.85}});

    if (velocity.x < 0)
        scale.x *= -1;

    TextureManager::drawSubTexture(window, {TextureType::Entities, position + Camera::getIntegerDrawOffset(), 0, 
        scale, entityData.textureOrigin, color}, entityData.textureRect);

    // DEBUG
    collisionRect.debugDraw(window);
}

bool Entity::isSelectedWithCursor(sf::Vector2f cursorWorldPos)
{
    return collisionRect.isPointInRect(cursorWorldPos.x, cursorWorldPos.y);
}

unsigned int Entity::getEntityType()
{
    return entityType;
}

sf::Vector2f Entity::getSize()
{
    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);

    return entityData.size * ResolutionHandler::getTileSize();
}