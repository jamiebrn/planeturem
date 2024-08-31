#include "World/ChunkManager.hpp"

// std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> ChunkManager::storedChunks;
// std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> ChunkManager::loadedChunks;

void ChunkManager::updateChunks(const FastNoise& noise, int worldSize)
{
    // Chunk load/unload

    // Get tile size
    float tileSize = ResolutionHandler::getTileSize();

    // Get screen bounds
    // sf::Vector2f screenTopLeft = -Camera::getDrawOffset();
    // sf::Vector2f screenBottomRight = -Camera::getDrawOffset() + static_cast<sf::Vector2f>(ResolutionHandler::getResolution());
    sf::Vector2f screenTopLeft = Camera::screenToWorldTransform({0, 0});
    sf::Vector2f screenBottomRight = Camera::screenToWorldTransform(static_cast<sf::Vector2f>(ResolutionHandler::getResolution()));

    // Convert screen bounds to chunk units
    sf::Vector2i screenTopLeftGrid;
    sf::Vector2i screenBottomRightGrid;

    screenTopLeftGrid.y = std::floor(screenTopLeft.y / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));
    screenTopLeftGrid.x = std::floor(screenTopLeft.x / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));
    screenBottomRightGrid.x = std::ceil(screenBottomRight.x / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));
    screenBottomRightGrid.y = std::ceil(screenBottomRight.y / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));

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
            chunkWorldPos.x = x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
            chunkWorldPos.y = y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;

            // Check if chunk is in memory, and load if so
            if (storedChunks.count(ChunkPosition(wrappedX, wrappedY)))
            {
                // Move chunk into loaded chunks for rendering
                loadedChunks[ChunkPosition(wrappedX, wrappedY)] = std::move(storedChunks[ChunkPosition(wrappedX, wrappedY)]);
                storedChunks.erase(ChunkPosition(wrappedX, wrappedY));

                // Update chunk position
                loadedChunks[ChunkPosition(wrappedX, wrappedY)]->setWorldPosition(chunkWorldPos, *this);

                continue;
            }

            // Generate new chunk if does not exist
            generateChunk(ChunkPosition(wrappedX, wrappedY), noise, worldSize, true, chunkWorldPos);
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

void ChunkManager::reloadChunks()
{
    for (auto iter = loadedChunks.begin(); iter != loadedChunks.end();)
    {
        ChunkPosition chunkPos = iter->first;
        
        // Store chunk in chunk memory
        storedChunks[chunkPos] = std::move(iter->second);
        // Unload chunk
        iter = loadedChunks.erase(iter);
    }
}

void ChunkManager::drawChunkTerrain(sf::RenderTarget& window, SpriteBatch& spriteBatch, float time)
{
    // Draw terrain
    for (auto& chunkPair : loadedChunks)
    {
        ChunkPosition chunkPos = chunkPair.first;
        std::unique_ptr<Chunk>& chunk = chunkPair.second;
        
        chunk->drawChunkTerrain(window, time);
    }

    // Draw visual terrain features e.g. cliffs
    for (auto& chunkPair : loadedChunks)
    {
        ChunkPosition chunkPos = chunkPair.first;
        std::unique_ptr<Chunk>& chunk = chunkPair.second;
        
        chunk->drawChunkTerrainVisual(window, spriteBatch, time);
    }
}

