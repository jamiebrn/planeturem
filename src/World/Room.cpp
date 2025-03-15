#include "World/Room.hpp"

Room::Room()
{
    
}

Room::Room(RoomType roomType, ChestDataPool* chestDataPool)
{
    this->roomType = roomType;

    createObjects(chestDataPool);

    createCollisionRects();
}

Room::Room(const Room& room)
{
    *this = room;
}

Room& Room::operator=(const Room& room)
{
    roomType = room.roomType;
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

void Room::createObjects(ChestDataPool* chestDataPool)
{
    const sf::Image& bitmaskImage = TextureManager::getBitmask(BitmaskType::Structures);

    const RoomData& roomData = StructureDataLoader::getRoomData(roomType);

    // Initialise object grid
    objectGrid.clear();

    for (int y = 0; y < roomData.tileSize.y; y++)
    {
        objectGrid.emplace_back();

        for (int x = 0; x < roomData.tileSize.x; x++)
        {
            objectGrid[y].emplace_back(nullptr);
        }
    }

    for (int y = 0; y < roomData.tileSize.y; y++)
    {
        for (int x = 0; x < roomData.tileSize.x; x++)
        {
            // std::unique_ptr<BuildableObject> object = nullptr;

            // Sample bitmask
            sf::Color bitmaskColour = bitmaskImage.getPixel(roomData.collisionBitmaskOffset.x + x, roomData.collisionBitmaskOffset.y + y);

            // Create object
            setObjectFromBitmask(sf::Vector2i(x, y), bitmaskColour.b, chestDataPool);

            // Add to array
            // objectGrid.back().push_back(std::move(object));
        }
    }
}

void Room::createCollisionRects()
{
    collisionRects.clear();

    const sf::Image& bitmaskImage = TextureManager::getBitmask(BitmaskType::Structures);

    const RoomData& roomData = StructureDataLoader::getRoomData(roomType);

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
            BuildableObject* object = getObject(sf::Vector2i(x, y));
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
    if (!warpExitRect.has_value())
    {
        return false;
    }

    return (warpExitRect->isPointInRect(playerPos.x, playerPos.y));
}

std::optional<sf::Vector2f> Room::getEntrancePosition() const
{
    if (!warpExitRect.has_value())
    {
        return std::nullopt;
    }

    sf::Vector2f entrancePos;
    entrancePos.x = warpExitRect->x + 0.5f * TILE_SIZE_PIXELS_UNSCALED;
    entrancePos.y = warpExitRect->y - 0.5f * TILE_SIZE_PIXELS_UNSCALED;

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

// BuildableObject* Room::getObject(sf::Vector2f mouseWorldPos)
// {
//     sf::Vector2i selectedTile = getSelectedTile(mouseWorldPos);

//     return getObject(selectedTile);
// }

void Room::setObjectFromBitmask(sf::Vector2i tile, uint8_t bitmaskValue, ChestDataPool* chestDataPool)
{
    const RoomData& roomData = StructureDataLoader::getRoomData(roomType);

    if (!roomData.objectsInRoom.contains(bitmaskValue))
    {
        return;
    }

    if (objectGrid[tile.y][tile.x] != nullptr)
    {
        return;
    }

    const RoomObjectData& roomObjectData = roomData.objectsInRoom.at(bitmaskValue);

    ObjectType objectTypeToSpawn = roomObjectData.objectType;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectTypeToSpawn);

    sf::Vector2f objectPos = sf::Vector2f(tile.x + 0.5f, tile.y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
    
    std::unique_ptr<BuildableObject> object = BuildableObjectFactory::create(objectPos, objectTypeToSpawn);

    if (roomObjectData.chestContents.has_value() && chestDataPool != nullptr)
    {
        if (roomObjectData.chestContents->size() > 0)
        {
            if (ChestObject* chest = dynamic_cast<ChestObject*>(object.get()))
            {
                const InventoryData& randomChestContents = roomObjectData.chestContents.value()[rand() % roomObjectData.chestContents->size()];

                uint16_t chestID = chestDataPool->createChest(randomChestContents);

                chest->setChestID(chestID);
            }
        }
    }

    objectGrid[tile.y][tile.x] = std::move(object);

    if (objectData.size == sf::Vector2i(1, 1))
    {
        return;
    }

    // Create object references
    ObjectReference objectReference;
    objectReference.tile = sf::Vector2i(tile.x, tile.y);

    for (int y = 0; y < objectData.size.y; y++)
    {
        if (y >= objectGrid.size())
        {
            break;
        }

        for (int x = 0; x < objectData.size.x; x++)
        {
            if (x == 0 && y == 0)
            {
                continue;
            }

            if (x >= objectGrid[y].size())
            {
                break;
            }
            
            objectGrid[tile.y + y][tile.x + x] = std::make_unique<BuildableObject>(objectReference);
        }
    }
}

bool Room::getFirstRocketObjectReference(ObjectReference& objectReference) const
{
    for (int y = 0; y < objectGrid.size(); y++)
    {
        for (int x = 0; x < objectGrid[y].size(); x++)
        {
            BuildableObject* object = objectGrid[y][x].get();

            if (!object)
            {
                continue;
            }

            if (object->isObjectReference() || object->isDummyObject())
            {
                continue;
            }

            const ObjectData& objectData = ObjectDataLoader::getObjectData(object->getObjectType());

            if (objectData.rocketObjectData.has_value())
            {
                objectReference.chunk = ChunkPosition(0, 0);
                objectReference.tile = sf::Vector2i(x, y);
                return true;
            }
        }
    }

    return false;
}

RoomType Room::getRoomType() const
{
    return roomType;
}

// sf::Vector2i Room::getSelectedTile(sf::Vector2f mouseWorldPos)
// {
//     sf::Vector2i selectedTile;
//     selectedTile.x = std::floor(mouseWorldPos.x / TILE_SIZE_PIXELS_UNSCALED);
//     selectedTile.y = std::floor(mouseWorldPos.y / TILE_SIZE_PIXELS_UNSCALED);
//     return selectedTile;
// }

std::vector<const WorldObject*> Room::getObjects() const
{
    std::vector<const WorldObject*> objects;
    for (int y = 0; y < objectGrid.size(); y++)
    {
        for (int x = 0; x < objectGrid[y].size(); x++)
        {
            BuildableObject* object = objectGrid[y][x].get();

            if (!object)
            {
                continue;
            }

            if (object->isObjectReference())
            {
                continue;
            }

            // Add object to vector
            objects.push_back(object);
        }
    }

    return objects;
}

void Room::draw(sf::RenderTarget& window, const Camera& camera) const
{
    float scale = ResolutionHandler::getScale();

    TextureDrawData drawData;
    drawData.position = camera.worldToScreenTransform(sf::Vector2f(0, 0));
    drawData.scale = sf::Vector2f(scale, scale);
    drawData.type = TextureType::Rooms;

    const RoomData& roomData = StructureDataLoader::getRoomData(roomType);

    TextureManager::drawSubTexture(window, drawData, roomData.textureRect);

    #if (!RELEASE_BUILD)
    if (DebugOptions::drawCollisionRects)
    {
        for (const CollisionRect& rect : collisionRects)
        {
            rect.debugDraw(window, camera);
        }
    }
    #endif
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

void Room::loadObjectPODs()
{
    objectGrid.clear();

    if (loadingObjectPodsTemp == nullptr)
    {
        std::cout << "Error: Room has no object POD loaded\n";
        return;
    }

    for (int y = 0; y < loadingObjectPodsTemp->size(); y++)
    {
        objectGrid.emplace_back(loadingObjectPodsTemp->at(y).size());

        for (int x = 0; x < loadingObjectPodsTemp->at(y).size(); x++)
        {
            if (!loadingObjectPodsTemp->at(y).at(x).has_value())
            {
                objectGrid[y][x] = nullptr;
                continue;
            }

            // Object from POD
            sf::Vector2f objectPos((x + 0.5f) * TILE_SIZE_PIXELS_UNSCALED, (y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED);

            std::unique_ptr<BuildableObject> object = BuildableObjectFactory::create(objectPos, loadingObjectPodsTemp->at(y).at(x)->objectType);

            object->loadFromPOD(loadingObjectPodsTemp->at(y).at(x).value());

            objectGrid[y][x] = std::move(object);
        }
    }

    createCollisionRects();
}