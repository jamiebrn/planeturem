#include "World/ChunkManager.hpp"

void ChunkManager::setSeed(int seed)
{
    this->seed = seed;

    const PlanetGenData& planetGenData = PlanetGenDataLoader::getPlanetGenData(planetType);

    heightNoise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
    biomeNoise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
    riverNoise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
    heightNoise.SetFrequency(planetGenData.heightNoiseFrequency);
    biomeNoise.SetFrequency(planetGenData.biomeNoiseFrequency);
    riverNoise.SetFrequency(planetGenData.riverNoiseFrequency);

    heightNoise.SetSeed(seed + planetType);
    biomeNoise.SetSeed(seed + planetType + 1);
    riverNoise.SetSeed(seed + planetType + 2);
}

int ChunkManager::getSeed() const
{
    return seed;
}

void ChunkManager::setPlanetType(PlanetType planetType)
{
    deleteAllChunks();

    this->planetType = planetType;

    // Update noise seeds
    setSeed(seed);

    const PlanetGenData& planetGenData = PlanetGenDataLoader::getPlanetGenData(planetType);

    // // Set water colour
    // sf::Shader* waterShader = Shaders::getShader(ShaderType::Water);
    // waterShader->setUniform("waterColor", sf::Glsl::Vec4(planetGenData.waterColour));

    // Set planet size
    worldSize = planetGenData.worldSize;

    // Reset pathfinding engine
    int worldTileSize = worldSize * static_cast<int>(CHUNK_TILE_SIZE);
    pathfindingEngine.resize(worldTileSize, worldTileSize);
}

void ChunkManager::deleteAllChunks()
{
    loadedChunks.clear();
    storedChunks.clear();
}

bool ChunkManager::updateChunks(Game& game, const Camera& camera)
{
    // Chunk load/unload

    bool hasModifiedChunks = false;

    // Get tile size
    // float tileSize = ResolutionHandler::getTileSize();

    // Get screen bounds
    // sf::Vector2f screenTopLeft = -Camera::getDrawOffset();
    // sf::Vector2f screenBottomRight = -Camera::getDrawOffset() + static_cast<sf::Vector2f>(ResolutionHandler::getResolution());
    sf::Vector2f screenTopLeft = camera.screenToWorldTransform({0, 0});
    sf::Vector2f screenBottomRight = camera.screenToWorldTransform(static_cast<sf::Vector2f>(ResolutionHandler::getResolution()));

    // Convert screen bounds to chunk units
    sf::Vector2i screenTopLeftGrid;
    sf::Vector2i screenBottomRightGrid;

    screenTopLeftGrid.y = std::floor(screenTopLeft.y / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));
    screenTopLeftGrid.x = std::floor(screenTopLeft.x / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));
    screenBottomRightGrid.x = std::ceil(screenBottomRight.x / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));
    screenBottomRightGrid.y = std::ceil(screenBottomRight.y / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));

    // Check any chunks needed to load
    for (int y = screenTopLeftGrid.y - CHUNK_VIEW_LOAD_BORDER; y <= screenBottomRightGrid.y + CHUNK_VIEW_LOAD_BORDER; y++)
    {
        for (int x = screenTopLeftGrid.x - CHUNK_VIEW_LOAD_BORDER; x <= screenBottomRightGrid.x + CHUNK_VIEW_LOAD_BORDER; x++)
        {
            // Get wrapped x and y using world size
            int wrappedX = ((x % worldSize) + worldSize) % worldSize;
            int wrappedY = ((y % worldSize) + worldSize) % worldSize;

            // Chunk already loaded
            if (loadedChunks.count(ChunkPosition(wrappedX, wrappedY)))
                continue;
            
            // Chunk not loaded
            hasModifiedChunks = true;

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

                auto& chunk = loadedChunks[ChunkPosition(wrappedX, wrappedY)];

                // Update chunk position
                chunk->setWorldPosition(chunkWorldPos, *this);

                // If chunk was loaded through POD / save file, has not yet been initialised (tilemaps, collision, pathfinding etc)
                // Therefore must initialise
                if (chunk->wasGeneratedFromPOD())
                {
                    chunk->generateTilemapsAndInit(*this, pathfindingEngine);
                }

                continue;
            }

            // Generate new chunk if does not exist
            generateChunk(ChunkPosition(wrappedX, wrappedY), game, true, chunkWorldPos);
        }
    }

    // Check any loaded chunks need unloading
    for (auto iter = loadedChunks.begin(); iter != loadedChunks.end();)
    {
        ChunkPosition chunkPos = iter->first;

        // Calculate normalized chunk positions
        int normalizedChunkX = ((chunkPos.x % worldSize) + worldSize) % worldSize;
        int normalizedChunkY = ((chunkPos.y % worldSize) + worldSize) % worldSize;

        int normalizedScreenLeft = (((screenTopLeftGrid.x - CHUNK_VIEW_LOAD_BORDER) % worldSize) + worldSize) % worldSize;
        int normalizedScreenRight = (((screenBottomRightGrid.x + CHUNK_VIEW_LOAD_BORDER) % worldSize) + worldSize) % worldSize;
        int normalizedScreenTop = (((screenTopLeftGrid.y - CHUNK_VIEW_LOAD_BORDER) % worldSize) + worldSize) % worldSize;
        int normalizedScreenBottom = (((screenBottomRightGrid.y + CHUNK_VIEW_LOAD_BORDER) % worldSize) + worldSize) % worldSize;

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
            hasModifiedChunks = true;

            // If chunk has been modified, store it
            if (iter->second->hasBeenModified())
            {
                // Store chunk in chunk memory
                storedChunks[chunkPos] = std::move(iter->second);
            }
            // Unload / delete chunk
            iter = loadedChunks.erase(iter);
            continue;
        }
        iter++;
    }

    return hasModifiedChunks;
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