void ChunkManager::drawChunkWater(sf::RenderTarget& window, float time)
{
    // Set shader time paramenter
    sf::Shader* waterShader = Shaders::getShader(ShaderType::Water);
    waterShader->setUniform("time", time);

    for (auto& chunkPair : loadedChunks)
    {
        ChunkPosition chunkPos = chunkPair.first;
        std::unique_ptr<Chunk>& chunk = chunkPair.second;
        
        chunk->drawChunkWater(window);
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

TileType ChunkManager::getLoadedChunkTileType(ChunkPosition chunk, sf::Vector2i tile) const
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return TileType::Water;
    
    return loadedChunks.at(chunk)->getTileType(tile);
}

TileType ChunkManager::getChunkTileType(ChunkPosition chunk, sf::Vector2i tile) const
{
    // Chunk is not generated
    if (!isChunkGenerated(chunk))
        return TileType::Water;
    
    // Not in loaded chunks, go to stored chunks
    if (loadedChunks.count(chunk) <= 0)
        return storedChunks.at(chunk)->getTileType(tile);
    
    return loadedChunks.at(chunk)->getTileType(tile);
}

bool ChunkManager::isChunkGenerated(ChunkPosition chunk) const
{
    return (loadedChunks.count(chunk) + storedChunks.count(chunk)) > 0;
}

void ChunkManager::setObject(ChunkPosition chunk, sf::Vector2i tile, ObjectType objectType, int worldSize)
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

bool ChunkManager::canPlaceObject(ChunkPosition chunk, sf::Vector2i tile, ObjectType objectType, int worldSize, const CollisionRect& playerCollisionRect)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return false;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    if (objectData.hasCollision)
    {
        // Create collision rect for object using world position
        sf::Vector2f chunkWorldPosition = loadedChunks[chunk]->getWorldPosition();

        CollisionRect objectCollisionRect;
        objectCollisionRect.x = chunkWorldPosition.x + tile.x * TILE_SIZE_PIXELS_UNSCALED;
        objectCollisionRect.y = chunkWorldPosition.y + tile.y * TILE_SIZE_PIXELS_UNSCALED;
        objectCollisionRect.width = objectData.size.x * TILE_SIZE_PIXELS_UNSCALED;
        objectCollisionRect.height = objectData.size.y * TILE_SIZE_PIXELS_UNSCALED;

        // Test if colliding with player
        if (playerCollisionRect.isColliding(objectCollisionRect))
            return false;
        
        // Test if colliding with entities in adjacent chunks
        for (int y = chunk.y - 1; y <= chunk.y + 1; y++)
        {
            for (int x = chunk.x - 1; x <= chunk.x + 1; x++)
            {
                int wrappedX = (x % worldSize + worldSize) % worldSize;
                int wrappedY = (y % worldSize + worldSize) % worldSize;

                // FIX: May have to check entities in stored chunks aswell

                // If chunk does not exist, do not attempt to check collision
                if (loadedChunks.count(ChunkPosition(wrappedX, wrappedY)) <= 0)
                    continue;
                
                if (loadedChunks[ChunkPosition(wrappedX, wrappedY)]->isCollisionRectCollidingWithEntities(objectCollisionRect))
                    return false;
            }
        }
    }

    // If not colliding with player / entities, test whether colliding with other objects
    return loadedChunks[chunk]->canPlaceObject(tile, objectType, worldSize, *this);
}

bool ChunkManager::canDestroyObject(ChunkPosition chunk, sf::Vector2i tile, int worldSize, const CollisionRect& playerCollisionRect)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return false;
    
    std::optional<BuildableObject>& objectOptional = loadedChunks[chunk]->getObject(tile);

    if (!objectOptional.has_value())
        return false;
    
    BuildableObject& object = objectOptional.value();

    const ObjectData& objectData = ObjectDataLoader::getObjectData(object.getObjectType());

    // Object is a water object, so may support player / entities over water
    // Therefore must check against player / entities before destroying
    if (objectData.placeOnWater)
    {
        // Create collision rect for object using world position
        sf::Vector2f chunkWorldPosition = loadedChunks[chunk]->getWorldPosition();

        CollisionRect objectCollisionRect;
        objectCollisionRect.x = chunkWorldPosition.x + tile.x * TILE_SIZE_PIXELS_UNSCALED;
        objectCollisionRect.y = chunkWorldPosition.y + tile.y * TILE_SIZE_PIXELS_UNSCALED;
        objectCollisionRect.width = objectData.size.x * TILE_SIZE_PIXELS_UNSCALED;
        objectCollisionRect.height = objectData.size.y * TILE_SIZE_PIXELS_UNSCALED;

        // Test if colliding with player
        if (playerCollisionRect.isColliding(objectCollisionRect))
            return false;
        
        // Test if colliding with entities in adjacent chunks
        for (int x = chunk.x - 1; x <= chunk.x + 1; x++)
        {
            for (int y = chunk.y - 1; y <= chunk.y + 1; y++)
            {
                int wrappedX = (x % worldSize + worldSize) % worldSize;
                int wrappedY = (y % worldSize + worldSize) % worldSize;

                // FIX: May have to check entities in stored chunks aswell

                // If chunk does not exist, do not attempt to check collision
                if (loadedChunks.count(ChunkPosition(wrappedX, wrappedY)) <= 0)
                    continue;
                
                if (loadedChunks[ChunkPosition(wrappedX, wrappedY)]->isCollisionRectCollidingWithEntities(objectCollisionRect))
                    return false;
            }
        }
    }

    // Default case
    return true;
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

