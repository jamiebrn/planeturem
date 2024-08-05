#include "World/ChunkManager.hpp"

// std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> ChunkManager::storedChunks;
// std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> ChunkManager::loadedChunks;

void ChunkManager::updateChunks(const FastNoise& noise, int worldSize)
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
            
            // Set chunk position
            chunk->setWorldPosition(chunkWorldPos);

            chunk->generateChunk(noise, worldSize, *this);


            loadedChunks.emplace(ChunkPosition(wrappedX, wrappedY), std::move(chunk));
        }
    }

    // Check any loaded chunks need unloading
    for (auto iter = loadedChunks.begin(); iter != loadedChunks.end();)
    {
        ChunkPosition chunkPos = iter->first;

        // Calculate normalized chunk positions
        int normalizedChunkX = ((chunkPos.x % worldSize) + worldSize) % worldSize;
        int normalizedChunkY = ((chunkPos.y % worldSize) + worldSize) % worldSize;

        int normalizedScreenLeft = (((screenTopLeftGrid.x - 1) % worldSize) + worldSize) % worldSize;
        int normalizedScreenRight = (((screenBottomRightGrid.x + 1) % worldSize) + worldSize) % worldSize;
        int normalizedScreenTop = (((screenTopLeftGrid.y - 1) % worldSize) + worldSize) % worldSize;
        int normalizedScreenBottom = (((screenBottomRightGrid.y + 1) % worldSize) + worldSize) % worldSize;

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

void ChunkManager::drawChunkTerrain(sf::RenderWindow& window, float time)
{
    for (auto& chunkPair : loadedChunks)
    {
        ChunkPosition chunkPos = chunkPair.first;
        std::unique_ptr<Chunk>& chunk = chunkPair.second;
        
        chunk->drawChunkTerrain(window, time);
    }
}

void ChunkManager::drawChunkWater(sf::RenderWindow& window, float time)
{
    for (auto& chunkPair : loadedChunks)
    {
        ChunkPosition chunkPos = chunkPair.first;
        std::unique_ptr<Chunk>& chunk = chunkPair.second;
        
        chunk->drawChunkWater(window, time);
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

    if (!selectedObject.has_value())
        return null;

    // Test if object is object reference object, to then get the actual object
    if (selectedObject->isObjectReference())
    {
        const ObjectReference& objectReference = selectedObject->getObjectReference().value();
        return getChunkObject(objectReference.chunk, objectReference.tile);
    }

    // Return retrieved object
    return selectedObject;
}

TileType ChunkManager::getChunkTileType(ChunkPosition chunk, sf::Vector2i tile) const
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return TileType::Water;
    
    return loadedChunks.at(chunk)->getTileType(tile);
}

void ChunkManager::setObject(ChunkPosition chunk, sf::Vector2i tile, unsigned int objectType, int worldSize)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return;
    
    // Set chunk object at position
    loadedChunks[chunk]->setObject(tile, objectType, worldSize, *this);
}

void ChunkManager::deleteObject(ChunkPosition chunk, sf::Vector2i tile)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return;
    
    loadedChunks[chunk]->deleteObject(tile, *this);
}

void ChunkManager::setObjectReference(const ChunkPosition& chunk, const ObjectReference& objectReference, sf::Vector2i tile)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return;
    
    loadedChunks[chunk]->setObjectReference(objectReference, tile, *this);
}

bool ChunkManager::canPlaceObject(ChunkPosition chunk, sf::Vector2i tile, unsigned int objectType, int worldSize)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return false;

    return loadedChunks[chunk]->canPlaceObject(tile, objectType, worldSize, *this);
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

void ChunkManager::updateChunksEntities(float dt, int worldSize)
{
    for (auto& chunkPair : loadedChunks)
    {
        chunkPair.second->updateChunkEntities(dt, worldSize, *this);
    }
}

void ChunkManager::moveEntityToChunkFromChunk(std::unique_ptr<Entity> entity, ChunkPosition newChunk)
{
    if (loadedChunks.count(newChunk) <= 0)
        return;
    
    loadedChunks[newChunk]->moveEntityToChunk(std::move(entity));
}

Entity* ChunkManager::getSelectedEntity(ChunkPosition chunk, sf::Vector2f cursorPos)
{
    // Check entities in chunks around cursor in 3x3 area
    // i.e. chunk.x - 1 to chunk.x + 1 and chunk.y - 1 to chunk.y + 1
    for (int x = chunk.x - 1; x <= chunk.x + 1; x++)
    {
        for (int y = chunk.y - 1; y <= chunk.y + 1; y++)
        {
            // Chunk identifier
            ChunkPosition chunkPos(x, y);

            // If chunk not loaded, do not attempt to check chunk
            if (loadedChunks.count(chunkPos) <= 0)
                continue;
            
            auto& chunk = loadedChunks[chunkPos];
            Entity* selectedEntity = chunk->getSelectedEntity(cursorPos);

            // If entity is selected, return it
            if (selectedEntity != nullptr)
                return selectedEntity;
        }
    }
    
    // Default case
    return nullptr;
}

std::vector<CollisionRect*> ChunkManager::getChunkCollisionRects()
{
    std::vector<CollisionRect*> collisionRects;
    for (auto& chunkPair : loadedChunks)
    {
        std::vector<CollisionRect*> chunkCollisionRects = chunkPair.second->getCollisionRects();
        // collisionRects.insert(collisionRects.end(), std::make_move_iterator(chunkCollisionRects.begin()), std::make_move_iterator(chunkCollisionRects.end()));
        collisionRects.insert(collisionRects.end(), chunkCollisionRects.begin(), chunkCollisionRects.end());
    }
    return collisionRects;
}

bool ChunkManager::collisionRectChunkStaticCollisionX(CollisionRect& collisionRect, float dx)
{
    bool collision = false;
    for (auto& chunkPair : loadedChunks)
    {
        if (chunkPair.second->collisionRectStaticCollisionX(collisionRect, dx))
            collision = true;
    }
    return collision;
}

bool ChunkManager::collisionRectChunkStaticCollisionY(CollisionRect& collisionRect, float dy)
{
    bool collision = false;
    for (auto& chunkPair : loadedChunks)
    {
        if (chunkPair.second->collisionRectStaticCollisionY(collisionRect, dy))
            collision = true;
    }
    return collision;
}

std::vector<WorldObject*> ChunkManager::getChunkEntities()
{
    std::vector<WorldObject*> entities;
    for (auto& chunkPair : loadedChunks)
    {
        std::vector<WorldObject*> chunkEntities = chunkPair.second->getEntities();
        entities.insert(entities.end(), chunkEntities.begin(), chunkEntities.end());
    }
    return entities;
}