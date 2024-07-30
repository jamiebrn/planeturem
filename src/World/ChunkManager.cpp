#include "World/ChunkManager.hpp"

// std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> ChunkManager::storedChunks;
// std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> ChunkManager::loadedChunks;

void ChunkManager::updateChunks(const FastNoiseLite& noise, int worldSize)
{
    // Chunk load/unload

    // Get tile size
    float tileSize = ResolutionHandler::getTileSize();

    // Get screen bounds
    sf::Vector2f screenTopLeft = -Camera::getDrawOffset();
    sf::Vector2f screenBottomRight = -Camera::getDrawOffset() + static_cast<sf::Vector2f>(ResolutionHandler::getResolution());

    // Convert screen bounds to chunk units
    sf::Vector2i screenTopLeftGrid(std::floor(screenTopLeft.x / (tileSize * 8)), std::floor(screenTopLeft.y / (tileSize * 8)));
    sf::Vector2i screenBottomRightGrid = screenTopLeftGrid + sf::Vector2i(
        std::ceil(ResolutionHandler::getResolution().x / (tileSize * 8)), std::ceil(ResolutionHandler::getResolution().y / (tileSize * 8)) + 1);

    // Check any chunks needed to load
    for (int y = screenTopLeftGrid.y - 1; y <= screenBottomRightGrid.y + 1; y++)
    {
        for (int x = screenTopLeftGrid.x - 1; x <= screenBottomRightGrid.x + 1; x++)
        {
            // Get wrapped x and y using world size
            int wrappedX = ((x % worldSize) + worldSize) % worldSize;
            int wrappedY = ((y % worldSize) + worldSize) % worldSize;

            // Chunk already loaded
            if (loadedChunks.count(ChunkPosition(wrappedX, wrappedY)))
                continue;
            
            // Chunk not loaded

            // Calculate chunk world pos
            sf::Vector2f chunkWorldPos;
            chunkWorldPos.x = x * 8.0f * tileSize;
            chunkWorldPos.y = y * 8.0f * tileSize;

            // Check if chunk is in memory, and load if so
            if (storedChunks.count(ChunkPosition(wrappedX, wrappedY)))
            {
                // Move chunk into loaded chunks for rendering
                loadedChunks[ChunkPosition(wrappedX, wrappedY)] = std::move(storedChunks[ChunkPosition(wrappedX, wrappedY)]);
                storedChunks.erase(ChunkPosition(wrappedX, wrappedY));

                // Update chunk position
                loadedChunks[ChunkPosition(wrappedX, wrappedY)]->setWorldPosition(chunkWorldPos);

                continue;
            }

            // Generate new chunk if does not exist
            std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(sf::Vector2i(wrappedX, wrappedY));
            chunk->generateChunk(noise);

            // Set chunk position
            chunk->setWorldPosition(chunkWorldPos);

            loadedChunks.emplace(ChunkPosition(wrappedX, wrappedY), std::move(chunk));
        }
    }

    // Function for getting chunk coordinates in correct repeating format
    auto normalizeChunkCoordinate = [worldSize](int coordinate) -> int
    {
        if (coordinate < 0)
            return (coordinate % worldSize) + worldSize;
        return coordinate % worldSize;
    };

    // Check any loaded chunks need unloading
    for (auto iter = loadedChunks.begin(); iter != loadedChunks.end();)
    {
        ChunkPosition chunkPos = iter->first;

        // Calculate normalized chunk positions
        int normalizedChunkX = normalizeChunkCoordinate(chunkPos.x);
        int normalizedChunkY = normalizeChunkCoordinate(chunkPos.y);

        int normalizedScreenLeft = normalizeChunkCoordinate(screenTopLeftGrid.x);
        int normalizedScreenRight = normalizeChunkCoordinate(screenBottomRightGrid.x);
        int normalizedScreenTop = normalizeChunkCoordinate(screenTopLeftGrid.y);
        int normalizedScreenBottom = normalizeChunkCoordinate(screenBottomRightGrid.y);

        // Use normalized chunk coordinates to determine whether the chunk is visible
        bool chunkVisibleX = (normalizedChunkX >= normalizedScreenLeft && normalizedChunkX <= normalizedScreenRight) ||
                        (normalizedScreenLeft > normalizedScreenRight && 
                        (normalizedChunkX >= normalizedScreenLeft || normalizedChunkX <= normalizedScreenRight));

        bool chunkVisibleY = (normalizedChunkY >= normalizedScreenTop && normalizedChunkY <= normalizedScreenBottom) ||
                        (normalizedScreenTop > normalizedScreenBottom && 
                        (normalizedChunkY >= normalizedScreenTop || normalizedChunkY <= normalizedScreenBottom));
        
        bool chunkVisible = chunkVisibleX && chunkVisibleY;

        // If chunk is not visible, unload chunk
        if (!chunkVisible)
        {
            // Store chunk in chunk memory
            storedChunks[chunkPos] = std::move(iter->second);
            // Unload chunk
            iter = loadedChunks.erase(iter);
            continue;
        }
        iter++;
    }
}

