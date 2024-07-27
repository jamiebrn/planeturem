#include "World/Chunk.hpp"

Chunk::Chunk(sf::Vector2i worldGridPosition)
{
    this->worldGridPosition = worldGridPosition;
    groundVertexArray = sf::VertexArray(sf::Quads, 8 * 8 * 4);
}

void Chunk::generateChunk(const FastNoiseLite& noise)
{
    sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;
    sf::Vector2f worldNoisePosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f;

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            float height = noise.GetNoise(worldNoisePosition.x + (float)x, worldNoisePosition.y + (float)y);

            int vertexArrayIndex = (x + y * 8) * 4;
            groundVertexArray[vertexArrayIndex].position = sf::Vector2f(x * 48, y * 48);
            groundVertexArray[vertexArrayIndex + 1].position = sf::Vector2f(x * 48 + 48, y * 48);
            groundVertexArray[vertexArrayIndex + 3].position = sf::Vector2f(x * 48, y * 48 + 48);
            groundVertexArray[vertexArrayIndex + 2].position = sf::Vector2f(x * 48 + 48, y * 48 + 48);

            if (height < 0)
            {
                groundVertexArray[vertexArrayIndex].texCoords = {2 * 16, 0};
                groundVertexArray[vertexArrayIndex + 1].texCoords = {2 * 16 + 16, 0};
                groundVertexArray[vertexArrayIndex + 3].texCoords = {2 * 16, 16};
                groundVertexArray[vertexArrayIndex + 2].texCoords = {2 * 16 + 16, 16};
                groundTileGrid[y][x] = TileType::Water;
            }
            else if (height > 0.7)
            {
                groundVertexArray[vertexArrayIndex].texCoords = {1 * 16, 0};
                groundVertexArray[vertexArrayIndex + 1].texCoords = {1 * 16 + 16, 0};
                groundVertexArray[vertexArrayIndex + 3].texCoords = {1 * 16, 16};
                groundVertexArray[vertexArrayIndex + 2].texCoords = {1 * 16 + 16, 16};
                groundTileGrid[y][x] = TileType::DarkGrass;
            }
            else if (height >=0 && height < 0.2)
            {
                groundVertexArray[vertexArrayIndex].texCoords = {3 * 16, 0};
                groundVertexArray[vertexArrayIndex + 1].texCoords = {3 * 16 + 16, 0};
                groundVertexArray[vertexArrayIndex + 3].texCoords = {3 * 16, 16};
                groundVertexArray[vertexArrayIndex + 2].texCoords = {3 * 16 + 16, 16};
                groundTileGrid[y][x] = TileType::Sand;
            }
            else
            {
                groundVertexArray[vertexArrayIndex].texCoords = {0 * 16, 0};
                groundVertexArray[vertexArrayIndex + 1].texCoords = {0 * 16 + 16, 0};
                groundVertexArray[vertexArrayIndex + 3].texCoords = {0 * 16, 16};
                groundVertexArray[vertexArrayIndex + 2].texCoords = {0 * 16 + 16, 16};
                groundTileGrid[y][x] = TileType::Grass;
            }

            sf::Vector2f objectPos = worldPosition + sf::Vector2f(x * 48.0f + 24.0f, y * 48.0f + 24.0f);
            objectGrid[y][x] = std::nullopt;

            int spawn_chance = rand() % 40;

            if (spawn_chance < 4 && height >= 0.2)
            {
                // Make tree
                // objectGrid[y][x] = std::move(std::make_unique<BuildableObject>(objectPos, 0));
                objectGrid[y][x] = BuildableObject(objectPos, 0);
            }
            else if (spawn_chance == 4 && height >= 0.2)
            {
                // Make bush
                // objectGrid[y][x] = std::move(std::make_unique<BuildableObject>(objectPos, 1));
                objectGrid[y][x] = BuildableObject(objectPos, 1);
            }
            else if (spawn_chance == 5 && height >= 0.2)
            {
                // Make rock
                // objectGrid[y][x] = std::move(std::make_unique<BuildableObject>(objectPos, 4));
                objectGrid[y][x] = BuildableObject(objectPos, 4);
            }
        }
    }
}

void Chunk::drawChunkTerrain(sf::RenderWindow& window)
{
    sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;

    sf::Transform transform;
    transform.translate(Camera::getIntegerDrawOffset() + worldPosition);

    sf::RenderStates state;
    state.texture = TextureManager::getTexture(TextureType::GroundTiles);
    state.transform = transform;
    window.draw(groundVertexArray, state);

    sf::VertexArray lines(sf::Lines, 8);
    lines[0].position = Camera::getIntegerDrawOffset() + worldPosition; lines[1].position = Camera::getIntegerDrawOffset() + worldPosition + sf::Vector2f(48.0f * 8, 0);
    lines[2].position = Camera::getIntegerDrawOffset() + worldPosition; lines[3].position = Camera::getIntegerDrawOffset() + worldPosition + sf::Vector2f(0, 48.0f * 8);
    lines[4].position = Camera::getIntegerDrawOffset() + worldPosition + sf::Vector2f(48.0f * 8, 0); lines[5].position = Camera::getIntegerDrawOffset() + worldPosition + sf::Vector2f(48.0f * 8, 48.0f * 8);
    lines[6].position = Camera::getIntegerDrawOffset() + worldPosition + sf::Vector2f(0, 48.0f * 8); lines[7].position = Camera::getIntegerDrawOffset() + worldPosition + sf::Vector2f(48.0f * 8, 48.0f * 8);

    window.draw(lines);
}

