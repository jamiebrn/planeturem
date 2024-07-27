#include "World/ChunkManager.hpp"

std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> ChunkManager::storedChunks;
std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> ChunkManager::loadedChunks;

void ChunkManager::updateChunks(const FastNoiseLite& noise)
{
    // Chunk load/unload
    sf::Vector2f screenTopLeft = -Camera::getDrawOffset();
    sf::Vector2f screenBottomRight = -Camera::getDrawOffset() + static_cast<sf::Vector2f>(ResolutionHandler::getResolution());
    sf::Vector2i screenTopLeftGrid(std::floor(screenTopLeft.x / (48 * 8)), std::floor(screenTopLeft.y / (48 * 8)));
    sf::Vector2i screenBottomRightGrid = screenTopLeftGrid + sf::Vector2i(
        std::ceil(ResolutionHandler::getResolution().x / (48 * 8)), std::ceil(ResolutionHandler::getResolution().y / (48 * 8)) + 1);

    // Check any chunks needed to load
    for (int y = screenTopLeftGrid.y; y <= screenBottomRightGrid.y; y++)
    {
        for (int x = screenTopLeftGrid.x; x <= screenBottomRightGrid.x; x++)
        {
            // Chunk already loaded
            if (loadedChunks.count(ChunkPosition(x, y)))
                continue;
            
            // Chunk not loaded

            // Check if chunk is in memory, and load if so
            if (storedChunks.count(ChunkPosition(x, y)))
            {
                loadedChunks[ChunkPosition(x, y)] = std::move(storedChunks[ChunkPosition(x, y)]);
                storedChunks.erase(ChunkPosition(x, y));
                continue;
            }

            // Generate new chunk if does not exist
            std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(sf::Vector2i(x, y));
            chunk->generateChunk(noise);

            loadedChunks.emplace(ChunkPosition(x, y), std::move(chunk));
        }
    }

    // Check any loaded chunks need unloading
    for (auto iter = loadedChunks.begin(); iter != loadedChunks.end();)
    {
        ChunkPosition chunkPos = iter->first;

        if (chunkPos.x < screenTopLeftGrid.x || chunkPos.x > screenBottomRightGrid.x
            || chunkPos.y < screenTopLeftGrid.y || chunkPos.y > screenBottomRightGrid.y)
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
        chunkPair.second->updateChunkObjects(dt);
    }
}

BuildableObject* ChunkManager::getSelectedObject(ChunkPosition chunk, sf::Vector2i tile)
{
    // ChunkPosition chunkPos(std::floor(selected_tile.x / 8.0f), std::floor(selected_tile.y / 8.0f));

    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return nullptr;
    
    // Get objects in chunk
    auto& chunkObjects = loadedChunks[chunk]->getObjectGrid();

    std::optional<BuildableObject>& selectedObject = chunkObjects[tile.y][tile.x];

    if (!selectedObject.has_value())
        return nullptr;

    // Test if object is occupied tile object, to then get the actual object
    if (selectedObject->isObjectReference())
    {
        const ObjectReference& objectReference = selectedObject->getObjectReference().value();
        return getSelectedObject(objectReference.chunk, objectReference.tile);
    }

    // Get object at position and return
    return &(selectedObject.value());
}

// bool ChunkManager::interactWithObject(sf::Vector2i selected_tile)
// {

// }

void ChunkManager::setObject(ChunkPosition chunk, sf::Vector2i tile, unsigned int objectType)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return;
    
    // Set chunk object at position
    loadedChunks[chunk]->setObject(tile, objectType);
}

void ChunkManager::deleteObject(ChunkPosition chunk, sf::Vector2i tile)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return;
    
    loadedChunks[chunk]->deleteObject(tile);
}

unsigned int ChunkManager::getObjectTypeFromObjectReference(const ObjectReference& objectReference)
{
    // Chunk does not exist
    if (loadedChunks.count(objectReference.chunk) <= 0)
        return 0;
    
    return loadedChunks[objectReference.chunk]->getObjectGrid()[objectReference.tile.y][objectReference.tile.x].value().getObjectType();
}

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

    return loadedChunks[chunk]->canPlaceObject(tile, objectType);
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
        std::vector<std::unique_ptr<CollisionRect>> chunkCollisionRects = chunkPair.second->getCollisionRects();
        collisionRects.insert(collisionRects.end(), std::make_move_iterator(chunkCollisionRects.begin()), std::make_move_iterator(chunkCollisionRects.end()));
    }
    return collisionRects;
}