void ChunkManager::drawChunkTerrain(sf::RenderWindow& window)
{
    for (auto& chunkPair : loadedChunks)
    {
        ChunkPosition chunkPos = chunkPair.first;
        std::unique_ptr<Chunk>& chunk = chunkPair.second;
        
        chunk->drawChunkTerrain(window);
    }
}

void ChunkManager::updateChunksObjects(float dt)
{
    for (auto& chunkPair : loadedChunks)
    {
        chunkPair.second->updateChunkObjects(dt, *this);
    }
}

std::optional<BuildableObject>& ChunkManager::getChunkObject(ChunkPosition chunk, sf::Vector2i tile)
{
    // Empty object to return if chunk does not exist
    static std::optional<BuildableObject> null = std::nullopt;

    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return null;
    
    // Get object from chunk
    std::optional<BuildableObject>& selectedObject = loadedChunks[chunk]->getObject(sf::Vector2i(tile.x, tile.y));

    // Test if object is object reference object, to then get the actual object
    if (selectedObject->isObjectReference())
    {
        const ObjectReference& objectReference = selectedObject->getObjectReference().value();
        return getChunkObject(objectReference.chunk, objectReference.tile);
    }

    // Return retrieved object
    return selectedObject;
}

// bool ChunkManager::interactWithObject(sf::Vector2i selected_tile)
// {

// }

TileType ChunkManager::getChunkTileType(ChunkPosition chunk, sf::Vector2i tile) const
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return TileType::Water;
    
    return loadedChunks.at(chunk)->getTileType(tile);
}

void ChunkManager::setObject(ChunkPosition chunk, sf::Vector2i tile, unsigned int objectType)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return;
    
    // Set chunk object at position
    loadedChunks[chunk]->setObject(tile, objectType, *this);
}

void ChunkManager::deleteObject(ChunkPosition chunk, sf::Vector2i tile)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return;
    
    loadedChunks[chunk]->deleteObject(tile, *this);
}

// unsigned int ChunkManager::getObjectTypeFromObjectReference(const ObjectReference& objectReference) const
// {
//     // Chunk does not exist
//     if (loadedChunks.count(objectReference.chunk) <= 0)
//         return 0;
    
//     return loadedChunks.at(objectReference.chunk)->getObjectGrid()[objectReference.tile.y][objectReference.tile.x].value().getObjectType();
// }

void ChunkManager::setObjectReference(const ChunkPosition& chunk, const ObjectReference& objectReference, sf::Vector2i tile)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return;
    
    loadedChunks[chunk]->setObjectReference(objectReference, tile);
}

bool ChunkManager::canPlaceObject(ChunkPosition chunk, sf::Vector2i tile, unsigned int objectType)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return false;

    return loadedChunks[chunk]->canPlaceObject(tile, objectType, *this);
}

std::vector<WorldObject*> ChunkManager::getChunkObjects()
{
    std::vector<WorldObject*> objects;
    for (auto& chunkPair : loadedChunks)
    {
        std::vector<WorldObject*> chunkObjects = chunkPair.second->getObjects();
        objects.insert(objects.end(), chunkObjects.begin(), chunkObjects.end());
    }
    return objects;
}

std::vector<std::unique_ptr<CollisionRect>> ChunkManager::getChunkCollisionRects()
{
    std::vector<std::unique_ptr<CollisionRect>> collisionRects;
    for (auto& chunkPair : loadedChunks)
    {
        std::vector<std::unique_ptr<CollisionRect>> chunkCollisionRects = chunkPair.second->getCollisionRects(*this);
        collisionRects.insert(collisionRects.end(), std::make_move_iterator(chunkCollisionRects.begin()), std::make_move_iterator(chunkCollisionRects.end()));
    }
    return collisionRects;
}