void Chunk::drawChunkObjects(sf::RenderWindow& window)
{
    // sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;

    // for (int y = 0; y < 8; y++)
    // {
    //     for (int x = 0; x < 8; x++)
    //     {
    //         if (objectGrid[y][x])
    //             objectGrid[y][x]->draw(window, dt);
    //     }
    // }
}

void Chunk::updateChunkObjects(float dt)
{
    for (int y = 0; y < objectGrid.size(); y++)
    {
        auto& object_row = objectGrid[y];
        for (int x = 0; x < object_row.size(); x++)
        {
            auto& object = object_row[x];
            if (object)
            {
                // OccupiedTileObject* occupiedTile = dynamic_cast<OccupiedTileObject*>(object.get());
                // If object is occupied tile, do not update
                if (object->isObjectReference())
                    continue;
                
                object->update(dt);
                if (!object->isAlive())
                    deleteObject(sf::Vector2i(x, y));
            }
        }
    }
}

std::vector<WorldObject*> Chunk::getObjects()
{
    std::vector<WorldObject*> objects;
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            // OccupiedTileObject* occupiedTile = dynamic_cast<OccupiedTileObject*>(objectGrid[y][x].get());
            if (!objectGrid[y][x].has_value())
                continue;

            // If object is occupied tile, do not draw
            if (objectGrid[y][x]->isObjectReference())
                continue;

            if (objectGrid[y][x])
                objects.push_back(&objectGrid[y][x].value());
        }
    }
    return objects;
}

void Chunk::setObject(sf::Vector2i position, unsigned int objectType)
{
    sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;
    sf::Vector2f objectPos = worldPosition + sf::Vector2f(position.x * 48.0f + 24.0f, position.y * 48.0f + 24.0f);

    // std::unique_ptr<BuildableObject> object = std::make_unique<BuildableObject>(objectPos, objectType);
    BuildableObject object(objectPos, objectType);

    sf::Vector2i objectSize = ObjectDataLoader::getObjectData(objectType).size;

    // Create occupied tile objects if object is larger than one tile
    if (objectSize != sf::Vector2i(1, 1))
    {
        ObjectReference objectReference;
        objectReference.chunk = ChunkPosition(worldGridPosition.x, worldGridPosition.y);
        objectReference.tile = position;

        // Iterate over all tiles which object occupies and add occupied tile
        for (int y = position.y; y < std::min(position.y + objectSize.y, (int)objectGrid.size()); y++)
        {
            for (int x = position.x; x < std::min(position.x + objectSize.x, (int)objectGrid[0].size()); x++)
            {
                // If tile is actual object tile, don't place occupied tile
                if (x == position.x && y == position.y)
                    continue;
                
                // sf::Vector2f occupiedTilePos = objectPos + sf::Vector2f(position.x, position.y) * 48.0f;

                // std::unique_ptr<OccupiedTileObject> occupiedTile = std::make_unique<OccupiedTileObject>(occupiedTilePos, objectType, objectReference);

                // std::unique_ptr<BuildableObject> occupiedTile = std::make_unique<BuildableObject>(objectReference);

                // objectGrid[y][x] = std::move(occupiedTile);
                objectGrid[y][x] = BuildableObject(objectReference);
            }
        }

        // Calculate remaining tiles to place, if placed across multiple chunks
        int x_remaining = objectSize.x - ((int)objectGrid[0].size() - 1 - position.x) - 1;
        int y_remaining = objectSize.y - ((int)objectGrid.size() - 1 - position.y) - 1;

        // Add tiles to right (direction) chunk if required
        for (int y = position.y; y < std::min(position.y + objectSize.y, (int)objectGrid.size()); y++)
        {
            for (int x = 0; x < x_remaining; x++)
            {
                ChunkManager::setObjectReference(ChunkPosition(worldGridPosition.x + 1, worldGridPosition.y), objectReference, sf::Vector2i(x, y));
            }
        }

        // Add tiles to down (direction) chunk if required
        for (int y = 0; y < y_remaining; y++)
        {
            for (int x = position.x; x < std::min(position.x + objectSize.x, (int)objectGrid[0].size()); x++)
            {
                ChunkManager::setObjectReference(ChunkPosition(worldGridPosition.x, worldGridPosition.y + 1), objectReference, sf::Vector2i(x, y));
            }
        }

        // Add tiles to down-right (direction) chunk if required
        for (int y = 0; y < y_remaining; y++)
        {
            for (int x = 0; x < x_remaining; x++)
            {
                ChunkManager::setObjectReference(ChunkPosition(worldGridPosition.x + 1, worldGridPosition.y + 1), objectReference, sf::Vector2i(x, y));
            }
        }
    }

    objectGrid[position.y][position.x] = object;
}

