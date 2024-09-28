#include "World/Room.hpp"

Room::Room(const RoomData& roomData)
{
    this->roomData = roomData;

    createObjects();

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

void Room::createObjects()
{
    const sf::Image& bitmaskImage = TextureManager::getBitmask(BitmaskType::Structures);

    for (int y = 0; y < roomData.tileSize.y; y++)
    {
        objectGrid.push_back(std::vector<std::optional<BuildableObject>>());
        for (int x = 0; x < roomData.tileSize.x; x++)
        {
            std::optional<BuildableObject> object = std::nullopt;

            // Sample bitmask
            sf::Color bitmaskColor = bitmaskImage.getPixel(roomData.collisionBitmaskOffset.x + x, roomData.collisionBitmaskOffset.y + y);

            // Create object
            if (roomData.objectsInRoom.contains(bitmaskColor.b))
            {
                ObjectType objectTypeToSpawn = roomData.objectsInRoom.at(bitmaskColor.b);
                object = BuildableObject(sf::Vector2f((x + 0.5f) * TILE_SIZE_PIXELS_UNSCALED, (y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED), objectTypeToSpawn);
            }

            // Add to array
            objectGrid.back().push_back(object);
        }
    }
}

void Room::createCollisionRects()
{
    const sf::Image& bitmaskImage = TextureManager::getBitmask(BitmaskType::Structures);

    for (int x = 0; x < roomData.tileSize.x; x++)
    {
        for (int y = 0; y < roomData.tileSize.y; y++)
        {
            CollisionRect collisionRect;
            collisionRect.x = x * TILE_SIZE_PIXELS_UNSCALED;
            collisionRect.y = y * TILE_SIZE_PIXELS_UNSCALED;
            collisionRect.height = TILE_SIZE_PIXELS_UNSCALED;
            collisionRect.width = TILE_SIZE_PIXELS_UNSCALED;

            bool collisionCreated = false;

            // Check object first
            if (objectGrid[y][x].has_value())
            {
                BuildableObject& object = objectGrid[y][x].value();
                
                const ObjectData& objectData = ObjectDataLoader::getObjectData(object.getObjectType());
                if (objectData.hasCollision)
                {
                    collisionRects.push_back(collisionRect);
                    collisionCreated = true;
                }
            }

            // Sample bitmask
            sf::Color bitmaskColor = bitmaskImage.getPixel(roomData.collisionBitmaskOffset.x + x, roomData.collisionBitmaskOffset.y + y);

            if (bitmaskColor == sf::Color(0, 0, 0, 0))
                continue;

            // Collision
            if (bitmaskColor == sf::Color(255, 0, 0) && !collisionCreated)
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

void Room::updateObjects(float dt)
{
    for (int y = 0; y < objectGrid.size(); y++)
    {
        for (int x = 0; x < objectGrid[y].size(); x++)
        {
            std::optional<BuildableObject>& objectOptional = objectGrid[y][x];

            if (!objectOptional.has_value())
                continue;

            objectOptional->update(dt, false);
        }
    }
}

BuildableObject* Room::getObject(sf::Vector2f mouseWorldPos)
{
    sf::Vector2i selectedTile = getSelectedTile(mouseWorldPos);

    return getObject(selectedTile);
}

BuildableObject* Room::getObject(sf::Vector2i tile)
{
    // Bounds checking
    if (tile.x < 0 || tile.x >= objectGrid[0].size())
        return nullptr;
    
    if (tile.y < 0 || tile.y >= objectGrid.size())
        return nullptr;
    
    std::optional<BuildableObject>& objectOptional = objectGrid[tile.y][tile.x];

    if (objectOptional.has_value())
    {
        return &objectOptional.value();
    }

    // Default case
    return nullptr;
}

sf::Vector2i Room::getSelectedTile(sf::Vector2f mouseWorldPos)
{
    sf::Vector2i selectedTile;
    selectedTile.x = std::floor(mouseWorldPos.x / TILE_SIZE_PIXELS_UNSCALED);
    selectedTile.y = std::floor(mouseWorldPos.y / TILE_SIZE_PIXELS_UNSCALED);
    return selectedTile;
}

std::vector<const WorldObject*> Room::getObjects() const
{
    std::vector<const WorldObject*> objects;
    for (int y = 0; y < roomData.tileSize.y; y++)
    {
        for (int x = 0; x < roomData.tileSize.x; x++)
        {
            if (!objectGrid[y][x].has_value())
                continue;

            // Add object to vector
            objects.push_back(&objectGrid[y][x].value());
        }
    }

    return objects;
}

void Room::draw(sf::RenderTarget& window) const
{
    float scale = ResolutionHandler::getScale();

    TextureDrawData drawData;
    drawData.position = Camera::worldToScreenTransform(sf::Vector2f(0, 0));
    drawData.scale = sf::Vector2f(scale, scale);
    drawData.type = TextureType::Rooms;

    TextureManager::drawSubTexture(window, drawData, roomData.textureRect);
    
    if (DebugOptions::drawCollisionRects)
    {
        for (const CollisionRect& rect : collisionRects)
        {
            rect.debugDraw(window);
        }
    }
}