bool ChunkManager::canPlaceLand(ChunkPosition chunk, sf::Vector2i tile)
{
    // Chunk not loaded
    if (loadedChunks.count(chunk) <= 0)
        return false;
    
    return loadedChunks[chunk]->canPlaceLand(tile);
}

void ChunkManager::placeLand(ChunkPosition chunk, sf::Vector2i tile, const FastNoise& noise, int worldSize)
{
    // Chunk not loaded
    if (loadedChunks.count(chunk) <= 0)
        return;
    
    // Cannot place land
    if (!loadedChunks[chunk]->canPlaceLand(tile))
        return;
    
    // Place land and update visual tiles for chunk
    loadedChunks[chunk]->placeLand(tile, worldSize, noise, *this);

    // Update visual tiles for adjacent chunks
    for (int x = chunk.x - 1; x <= chunk.x + 1; x++)
    {
        for (int y = chunk.y - 1; y <= chunk.y + 1; y++)
        {
            // If chunk is centre chunk, do not update as has already been updated
            if (ChunkPosition(x, y) == chunk)
                continue;
            
            // Wrap around world if required
            int wrappedX = (x % worldSize + worldSize) % worldSize;
            int wrappedY = (y % worldSize + worldSize) % worldSize;

            loadedChunks[ChunkPosition(wrappedX, wrappedY)]->generateVisualEffectTiles(noise, worldSize, *this);
        }
    }
}

sf::Vector2f ChunkManager::findValidSpawnPosition(int waterlessAreaSize, const FastNoise& noise, int worldSize)
{
    // Move all loaded chunks (if any) into stored chunks temporarily
    // Makes testing area simpler as only have to check stored chunk map
    reloadChunks();

    int searchIncrement = 1 + waterlessAreaSize * 2;

    // Iterate over all chunks and check area
    for (int y = 0; y < worldSize; y += searchIncrement)
    {
        for (int x = 0; x < worldSize; x += searchIncrement)
        {
            bool validSpawn = true;

            // Check over area around chunk, including centre chunk
            for (int yArea = y - waterlessAreaSize; yArea <= y + waterlessAreaSize; yArea++)
            {
                for (int xArea = x - waterlessAreaSize; xArea <= x + waterlessAreaSize; xArea++)
                {
                    int wrappedX = (xArea % worldSize + worldSize) % worldSize;
                    int wrappedY = (yArea % worldSize + worldSize) % worldSize;

                    // If not generated, generate
                    if (storedChunks.count(ChunkPosition(wrappedX, wrappedY)) <= 0)
                        generateChunk(ChunkPosition(wrappedX, wrappedY), noise, worldSize, false);
                    
                    // If chunk does not contain water, continue to check other chunks
                    if (!storedChunks[ChunkPosition(wrappedX, wrappedY)]->getContainsWater())
                        continue;
                    
                    // Chunk contains water - move onto checking next area
                    validSpawn = false;
                    break;
                }

                if (!validSpawn)
                    break;
            }

            if (!validSpawn)
                continue;
            
            // Is valid spawn - calculate position of centre of chunk and return
            sf::Vector2f spawnPosition;
            spawnPosition.x = x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
            spawnPosition.y = y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;

            return spawnPosition;
        }
    }

    // Recursive call to find smaller valid area
    if (waterlessAreaSize > 0)
        return findValidSpawnPosition(waterlessAreaSize - 1, noise, worldSize);
    
    // Can't find valid spawn, so default return
    return sf::Vector2f(0, 0);
}

