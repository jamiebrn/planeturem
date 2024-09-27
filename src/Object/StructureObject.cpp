#include "Object/StructureObject.hpp"

StructureObject::StructureObject(sf::Vector2f position, StructureType structureType)
    : WorldObject(position)
{
    this->structureType = structureType;
}

void StructureObject::createWarpRect(sf::Vector2f rectPosition)
{
    if (warpEntranceRect.has_value())
        return;
    
    warpEntranceRect = CollisionRect();
    warpEntranceRect->x = rectPosition.x;
    warpEntranceRect->y = rectPosition.y;
    warpEntranceRect->width = TILE_SIZE_PIXELS_UNSCALED;
    warpEntranceRect->height = TILE_SIZE_PIXELS_UNSCALED;
}

bool StructureObject::isPlayerInEntrance(sf::Vector2f playerPos, StructureEnterEvent& enterEvent)
{
    if (!warpEntranceRect.has_value())
        return false;

    if (warpEntranceRect->isPointInRect(playerPos.x, playerPos.y))
    {
        enterEvent.enteredStructure = this;
        enterEvent.entrancePosition = sf::Vector2f(warpEntranceRect->x, warpEntranceRect->y);

        return true;
    }

    return false;
}

void StructureObject::setWorldPosition(sf::Vector2f newPosition)
{
    sf::Vector2f entranceRelativePos = sf::Vector2f(warpEntranceRect->x, warpEntranceRect->y) - position;

    warpEntranceRect->x = newPosition.x + entranceRelativePos.x;
    warpEntranceRect->y = newPosition.y + entranceRelativePos.y;

    position = newPosition;
}

void StructureObject::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) const
{
    const StructureData& structureData = StructureDataLoader::getStructureData(structureType);

    TextureDrawData textureDrawData;
    textureDrawData.type = TextureType::Objects;
    textureDrawData.position = Camera::worldToScreenTransform(position);
    textureDrawData.scale = sf::Vector2f(ResolutionHandler::getScale(), ResolutionHandler::getScale());
    textureDrawData.centerRatio = structureData.textureOrigin;
    
    spriteBatch.draw(window, textureDrawData, structureData.textureRect);
}