void ChunkManager::regenerateChunkWithoutStructure(ChunkPosition chunk, Game& game)
{
    Chunk* chunkPtr = getChunk(chunk);

    if (!chunkPtr)
    {
        return;
    }

    // Full reset of chunk
    chunkPtr->reset(true);

    // Regenerate without structure
    chunkPtr->generateChunk(heightNoise, biomeNoise, riverNoise, planetType, game, *this, pathfindingEngine, false);
}

void ChunkManager::drawChunkTerrain(sf::RenderTarget& window, SpriteBatch& spriteBatch, const Camera& camera, float time)
{
    // Draw terrain
    for (auto& chunkPair : loadedChunks)
    {
        ChunkPosition chunkPos = chunkPair.first;
        std::unique_ptr<Chunk>& chunk = chunkPair.second;
        
        chunk->drawChunkTerrain(window, camera, time);
    }

    // Draw visual terrain features e.g. cliffs
    for (auto& chunkPair : loadedChunks)
    {
        ChunkPosition chunkPos = chunkPair.first;
        std::unique_ptr<Chunk>& chunk = chunkPair.second;
        
        chunk->drawChunkTerrainVisual(window, spriteBatch, camera, planetType, time);
    }
}

void ChunkManager::drawChunkWater(sf::RenderTarget& window, const Camera& camera, float time)
{
    // Set shader time paramenter
    sf::Shader* waterShader = Shaders::getShader(ShaderType::Water);
    waterShader->setUniform("time", time);

    // Set water colour
    const PlanetGenData& planetGenData = PlanetGenDataLoader::getPlanetGenData(planetType);
    waterShader->setUniform("waterColor", sf::Glsl::Vec4(planetGenData.waterColour));
    waterShader->setUniform("spriteSheetSize", sf::Glsl::Vec2(TextureManager::getTextureSize(TextureType::Water)));
    waterShader->setUniform("textureRect", sf::Glsl::Vec4(planetGenData.waterTextureOffset.x, planetGenData.waterTextureOffset.y, 32, 32));

    for (auto& chunkPair : loadedChunks)
    {
        ChunkPosition chunkPos = chunkPair.first;
        std::unique_ptr<Chunk>& chunk = chunkPair.second;
        
        chunk->drawChunkWater(window, camera);
    }
}

