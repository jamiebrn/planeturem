#include "World/Room.hpp"

Room::Room(const RoomData& roomData)
{
    this->roomData = roomData;

    createCollisionRects();
}

bool Room::handleStaticCollisionX(CollisionRect& collisionRect, float dx) const
{
    bool collision = false;
    for (const CollisionRect& rect : collisionRects)
    {
        if (collisionRect.handleStaticCollisionX(rect, dx))
            collision = true;
    }
    return collision;
}

bool Room::handleStaticCollisionY(CollisionRect& collisionRect, float dy) const
{
    bool collision = false;
    for (const CollisionRect& rect : collisionRects)
    {
        if (collisionRect.handleStaticCollisionY(rect, dy))
            collision = true;
    }
    return collision;
}

void Room::createCollisionRects()
{
    const sf::Image& bitmaskImage = TextureManager::getBitmask(BitmaskType::Structures);

    for (int x = 0; x < roomData.tileSize.x; x++)
    {
        for (int y = 0; y < roomData.tileSize.y; y++)
        {
            // Sample bitmask
            sf::Color bitmaskColor = bitmaskImage.getPixel(roomData.collisionBitmaskOffset.x + x, roomData.collisionBitmaskOffset.y + y);

            if (bitmaskColor == sf::Color(0, 0, 0, 0))
                continue;

            CollisionRect collisionRect;
            collisionRect.x = x * TILE_SIZE_PIXELS_UNSCALED;
            collisionRect.y = y * TILE_SIZE_PIXELS_UNSCALED;
            collisionRect.height = TILE_SIZE_PIXELS_UNSCALED;
            collisionRect.width = TILE_SIZE_PIXELS_UNSCALED;

            // Collision
            if (bitmaskColor == sf::Color(255, 0, 0))
            {
                collisionRects.push_back(collisionRect);
            }

            // Warp
            if (bitmaskColor == sf::Color(0, 255, 0))
            {
                warpExitRect = collisionRect;
            }
        }
    }
}

bool Room::isPlayerInExit(sf::Vector2f playerPos) const
{
    return (warpExitRect.isPointInRect(playerPos.x, playerPos.y));
}

sf::Vector2f Room::getEntrancePosition() const
{
    sf::Vector2f entrancePos;
    entrancePos.x = warpExitRect.x + 0.5f * TILE_SIZE_PIXELS_UNSCALED;
    entrancePos.y = warpExitRect.y - 0.5f * TILE_SIZE_PIXELS_UNSCALED;

    return entrancePos;
}

void Room::draw(sf::RenderTarget& window) const
{
    if (DebugOptions::drawCollisionRects)
    {
        for (const CollisionRect& rect : collisionRects)
        {
            rect.debugDraw(window);
        }
    }

    float scale = ResolutionHandler::getScale();

    TextureDrawData drawData;
    drawData.position = Camera::worldToScreenTransform(sf::Vector2f(0, 0));
    drawData.scale = sf::Vector2f(scale, scale);
    drawData.type = TextureType::Rooms;

    TextureManager::drawSubTexture(window, drawData, roomData.textureRect);
}