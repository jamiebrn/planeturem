#include "Object/StructureObject.hpp"

StructureObject::StructureObject(sf::Vector2f position, StructureType structureType)
    : WorldObject(position)
{
    this->structureType = structureType;

    // Create entrance warp rect from bitmask
    // const sf::Image& bitmaskImage = TextureManager::getBitmask(BitmaskType::Structures);

    // const StructureData& structureData = StructureDataLoader::getStructureData(structureType);

    // bool createdRect = false;

    // for (int x = 0; x < structureData.size.x; x++)
    // {
    //     for (int y = 0; y < structureData.size.y; y++)
    //     {
    //         sf::Color bitmaskColor = bitmaskImage.getPixel(structureData.collisionBitmaskOffset.x + x, structureData.collisionBitmaskOffset.y + y);

    //         // If green, make warp rect at position
    //         if (bitmaskColor == sf::Color(0, 255, 0))
    //         {
    //             warpEntranceRect = CollisionRect();
    //             warpEntranceRect->x = this->position.x + x * TILE_SIZE_PIXELS_UNSCALED;
    //             warpEntranceRect->y = this->position.y + y * TILE_SIZE_PIXELS_UNSCALED;
    //             warpEntranceRect->width = TILE_SIZE_PIXELS_UNSCALED;
    //             warpEntranceRect->height = TILE_SIZE_PIXELS_UNSCALED;

    //             std::cout << "created warp rect\n";

    //             createdRect = true;
    //             break;
    //         }
    //     }

    //     if (createdRect)
    //         break;
    // }
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

bool StructureObject::isPlayerInEntrance(sf::Vector2f playerPos)
{
    if (!warpEntranceRect.has_value())
        return false;

    return (warpEntranceRect->isPointInRect(playerPos.x, playerPos.y));
}

void StructureObject::setWorldPosition(sf::Vector2f newPosition)
{
    sf::Vector2f entranceRelativePos = sf::Vector2f(warpEntranceRect->x, warpEntranceRect->y) - position;

    warpEntranceRect->x = newPosition.x + entranceRelativePos.x;
    warpEntranceRect->y = newPosition.y + entranceRelativePos.y;

    position = newPosition;
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