Chunk* ChunkManager::getChunk(ChunkPosition chunk)
{
    if (loadedChunks.count(chunk) > 0)
    {
        return loadedChunks[chunk].get();
    }
    else if (storedChunks.count(chunk) > 0)
    {
        return storedChunks[chunk].get();
    }

    return nullptr;
}

void ChunkManager::updateChunksObjects(Game& game, float dt)
{
    for (auto& chunkPair : loadedChunks)
    {
        chunkPair.second->updateChunkObjects(game, dt, worldSize, *this, pathfindingEngine);
    }
}

TileMap* ChunkManager::getChunkTileMap(ChunkPosition chunk, int tileMap)
{
    // Chunk is not generated
    if (!isChunkGenerated(chunk))
        return nullptr;
    
    // Not in loaded chunks, go to stored chunks
    if (loadedChunks.count(chunk) <= 0)
        return storedChunks.at(chunk)->getTileMap(tileMap);
    
    return loadedChunks.at(chunk)->getTileMap(tileMap);
}

std::set<int> ChunkManager::setChunkTile(ChunkPosition chunk, int tileMap, sf::Vector2i position, bool tileGraphicUpdate)
{
    if (!isChunkGenerated(chunk))
        return {};

    TileMap* upTiles = getChunkTileMap(ChunkPosition(chunk.x, ((chunk.y - 1) % worldSize + worldSize) % worldSize), tileMap);
    TileMap* downTiles = getChunkTileMap(ChunkPosition(chunk.x, ((chunk.y + 1) % worldSize + worldSize) % worldSize), tileMap);
    TileMap* leftTiles = getChunkTileMap(ChunkPosition(((chunk.x - 1) % worldSize + worldSize) % worldSize, chunk.y), tileMap);
    TileMap* rightTiles = getChunkTileMap(ChunkPosition(((chunk.x + 1) % worldSize + worldSize) % worldSize, chunk.y), tileMap);

    // Update chunk tile
    getChunk(chunk)->setTile(tileMap, position, upTiles, downTiles, leftTiles, rightTiles);

    std::set<int> tileMapsModified = setBackgroundAdjacentTilesForTile(chunk, tileMap, position);

    if (tileGraphicUpdate)
    {
        performChunkSetTileUpdate(chunk, tileMapsModified);   
    }
    
    return tileMapsModified;
}

