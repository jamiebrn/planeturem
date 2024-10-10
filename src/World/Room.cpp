#include "World/Room.hpp"

Room::Room()
{
    
}

Room::Room(const RoomData& roomData, ChestDataPool& chestDataPool)
{
    this->roomData = roomData;

    createObjects(chestDataPool);

    createCollisionRects();
}

Room::Room(const Room& room)
{
    *this = room;
}

Room& Room::operator=(const Room& room)
{
    roomData = room.roomData;
    collisionRects = room.collisionRects;
    warpExitRect = room.warpExitRect;

    objectGrid.clear();

    // Copy object ptrs
    for (int y = 0; y < room.objectGrid.size(); y++)
    {
        // Create empty vector of ptrs
        objectGrid.emplace_back();

        for (int x = 0; x < room.objectGrid[y].size(); x++)
        {
            // Check for nullptr
            if (!room.objectGrid[y][x])
            {
                objectGrid[y].emplace_back(nullptr);
                continue;
            }

            // Deep copy ptr
            objectGrid[y].emplace_back(room.objectGrid[y][x]->clone());
        }
    }

    return *this;
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

void Room::createObjects(ChestDataPool& chestDataPool)
{
    const sf::Image& bitmaskImage = TextureManager::getBitmask(BitmaskType::Structures);

    for (int y = 0; y < roomData.tileSize.y; y++)
    {
        objectGrid.emplace_back();

        for (int x = 0; x < roomData.tileSize.x; x++)
        {
            std::unique_ptr<BuildableObject> object = nullptr;

            // Sample bitmask
            sf::Color bitmaskColor = bitmaskImage.getPixel(roomData.collisionBitmaskOffset.x + x, roomData.collisionBitmaskOffset.y + y);

            // Create object
            if (roomData.objectsInRoom.contains(bitmaskColor.b))
            {
                const RoomObjectData& roomObjectData = roomData.objectsInRoom.at(bitmaskColor.b);

                ObjectType objectTypeToSpawn = roomObjectData.objectType;

                sf::Vector2f objectPos((x + 0.5f) * TILE_SIZE_PIXELS_UNSCALED, (y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED);
                
                object = BuildableObjectFactory::create(objectPos, objectTypeToSpawn);

                if (roomObjectData.chestContents.has_value())
                {
                    if (ChestObject* chest = dynamic_cast<ChestObject*>(object.get()))
                    {
                        uint16_t chestID = chestDataPool.createChest(roomObjectData.chestContents.value());

                        chest->setChestID(chestID);
                    }
                }
            }

            // Add to array
            objectGrid.back().push_back(std::move(object));
        }
    }
}

void Room::createCollisionRects()
{
    collisionRects.clear();

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
            BuildableObject* object = objectGrid[y][x].get();
            if (object)
            {
                const ObjectData& objectData = ObjectDataLoader::getObjectData(object->getObjectType());
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

void Room::updateObjects(Game& game, float dt)
{
    for (int y = 0; y < objectGrid.size(); y++)
    {
        for (int x = 0; x < objectGrid[y].size(); x++)
        {
            BuildableObject* object = objectGrid[y][x].get();

            if (!object)
                continue;

            object->update(game, dt, false);
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
    
    return objectGrid[tile.y][tile.x].get();
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
    for (int y = 0; y < objectGrid.size(); y++)
    {
        for (int x = 0; x < objectGrid[y].size(); x++)
        {
            if (!objectGrid[y][x])
                continue;

            // Add object to vector
            objects.push_back(objectGrid[y][x].get());
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


std::vector<std::vector<std::optional<BuildableObjectPOD>>> Room::getObjectPODs() const
{
    std::vector<std::vector<std::optional<BuildableObjectPOD>>> pods(objectGrid.size());
    for (int y = 0; y < objectGrid.size(); y++)
    {
        pods[y].reserve(objectGrid[y].size());

        for (int x = 0; x < objectGrid[y].size(); x++)
        {
            pods[y].emplace_back();

            if (!objectGrid[y][x])
            {
                pods[y][x] = std::nullopt;
                continue;
            }

            // Create POD for object
            pods[y][x] = objectGrid[y][x]->getPOD();
        }
    }

    return pods;
}

void Room::loadObjectPODs(const std::vector<std::vector<std::optional<BuildableObjectPOD>>>& pods)
{
    objectGrid.clear();

    for (int y = 0; y < pods.size(); y++)
    {
        objectGrid.emplace_back(pods[y].size());

        for (int x = 0; x < pods[y].size(); x++)
        {
            objectGrid[y].emplace_back();

            if (!pods[y][x].has_value())
            {
                objectGrid[y][x] = nullptr;
                continue;
            }

            // Object from POD
            sf::Vector2f objectPos((x + 0.5f) * TILE_SIZE_PIXELS_UNSCALED, (y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED);

            std::unique_ptr<BuildableObject> object = BuildableObjectFactory::create(objectPos, pods[y][x]->objectType);

            object->loadFromPOD(pods[y][x].value());
            
            objectGrid[y][x] = std::move(object);
        }
    }

    createCollisionRects();
}