void Chunk::deleteObject(sf::Vector2i position)
{
    // Get size of object to handle different deletion cases
    unsigned int objectType = objectGrid[position.y][position.x]->getObjectType();
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    sf::Vector2i objectSize = objectData.size;

    // Object is single tile
    if (objectSize == sf::Vector2i(1, 1))
    {
        objectGrid[position.y][position.x].reset();
        return;
    }

    // Object is multiple tiles in size

    // Delete reference tiles in current chunk
    for (int y = position.y; y < std::min(position.y + objectSize.y, (int)objectGrid.size()); y++)
    {
        for (int x = position.x; x < std::min(position.x + objectSize.x, (int)objectGrid[0].size()); x++)
        {
            objectGrid[y][x].reset();
        }
    }

    // Calculate remaining tiles to delete, if placed across multiple chunks
    int x_remaining = objectSize.x - ((int)objectGrid[0].size() - 1 - position.x) - 1;
    int y_remaining = objectSize.y - ((int)objectGrid.size() - 1 - position.y) - 1;

    // Delete tiles to right (direction) chunk if required
    for (int y = position.y; y < std::min(position.y + objectSize.y, (int)objectGrid.size()); y++)
    {
        for (int x = 0; x < x_remaining; x++)
        {
            ChunkManager::deleteObject(ChunkPosition(worldGridPosition.x + 1, worldGridPosition.y), sf::Vector2i(x, y));
        }
    }

    // Delete tiles to down (direction) chunk if required
    for (int y = 0; y < y_remaining; y++)
    {
        for (int x = position.x; x < std::min(position.x + objectSize.x, (int)objectGrid[0].size()); x++)
        {
            ChunkManager::deleteObject(ChunkPosition(worldGridPosition.x, worldGridPosition.y + 1), sf::Vector2i(x, y));
        }
    }

    // Delete tiles to down-right (direction) chunk if required
    for (int y = 0; y < y_remaining; y++)
    {
        for (int x = 0; x < x_remaining; x++)
        {
            ChunkManager::deleteObject(ChunkPosition(worldGridPosition.x + 1, worldGridPosition.y + 1), sf::Vector2i(x, y));
        }
    }
}

void Chunk::setObjectReference(const ObjectReference& objectReference, sf::Vector2i tile)
{
    objectGrid[tile.y][tile.x] = BuildableObject(objectReference);
}

bool Chunk::canPlaceObject(sf::Vector2i selected_tile, unsigned int objectType)
{
    // Test terrain
    if (groundTileGrid[selected_tile.y][selected_tile.x] == TileType::Water)
        return false;
    
    // Test objects
    if (objectGrid[selected_tile.y][selected_tile.x].has_value())
        return false;
    
    return true;
}

std::vector<std::unique_ptr<CollisionRect>> Chunk::getCollisionRects()
{
    sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;

    auto createCollisionRect = [worldPosition](std::vector<std::unique_ptr<CollisionRect>>& rects, int x, int y) -> void
    {
        std::unique_ptr<CollisionRect> collisionRect = std::make_unique<CollisionRect>();

        collisionRect->x = worldPosition.x + x * 48.0f;
        collisionRect->y = worldPosition.y + y * 48.0f;
        collisionRect->width = 48.0f;
        collisionRect->height = 48.0f;

        rects.push_back(std::move(collisionRect));
    };

    std::vector<std::unique_ptr<CollisionRect>> collisionRects;

    // Get collisions for tiles
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            if (groundTileGrid[y][x] == TileType::Water)
                createCollisionRect(collisionRects, x, y);
        }
    }

    // Get collisions for objects
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            if (!objectGrid[y][x].has_value())
                continue;
            
            // Get object data for reference
            if (objectGrid[y][x]->isObjectReference())
            {
                const ObjectReference& objectReference = objectGrid[y][x]->getObjectReference().value();
                // Get referenced object data
                unsigned int objectType = ChunkManager::getObjectTypeFromObjectReference(objectReference);
                const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

                if (objectData.hasCollision)
                    createCollisionRect(collisionRects, x, y);
            }

            const ObjectData& objectData = ObjectDataLoader::getObjectData(objectGrid[y][x]->getObjectType());
            
            if (objectData.hasCollision)
                createCollisionRect(collisionRects, x, y);
        }
    }

    return collisionRects;
}

bool Chunk::isPointInChunk(sf::Vector2f position)
{
    sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;
    sf::Vector2f chunkExtentPosition = worldPosition + sf::Vector2f(8.0f * 48.0f, 8.0f * 48.0f);

    return (position.x >= worldPosition.x && position.x <= chunkExtentPosition.x
        && position.y >= worldPosition.y && position.y <= chunkExtentPosition.y);
}