std::set<int> ChunkManager::setBackgroundAdjacentTilesForTile(ChunkPosition chunk, int tileMap, sf::Vector2i position)
{
    // Update surrounding tiles for "underneath" tile placements
    std::pair<ChunkPosition, sf::Vector2i> chunkTileOffsets[4] = {
        getChunkTileFromOffset(chunk, position, 0, -1, worldSize),  // up
        getChunkTileFromOffset(chunk, position, 0, 1, worldSize),   // down
        getChunkTileFromOffset(chunk, position, -1, 0, worldSize),  // left
        getChunkTileFromOffset(chunk, position, 1, 0, worldSize)    // right
    };

    TileMap* upTiles = getChunkTileMap(ChunkPosition(chunk.x, ((chunk.y - 1) % worldSize + worldSize) % worldSize), tileMap);
    TileMap* downTiles = getChunkTileMap(ChunkPosition(chunk.x, ((chunk.y + 1) % worldSize + worldSize) % worldSize), tileMap);
    TileMap* leftTiles = getChunkTileMap(ChunkPosition(((chunk.x - 1) % worldSize + worldSize) % worldSize, chunk.y), tileMap);
    TileMap* rightTiles = getChunkTileMap(ChunkPosition(((chunk.x + 1) % worldSize + worldSize) % worldSize, chunk.y), tileMap);

    std::set<int> tileMapsModified = {tileMap};

    for (int i = 0; i < 4; i++)
    {
        if (!isChunkGenerated(chunkTileOffsets[i].first))
            continue;

        // Get tile type at position
        int adjacentTileType = getChunkTileType(chunkTileOffsets[i].first, chunkTileOffsets[i].second);

        if (adjacentTileType == 0)
            continue;

        // Get adjacent tilemaps for chunk (as adjacent tile being tested may be in different chunk to original testing tile)
        TileMap* upTilesAdjacent = getChunkTileMap(ChunkPosition(chunkTileOffsets[i].first.x, ((chunkTileOffsets[i].first.y - 1) % worldSize + worldSize) % worldSize), tileMap);
        TileMap* downTilesAdjacent = getChunkTileMap(ChunkPosition(chunkTileOffsets[i].first.x, ((chunkTileOffsets[i].first.y + 1) % worldSize + worldSize) % worldSize), tileMap);
        TileMap* leftTilesAdjacent = getChunkTileMap(ChunkPosition(((chunkTileOffsets[i].first.x - 1) % worldSize + worldSize) % worldSize, chunkTileOffsets[i].first.y), tileMap);
        TileMap* rightTilesAdjacent = getChunkTileMap(ChunkPosition(((chunkTileOffsets[i].first.x + 1) % worldSize + worldSize) % worldSize, chunkTileOffsets[i].first.y), tileMap);

        // If adjacent tile type is higher / greater than tile updating for, tile must be set underneath adjacent tilewdasd
        if (adjacentTileType > tileMap)
        {
            getChunk(chunkTileOffsets[i].first)->setTile(tileMap, chunkTileOffsets[i].second, upTilesAdjacent, downTilesAdjacent, leftTilesAdjacent, rightTilesAdjacent);
        }
        else if (adjacentTileType < tileMap)
        {
            // Adjacent tile is lower / smaller than tile updating for, so must be set underneath the original tile
            getChunk(chunk)->setTile(adjacentTileType, position, upTiles, downTiles, leftTiles, rightTiles);
            tileMapsModified.insert(adjacentTileType);
        }
    }

    return tileMapsModified;
}

void ChunkManager::updateAdjacentChunkTiles(ChunkPosition chunk, int tileMap)
{
    // Update surrounding chunks tilemaps
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            // Skip corners
            if ((y == -1 || y == 1) && (x == -1 || x == 1))
                continue;

            int chunkX = chunk.x + x;
            int chunkY = chunk.y + y;

            if (chunk == ChunkPosition(chunkX, chunkY))
                continue;
            
            ChunkPosition wrappedChunk;
            wrappedChunk.x = (chunkX % worldSize + worldSize) % worldSize;
            wrappedChunk.y = (chunkY % worldSize + worldSize) % worldSize;

            Chunk* chunkPtr = getChunk(wrappedChunk);
            if (!chunkPtr)
                continue;

            TileMap* upTiles = getChunkTileMap(ChunkPosition(wrappedChunk.x, ((chunkY - 1) % worldSize + worldSize) % worldSize), tileMap);
            TileMap* downTiles = getChunkTileMap(ChunkPosition(wrappedChunk.x, ((chunkY + 1) % worldSize + worldSize) % worldSize), tileMap);
            TileMap* leftTiles = getChunkTileMap(ChunkPosition(((chunkX - 1) % worldSize + worldSize) % worldSize, wrappedChunk.y), tileMap);
            TileMap* rightTiles = getChunkTileMap(ChunkPosition(((chunkX + 1) % worldSize + worldSize) % worldSize, wrappedChunk.y), tileMap);

            chunkPtr->updateTileMap(tileMap, x, y, upTiles, downTiles, leftTiles, rightTiles);
        }
    }
}

void ChunkManager::updateAdjacentChunkAdjacentChunkTiles(ChunkPosition centreChunk, int tileMap)
{
    // Update adjacent chunk's adjacent chunks tilemaps
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            // Skip corner chunks
            if ((y == -1 || y == 1) && (x == -1 || x == 1))
                continue;

            int chunkX = centreChunk.x + x;
            int chunkY = centreChunk.y + y;

            if (centreChunk == ChunkPosition(chunkX, chunkY))
                continue;
            
            ChunkPosition wrappedChunk;
            wrappedChunk.x = (chunkX % worldSize + worldSize) % worldSize;
            wrappedChunk.y = (chunkY % worldSize + worldSize) % worldSize;

            if (!isChunkGenerated(wrappedChunk))
                continue;
            
            updateAdjacentChunkTiles(wrappedChunk, tileMap);
        }
    }
}

