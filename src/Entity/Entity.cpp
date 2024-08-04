#include "Entity/Entity.hpp"

Entity::Entity(sf::Vector2f position, unsigned int entityType)
    : WorldObject(position)
{
    this->entityType = entityType;

    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);

    health = entityData.health;

    drawLayer = 0;
}

void Entity::draw(sf::RenderWindow& window, float dt, const sf::Color& color)
{
    const EntityData& entityData = EntityDataLoader::getEntityData(entityType);

    float scale = ResolutionHandler::getScale();

    TextureManager::drawSubTexture(window, {TextureType::Entities, position + Camera::getIntegerDrawOffset(), 0, 
        {scale, scale}, entityData.textureOrigin, color}, entityData.textureRect);
}