std::unordered_map<std::string, int> ChunkManager::getNearbyCraftingStationLevels(ChunkPosition playerChunk, sf::Vector2i playerTile, int searchArea, int worldSize)
{
    std::unordered_map<std::string, int> craftingStationLevels;

    // Search area
    for (int xOffset = -searchArea; xOffset <= searchArea; xOffset++)
    {
        for (int yOffset = -searchArea; yOffset <= searchArea; yOffset++)
        {
            // Get chunk and tile
            std::pair<ChunkPosition, sf::Vector2i> chunkTile = getChunkTileFromOffset(playerChunk, playerTile, xOffset, yOffset, worldSize);

            // If chunk not loaded, do not get object
            if (loadedChunks.count(chunkTile.first) <= 0)
                continue;
            
            std::optional<BuildableObject>& object = getChunkObject(chunkTile.first, chunkTile.second);

            // If no object, do not get data
            if (!object.has_value())
                continue;
            
            // No need to test if object reference as getChunkObject handles this

            // Get object data
            ObjectType objectType = object->getObjectType();
            const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

            // If not a crafting station, do not add to hashmap
            if (objectData.craftingStation.empty())
                continue;

            // Add crafting station to map
            if (craftingStationLevels.count(objectData.craftingStation) > 0)
            {
                craftingStationLevels[objectData.craftingStation] = std::max(objectData.craftingStationLevel, craftingStationLevels[objectData.craftingStation]);
            }
            else
            {
                craftingStationLevels[objectData.craftingStation] = objectData.craftingStationLevel;
            }
        }
    }

    return craftingStationLevels;
}

void ChunkManager::generateChunk(const ChunkPosition& chunkPosition, const FastNoise& noise, int worldSize, bool putInLoaded, std::optional<sf::Vector2f> positionOverride)
{
    std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(chunkPosition);

    sf::Vector2f chunkWorldPos;
    if (positionOverride.has_value())
    {
        chunkWorldPos.x = positionOverride->x;
        chunkWorldPos.y = positionOverride->y;
    }
    else
    {
        chunkWorldPos.x = chunkPosition.x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
        chunkWorldPos.y = chunkPosition.y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    }

    // Set chunk position
    chunk->setWorldPosition(chunkWorldPos, *this);

    // Generate
    chunk->generateChunk(noise, worldSize, *this);

    if (putInLoaded)
    {
        loadedChunks.emplace(chunkPosition, std::move(chunk));
        return;
    }
    
    storedChunks.emplace(chunkPosition, std::move(chunk));
}

// Static method
std::pair<ChunkPosition, sf::Vector2i> ChunkManager::getChunkTileFromOffset(ChunkPosition chunk, sf::Vector2i tile, int xOffset, int yOffset, int worldSize)
{
    tile.x += xOffset;
    tile.y += yOffset;

    if (tile.x < 0 || tile.x >= static_cast<int>(CHUNK_TILE_SIZE) || tile.y < 0 || tile.y >= static_cast<int>(CHUNK_TILE_SIZE))
    {
        // Add to chunk position and convert tile position
        // Took way too long to come up with these formulae...
        if (tile.x < 0)
            chunk.x -= std::ceil(std::abs(tile.x) / CHUNK_TILE_SIZE);
        else if (tile.x >= static_cast<int>(CHUNK_TILE_SIZE))
            chunk.x += std::ceil((1 + tile.x) / CHUNK_TILE_SIZE) - 1;

        if (tile.y < 0)
            chunk.y -= std::ceil(std::abs(tile.y) / CHUNK_TILE_SIZE);
        else if (tile.y >= static_cast<int>(CHUNK_TILE_SIZE))
            chunk.y += std::ceil((1 + tile.y) / CHUNK_TILE_SIZE) - 1;

        // Wrap tile
        tile.x = (tile.x % static_cast<int>(CHUNK_TILE_SIZE) + static_cast<int>(CHUNK_TILE_SIZE)) % static_cast<int>(CHUNK_TILE_SIZE);
        tile.y = (tile.y % static_cast<int>(CHUNK_TILE_SIZE) + static_cast<int>(CHUNK_TILE_SIZE)) % static_cast<int>(CHUNK_TILE_SIZE);

        // Wrap around world if necessary
        chunk.x = (chunk.x % worldSize + worldSize) % worldSize;
        chunk.y = (chunk.y % worldSize + worldSize) % worldSize;
    }

    return {chunk, tile};
}