void ChunkManager::performChunkSetTileUpdate(ChunkPosition chunk, std::set<int> tileMapsModified)
{
    for (int tileMapModified : tileMapsModified)
    {
        updateAdjacentChunkTiles(chunk, tileMapModified);
        updateAdjacentChunkAdjacentChunkTiles(chunk, tileMapModified);
    }
}

BuildableObject* ChunkManager::getChunkObject(ChunkPosition chunk, sf::Vector2i tile)
{
    // // Empty object to return if chunk / object does not exist
    // static std::optional<BuildableObject> null = std::nullopt;

    Chunk* chunkPtr = getChunk(chunk);

    // Chunk does not exist
    if (!chunkPtr)
    {
        return nullptr;
    }
    
    // Get object from chunk
    BuildableObject* selectedObject = chunkPtr->getObject(sf::Vector2i(tile.x, tile.y));

    if (!selectedObject)
        return nullptr;

    // Test if object is object reference object, to then get the actual object
    if (selectedObject->isObjectReference())
    {
        const ObjectReference& objectReference = selectedObject->getObjectReference().value();
        return getChunkObject(objectReference.chunk, objectReference.tile);
    }

    // Return retrieved object
    return selectedObject;
}

int ChunkManager::getLoadedChunkTileType(ChunkPosition chunk, sf::Vector2i tile) const
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return 0;
    
    return loadedChunks.at(chunk)->getTileType(tile);
}

int ChunkManager::getChunkTileType(ChunkPosition chunk, sf::Vector2i tile) const
{
    // Chunk is not generated
    if (!isChunkGenerated(chunk))
        return 0;
    
    // Not in loaded chunks, go to stored chunks
    if (loadedChunks.count(chunk) <= 0)
        return storedChunks.at(chunk)->getTileType(tile);
    
    return loadedChunks.at(chunk)->getTileType(tile);
}

int ChunkManager::getChunkTileTypeOrPredicted(ChunkPosition chunk, sf::Vector2i tile)
{
    Chunk* chunkPtr = getChunk(chunk);

    if (chunkPtr)
    {
        // Chunk has been generated, so get tile from chunk
        return chunkPtr->getTileType(tile);
    }

    // Chunk has not been generated, so predict tile from proc gen
    sf::Vector2i worldTile;
    worldTile.x = chunk.x * CHUNK_TILE_SIZE + tile.x;
    worldTile.y = chunk.y * CHUNK_TILE_SIZE + tile.y;

    const TileGenData* tileGenData = Chunk::getTileGenAtWorldTile(worldTile, worldSize, heightNoise, biomeNoise, riverNoise, planetType);
    if (!tileGenData)
    {
        return 0;
    }

    return tileGenData->tileID;
}

bool ChunkManager::isChunkGenerated(ChunkPosition chunk) const
{
    return (loadedChunks.count(chunk) + storedChunks.count(chunk)) > 0;
}

void ChunkManager::setObject(ChunkPosition chunk, sf::Vector2i tile, ObjectType objectType, Game& game)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return;
    
    // Set chunk object at position
    loadedChunks[chunk]->setObject(tile, objectType, game, *this, &pathfindingEngine);
}

void ChunkManager::deleteObject(ChunkPosition chunk, sf::Vector2i tile)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return;
    
    loadedChunks[chunk]->deleteObject(tile, *this, pathfindingEngine);
}

void ChunkManager::deleteSingleObject(ChunkPosition chunk, sf::Vector2i tile)
{
    if (loadedChunks.count(chunk) <= 0)
        return;
    
    loadedChunks[chunk]->deleteSingleObject(tile, *this, pathfindingEngine);
}

void ChunkManager::setObjectReference(const ChunkPosition& chunk, const ObjectReference& objectReference, sf::Vector2i tile)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return;
    
    loadedChunks[chunk]->setObjectReference(objectReference, tile, *this, pathfindingEngine);
}

