#include "Object/StructureObject.hpp"

StructureObject::StructureObject(sf::Vector2f position, StructureType structureType)
    : WorldObject(position)
{
    this->structureType = structureType;
}

void StructureObject::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color)
{
    const StructureData& structureData = StructureDataLoader::getStructureData(structureType);

    TextureDrawData textureDrawData;
    textureDrawData.type = TextureType::Objects;
    textureDrawData.position = Camera::worldToScreenTransform(position);
    textureDrawData.scale = sf::Vector2f(ResolutionHandler::getScale(), ResolutionHandler::getScale());
    textureDrawData.centerRatio = structureData.textureOrigin;
    
    spriteBatch.draw(window, textureDrawData, structureData.textureRect);
}