bool ChunkManager::canPlaceObject(ChunkPosition chunk, sf::Vector2i tile, ObjectType objectType, const CollisionRect& playerCollisionRect)
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

bool ChunkManager::canDestroyObject(ChunkPosition chunk, sf::Vector2i tile, const CollisionRect& playerCollisionRect)
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return false;
    
    BuildableObject* object = loadedChunks[chunk]->getObject(tile);

    if (!object)
        return false;

    if (object->getObjectType() < 0)
        return false;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(object->getObjectType());

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

void ChunkManager::updateChunksEntities(float dt, ProjectileManager& projectileManager, InventoryData& inventory, Game& game)
{
    for (auto& chunkPair : loadedChunks)
    {
        chunkPair.second->updateChunkEntities(dt, worldSize, projectileManager, inventory, *this, game);
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

bool ChunkManager::collisionRectChunkStaticCollisionX(CollisionRect& collisionRect, float dx) const
{
    bool collision = false;
    for (auto& chunkPair : loadedChunks)
    {
        if (chunkPair.second->collisionRectStaticCollisionX(collisionRect, dx))
            collision = true;
    }
    return collision;
}

bool ChunkManager::collisionRectChunkStaticCollisionY(CollisionRect& collisionRect, float dy) const
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

void ChunkManager::placeLand(ChunkPosition chunk, sf::Vector2i tile)
{
    // Chunk not loaded
    if (loadedChunks.count(chunk) <= 0)
        return;
    
    // Cannot place land
    if (!loadedChunks[chunk]->canPlaceLand(tile))
        return;
    
    // Place land and update visual tiles for chunk
    loadedChunks[chunk]->placeLand(tile, worldSize, heightNoise, biomeNoise, planetType, *this, pathfindingEngine);

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

            loadedChunks[ChunkPosition(wrappedX, wrappedY)]->generateVisualEffectTiles(*this);
        }
    }
}

bool ChunkManager::isPlayerInStructureEntrance(sf::Vector2f playerPos, StructureEnterEvent& enterEvent)
{
    for (auto chunkIter = loadedChunks.begin(); chunkIter != loadedChunks.end(); chunkIter++)
    {
        if (chunkIter->second->isPlayerInStructureEntrance(playerPos, enterEvent))
        {
            return true;
        }
    }

    return false;   
}

std::vector<ChunkPOD> ChunkManager::getChunkPODs()
{
    std::vector<ChunkPOD> pods;

    clearUnmodifiedStoredChunks();

    for (auto iter = storedChunks.begin(); iter != storedChunks.end(); ++iter)
    {
        pods.push_back(iter->second->getChunkPOD());
    }

    for (auto iter = loadedChunks.begin(); iter != loadedChunks.end(); ++iter)
    {
        pods.push_back(iter->second->getChunkPOD());
    }

    return pods;
}

void ChunkManager::loadFromChunkPODs(const std::vector<ChunkPOD>& pods, Game& game)
{
    deleteAllChunks();

    for (const ChunkPOD& pod : pods)
    {
        std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(pod.chunkPosition);
        chunk->loadFromChunkPOD(pod, game);

        storedChunks[pod.chunkPosition] = std::move(chunk);
    }
}

ChunkPosition ChunkManager::findValidSpawnChunk(int waterlessAreaSize)
{
    // Move all loaded chunks (if any) into stored chunks temporarily
    // Makes testing area simpler as only have to check stored chunk map
    // reloadChunks();

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

                    // Generate minimal version of chunk (tile grid and structure) to check against
                    Chunk chunk(ChunkPosition(wrappedX, wrappedY));
                    chunk.generateTilesAndStructure(heightNoise, biomeNoise, riverNoise, planetType, *this);

                    // Check against chunk
                    if (!chunk.getContainsWater())
                        continue;
                    
                   /* if (!chunk.hasStructure())
                        continue;*/

                    // If not generated, generate
                    // if (storedChunks.count(ChunkPosition(wrappedX, wrappedY)) <= 0)
                    //     generateChunk(ChunkPosition(wrappedX, wrappedY), false);
                    
                    // // If chunk does not contain water, continue to check other chunks
                    // if (!storedChunks[ChunkPosition(wrappedX, wrappedY)]->getContainsWater())
                    //     continue;
                    
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
            // sf::Vector2f spawnPosition;
            // spawnPosition.x = x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
            // spawnPosition.y = y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED + 0.5f * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;

            return ChunkPosition(x, y);
        }
    }

    // Recursive call to find smaller valid area
    if (waterlessAreaSize > 0)
        return findValidSpawnChunk(waterlessAreaSize - 1);
    
    // Can't find valid spawn, so default return
    return ChunkPosition(0, 0);
}

std::unordered_map<std::string, int> ChunkManager::getNearbyCraftingStationLevels(ChunkPosition playerChunk, sf::Vector2i playerTile, int searchArea)
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
            
            BuildableObject* object = getChunkObject(chunkTile.first, chunkTile.second);

            // If no object, do not get data
            if (!object)
                continue;
            
            // No need to test if object reference as getChunkObject handles this

            // Get object data
            ObjectType objectType = object->getObjectType();
            if (objectType < 0)
                continue;

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

void ChunkManager::generateChunk(const ChunkPosition& chunkPosition, Game& game, bool putInLoaded, std::optional<sf::Vector2f> positionOverride)
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

    Chunk* chunkPtr = nullptr;
    if (putInLoaded)
    {
        loadedChunks.emplace(chunkPosition, std::move(chunk));
        chunkPtr = loadedChunks[chunkPosition].get();
    }
    else
    {
        storedChunks.emplace(chunkPosition, std::move(chunk));
        chunkPtr = storedChunks[chunkPosition].get();
    }

    // Set chunk position
    chunkPtr->setWorldPosition(chunkWorldPos, *this);

    // Generate
    chunkPtr->generateChunk(heightNoise, biomeNoise, riverNoise, planetType, game, *this, pathfindingEngine);
}

void ChunkManager::clearUnmodifiedStoredChunks()
{
    for (auto iter = storedChunks.begin(); iter != storedChunks.end();)
    {
        if (!iter->second->hasBeenModified())
        {
            iter = storedChunks.erase(iter);
            continue;
        }
        iter++;
    }
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

sf::Vector2i ChunkManager::getChunksSizeInView(const Camera& camera)
{
    sf::Vector2f screenTopLeft = camera.screenToWorldTransform({0, 0});
    sf::Vector2f screenBottomRight = camera.screenToWorldTransform(static_cast<sf::Vector2f>(ResolutionHandler::getResolution()));

    // Convert screen bounds to chunk units
    sf::Vector2i screenTopLeftGrid;
    sf::Vector2i screenBottomRightGrid;

    screenTopLeftGrid.y = std::floor(screenTopLeft.y / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));
    screenTopLeftGrid.x = std::floor(screenTopLeft.x / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));
    screenBottomRightGrid.x = std::ceil(screenBottomRight.x / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));
    screenBottomRightGrid.y = std::ceil(screenBottomRight.y / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));

    sf::Vector2i chunkSizeInView;
    chunkSizeInView.x = screenBottomRightGrid.x + CHUNK_VIEW_LOAD_BORDER - (screenTopLeftGrid.x - CHUNK_VIEW_LOAD_BORDER);
    chunkSizeInView.y = screenBottomRightGrid.y + CHUNK_VIEW_LOAD_BORDER - (screenTopLeftGrid.y - CHUNK_VIEW_LOAD_BORDER);
    return chunkSizeInView;
}

sf::Vector2f ChunkManager::topLeftChunkPosInView(const Camera& camera)
{
    sf::Vector2f screenTopLeft = camera.screenToWorldTransform({0, 0});
    screenTopLeft.y = (std::floor(screenTopLeft.y / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE)) - CHUNK_VIEW_LOAD_BORDER) * TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE;
    screenTopLeft.x = (std::floor(screenTopLeft.x / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE)) - CHUNK_VIEW_LOAD_BORDER) * TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE;
    return screenTopLeft;
}