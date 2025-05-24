#include "World/ChunkManager.hpp"
#include "Network/NetworkHandler.hpp"
#include "Game.hpp"
#include "Player/Player.hpp"

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
    // waterShader->setUniform("waterColor", sf::Glsl::Vec4(planetGenData.waterColor));

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

    chunkBiomeCache.clear();
    chunkLastEntitySpawnTime.clear();
}

bool ChunkManager::updateChunks(Game& game, float gameTime, const std::vector<ChunkViewRange>& chunkViewRanges,
    bool isClient, std::vector<ChunkPosition>* chunksToRequestFromHost)
{
    // Chunk load/unload

    bool hasModifiedChunks = false;

    // Check any chunks needed to load
    for (ChunkPosition chunkPos : ChunkViewRange::getCombinedChunkSet(chunkViewRanges, worldSize))
    {
        // ChunkPosition chunkPos = iter.get(worldSize);
        
        // Chunk not loaded
        hasModifiedChunks = true;
    
        // Calculate chunk world pos
        // pl::Vector2f chunkWorldPos;
        // chunkWorldPos.x = chunkPos.x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
        // chunkWorldPos.y = chunkPos.y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
        
        // Chunk already loaded
        if (loadedChunks.count(chunkPos))
        {
            // Chunk already in correct world position, skip
            // if (loadedChunks.at(chunkPos)->getWorldPosition() == chunkWorldPos)
            // {
            //     continue;
            // }
            // else
            // {
                // Chunk is loaded but in incorrect position, reload
                // storedChunks[chunkPos] = std::move(loadedChunks[chunkPos]);
                // loadedChunks.erase(chunkPos);
            // }
            continue;
        }
    
        // Check if chunk is in memory, and load if so
        if (storedChunks.count(chunkPos))
        {
            // Move chunk into loaded chunks for rendering
            loadedChunks[chunkPos] = std::move(storedChunks[chunkPos]);
            storedChunks.erase(chunkPos);
    
            auto& chunk = loadedChunks[chunkPos];
    
            // Update chunk position
            // chunk->setWorldPosition(chunkWorldPos, *this);
    
            // If chunk was loaded through POD / save file, has not yet been initialised (tilemaps, collision, pathfinding etc)
            // Therefore must initialise
            if (chunk->wasGeneratedFromPOD())
            {
                chunk->generateTilemapsAndInit(*this, pathfindingEngine);
            }
            else
            {
                // Spawn entities if enough time has passed
    
                // Only attempt if chunk was not generated / loaded from POD, otherwise each time save is loaded
                // more entities will spawn in player view (meaning previous state is not retained from player perspective)
    
                if (getChunkEntitySpawnCooldown(chunkPos) >= MAX_CHUNK_ENTITY_SPAWN_COOLDOWN)
                {
                    chunk->spawnChunkEntities(worldSize, heightNoise, biomeNoise, riverNoise, planetType);
                    resetChunkEntitySpawnCooldown(chunkPos);
                }
            }
    
            continue;
        }

        if (isClient && chunksToRequestFromHost != nullptr)
        {
            chunksToRequestFromHost->push_back(chunkPos);
            continue;
        }
    
        // Generate new chunk if does not exist (only if host / solo)
        generateChunk(chunkPos, game, gameTime, true);
    }

    return hasModifiedChunks;
}

bool ChunkManager::unloadChunksOutOfView(const std::vector<ChunkViewRange>& chunkViewRanges)
{
    bool hasUnloadedChunks = false;

    // Check any loaded chunks need unloading
    for (auto iter = loadedChunks.begin(); iter != loadedChunks.end();)
    {
        ChunkPosition chunkPos = iter->first;

        // Must not be in any chunk view range
        bool isInRange = false;
        for (const ChunkViewRange& chunkViewRange : chunkViewRanges)
        {
            if (chunkViewRange.isChunkInRange(chunkPos, worldSize))
            {
                isInRange = true;
                break;
            }
        }

        if (isInRange)
        {
            iter++;
            continue;
        }

        // If chunk is not visible, unload chunk
        
        // If chunk has been modified, store it
        if (iter->second->hasBeenModified())
        {
            // Store chunk in chunk memory
            storedChunks[chunkPos] = std::move(iter->second);
        }
        
        // Unload / delete chunk
        iter = loadedChunks.erase(iter);
        
        hasUnloadedChunks = true;
    }

    return hasUnloadedChunks;
}

// void ChunkManager::reloadChunks(ChunkViewRange chunkViewRange)
// {
//     for (auto iter = loadedChunks.begin(); iter != loadedChunks.end();)
//     {
//         ChunkPosition chunkPos = iter->first;
        
//         // Store chunk in chunk memory
//         storedChunks[chunkPos] = std::move(iter->second);

//         if (chunkViewRange.isChunkInRange(chunkPos, worldSize))
//         {
//             // Reset chunk entity spawn cooldown to prevent chunks spawning new entities when reloaded
//             // e.g. when world boundary crossed, prevent new entities from spawning on loaded chunks
//             resetChunkEntitySpawnCooldown(chunkPos);
//         }

//         // Unload chunk
//         iter = loadedChunks.erase(iter);
//     }
// }

void ChunkManager::regenerateChunkWithStructureType(ChunkPosition chunk, Game& game, std::optional<StructureType> structureType)
{
    Chunk* chunkPtr = getChunk(chunk);

    if (!chunkPtr)
    {
        storedChunks[chunk] = std::make_unique<Chunk>(chunk, game.getGameTime());
        chunkPtr = storedChunks[chunk].get();
    }

    // Full reset of chunk
    chunkPtr->reset(true);

    // Regenerate without structure
    chunkPtr->generateChunk(heightNoise, biomeNoise, riverNoise, planetType, game, *this, pathfindingEngine, structureType.has_value(), structureType);
}

void ChunkManager::drawChunkTerrain(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, float time)
{
    ChunkViewRange chunkViewRange = camera.getChunkViewDrawRange();
    
    // Draw terrain
    for (auto iter = chunkViewRange.begin(); iter != chunkViewRange.end(); iter++)
    {
        ChunkPosition chunkPos = iter.get(worldSize);
        
        if (!loadedChunks.contains(chunkPos))
        {
            continue;
        }
        
        loadedChunks[chunkPos]->drawChunkTerrain(window, camera, time, worldSize);
    }

    // Draw visual terrain features e.g. cliffs
    for (auto iter = chunkViewRange.begin(); iter != chunkViewRange.end(); iter++)
    {
        ChunkPosition chunkPos = iter.get(worldSize);
        
        if (!loadedChunks.contains(chunkPos))
        {
            continue;
        }
        
        loadedChunks[chunkPos]->drawChunkTerrainVisual(window, spriteBatch, camera, planetType, worldSize, time);
    }
}

void ChunkManager::drawChunkWater(pl::RenderTarget& window, const Camera& camera, float time)
{
    pl::Texture* waterTexture = TextureManager::getTexture(TextureType::Water);
    const PlanetGenData& planetGenData = PlanetGenDataLoader::getPlanetGenData(planetType);

    // Set shader time paramenter
    pl::Shader* waterShader = Shaders::getShader(ShaderType::Water);
    waterShader->setUniform1f("time", time);
    waterShader->setUniform2f("spriteSheetSize", waterTexture->getWidth(), waterTexture->getHeight());
    waterShader->setUniform4f("textureRect", planetGenData.waterTextureOffset.x, planetGenData.waterTextureOffset.y, 32, 32);

    ChunkViewRange chunkViewRange = camera.getChunkViewDrawRange();

    for (auto iter = chunkViewRange.begin(); iter != chunkViewRange.end(); iter++)
    {
        ChunkPosition chunkPos = iter.get(worldSize);
        
        if (!loadedChunks.contains(chunkPos))
        {
            continue;
        }
        
        loadedChunks[chunkPos]->drawChunkWater(window, camera, *this);
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

std::vector<ChunkPosition> ChunkManager::updateChunksObjects(Game& game, float dt, float gameTime)
{
    std::vector<ChunkPosition> chunksModified;

    for (auto& chunkPair : loadedChunks)
    {
        bool modified = chunkPair.second->updateChunkObjects(game, dt, gameTime, worldSize, *this, pathfindingEngine);
        if (modified)
        {
            chunksModified.push_back(chunkPair.first);
        }
    }

    return chunksModified;
}

TileMap* ChunkManager::getChunkTileMap(ChunkPosition chunk, int tileMap)
{
    // Not in loaded chunks, go to stored chunks
    Chunk* chunkPtr = getChunk(chunk);
    if (!chunkPtr)
    {
        return nullptr;
    }

    return chunkPtr->getTileMap(tileMap);
}

std::set<int> ChunkManager::setChunkTile(ChunkPosition chunk, int tileMap, pl::Vector2<int> position, bool tileGraphicUpdate)
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

std::set<int> ChunkManager::setBackgroundAdjacentTilesForTile(ChunkPosition chunk, int tileMap, pl::Vector2<int> position)
{
    // Update surrounding tiles for "underneath" tile placements
    std::pair<ChunkPosition, pl::Vector2<int>> chunkTileOffsets[4] = {
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

int ChunkManager::getLoadedChunkTileType(ChunkPosition chunk, pl::Vector2<int> tile) const
{
    // Chunk does not exist
    if (loadedChunks.count(chunk) <= 0)
        return 0;
    
    return loadedChunks.at(chunk)->getTileType(tile);
}

int ChunkManager::getChunkTileType(ChunkPosition chunk, pl::Vector2<int> tile) const
{
    // Chunk is not generated
    if (!isChunkGenerated(chunk))
        return 0;
    
    // Not in loaded chunks, go to stored chunks
    if (loadedChunks.count(chunk) <= 0)
        return storedChunks.at(chunk)->getTileType(tile);
    
    return loadedChunks.at(chunk)->getTileType(tile);
}

int ChunkManager::getChunkTileTypeOrPredicted(ChunkPosition chunk, pl::Vector2<int> tile)
{
    Chunk* chunkPtr = getChunk(chunk);

    if (chunkPtr)
    {
        // Chunk has been generated, so get tile from chunk
        return chunkPtr->getTileType(tile);
    }

    // Chunk has not been generated, so predict tile from proc gen
    pl::Vector2<int> worldTile;
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

const BiomeGenData* ChunkManager::getChunkBiome(ChunkPosition chunk)
{
    if (chunkBiomeCache.contains(chunk))
    {
        return chunkBiomeCache.at(chunk);
    }

    // Biome for chunk has not been stored
    // Get chunk biome using centre chunk tile
    pl::Vector2<int> chunkTopLeft((chunk.x + 0.5f) * CHUNK_TILE_SIZE, (chunk.y + 0.5f) * CHUNK_TILE_SIZE);

    const BiomeGenData* biomeGenData = Chunk::getBiomeGenAtWorldTile(chunkTopLeft, worldSize, biomeNoise, planetType);

    if (biomeGenData == nullptr)
    {
        return nullptr;
    }

    // Store in cache
    chunkBiomeCache[chunk] = biomeGenData;

    return biomeGenData;
}

void ChunkManager::setObject(ChunkPosition chunk, pl::Vector2<int> tile, ObjectType objectType, Game& game, const BuildableObjectCreateParameters& parameters)
{
    Chunk* chunkPtr = getChunk(chunk);
    // Chunk does not exist
    if (!chunkPtr)
    {
        return;
    }

    // Set chunk object at position
    chunkPtr->setObject(tile, objectType, game, *this, &pathfindingEngine, parameters);
}

void ChunkManager::deleteObject(ChunkPosition chunk, pl::Vector2<int> tile, Game& game, bool dropItems)
{
    // Chunk does not exist
    Chunk* chunkPtr = getChunk(chunk);
    if (!chunkPtr)
    {
        return;
    }
    
    chunkPtr->deleteObject(tile, game, *this, pathfindingEngine, dropItems);
}

void ChunkManager::deleteSingleObject(ChunkPosition chunk, pl::Vector2<int> tile)
{
    Chunk* chunkPtr = getChunk(chunk);
    if (!chunkPtr)
    {
        return;
    }
    
    chunkPtr->deleteSingleObject(tile, *this, pathfindingEngine);
}

void ChunkManager::setObjectReference(const ChunkPosition& chunk, const ObjectReference& objectReference, pl::Vector2<int> tile)
{
    // Chunk does not exist
    Chunk* chunkPtr = getChunk(chunk);
    if (!chunkPtr)
    {
        return;
    }
    
    chunkPtr->setObjectReference(objectReference, tile, *this, pathfindingEngine);
}

bool ChunkManager::canPlaceObject(ChunkPosition chunk, pl::Vector2<int> tile, ObjectType objectType, const std::vector<Player*>& players)
{
    // Chunk does not exist
    Chunk* chunkPtr = getChunk(chunk);
    if (!chunkPtr)
    {
        return false;
    }

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    if (objectData.hasCollision)
    {
        // Create collision rect for object using world position
        pl::Vector2f chunkWorldPosition = loadedChunks[chunk]->getWorldPosition();

        CollisionRect objectCollisionRect;
        objectCollisionRect.x = chunkWorldPosition.x + tile.x * TILE_SIZE_PIXELS_UNSCALED;
        objectCollisionRect.y = chunkWorldPosition.y + tile.y * TILE_SIZE_PIXELS_UNSCALED;
        objectCollisionRect.width = objectData.size.x * TILE_SIZE_PIXELS_UNSCALED;
        objectCollisionRect.height = objectData.size.y * TILE_SIZE_PIXELS_UNSCALED;

        // Test if colliding with players
        for (Player* player : players)
        {
            if (player->getCollisionRect().isColliding(objectCollisionRect, worldSize))
            {
                return false;
            }
        }
        
        // Test if colliding with entities in adjacent chunks
        for (int y = chunk.y - 1; y <= chunk.y + 1; y++)
        {
            for (int x = chunk.x - 1; x <= chunk.x + 1; x++)
            {
                int wrappedX = (x % worldSize + worldSize) % worldSize;
                int wrappedY = (y % worldSize + worldSize) % worldSize;

                // If chunk does not exist, do not attempt to check collision
                if (Chunk* chunkPtr = getChunk(ChunkPosition(wrappedX, wrappedY)))
                {
                    if (chunkPtr->isCollisionRectCollidingWithEntities(objectCollisionRect, worldSize))
                    {
                        return false;
                    }
                }
            }
        }
    }

    // If not colliding with player / entities, test whether colliding with other objects
    return chunkPtr->canPlaceObject(tile, objectType, worldSize, *this);
}

bool ChunkManager::canDestroyObject(ChunkPosition chunk, pl::Vector2<int> tile, const std::vector<Player*>& players)
{
    Chunk* chunkPtr = getChunk(chunk);

    // Chunk does not exist
    if (!chunkPtr)
    {
        return false;
    }
    
    BuildableObject* object = chunkPtr->getObject(tile);

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
        pl::Vector2f chunkWorldPosition = chunkPtr->getWorldPosition();

        CollisionRect objectCollisionRect;
        objectCollisionRect.x = chunkWorldPosition.x + tile.x * TILE_SIZE_PIXELS_UNSCALED;
        objectCollisionRect.y = chunkWorldPosition.y + tile.y * TILE_SIZE_PIXELS_UNSCALED;
        objectCollisionRect.width = objectData.size.x * TILE_SIZE_PIXELS_UNSCALED;
        objectCollisionRect.height = objectData.size.y * TILE_SIZE_PIXELS_UNSCALED;

        // Test if colliding with players
        for (Player* player : players)
        {
            if (player->getCollisionRect().isColliding(objectCollisionRect, worldSize))
            {
                return false;
            }
        }
        
        // Test if colliding with entities in adjacent chunks
        for (int x = chunk.x - 1; x <= chunk.x + 1; x++)
        {
            for (int y = chunk.y - 1; y <= chunk.y + 1; y++)
            {
                int wrappedX = (x % worldSize + worldSize) % worldSize;
                int wrappedY = (y % worldSize + worldSize) % worldSize;
                
                // If chunk does not exist, do not attempt to check collision
                if (Chunk* chunkPtr = getChunk(ChunkPosition(wrappedX, wrappedY)))
                {
                    if (chunkPtr->isCollisionRectCollidingWithEntities(objectCollisionRect, worldSize))
                    {
                        return false;
                    }
                }
            }
        }
    }

    // Default case
    return true;
}

std::vector<WorldObject*> ChunkManager::getChunkObjects(ChunkViewRange chunkViewRange)
{
    std::vector<WorldObject*> objects;
    for (auto iter = chunkViewRange.begin(); iter != chunkViewRange.end(); iter++)
    {
        ChunkPosition chunkPos = iter.get(worldSize);
        
        if (!loadedChunks.contains(chunkPos))
        {
            continue;
        }

        std::vector<WorldObject*> chunkObjects = loadedChunks[chunkPos]->getObjects();
        objects.insert(objects.end(), chunkObjects.begin(), chunkObjects.end());
    }
    return objects;
}

void ChunkManager::updateChunksEntities(float dt, ProjectileManager& projectileManager, Game& game, bool networkUpdateOnly)
{
    for (auto& chunkPair : loadedChunks)
    {
        chunkPair.second->updateChunkEntities(dt, worldSize, &projectileManager, *this, &game, networkUpdateOnly);
    }
}

void ChunkManager::testChunkEntityHitCollision(const std::vector<HitRect>& hitRects, Game& game, float gameTime)
{
    for (auto& chunkPair : loadedChunks)
    {
        chunkPair.second->testEntityHitCollision(hitRects, *this, game, gameTime);
    }
}

void ChunkManager::moveEntityToChunkFromChunk(std::unique_ptr<Entity> entity, ChunkPosition newChunk)
{
    if (loadedChunks.count(newChunk) <= 0)
        return;
    
    loadedChunks[newChunk]->moveEntityToChunk(std::move(entity));
}

Entity* ChunkManager::getSelectedEntity(ChunkPosition chunk, pl::Vector2f cursorPos)
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

std::vector<WorldObject*> ChunkManager::getChunkEntities(ChunkViewRange chunkViewRange)
{
    std::vector<WorldObject*> entities;
    for (auto iter = chunkViewRange.begin(); iter != chunkViewRange.end(); iter++)
    {
        ChunkPosition chunkPos = iter.get(worldSize);

        if (!loadedChunks.contains(chunkPos))
        {
            continue;
        }

        std::vector<WorldObject*> chunkEntities = loadedChunks[chunkPos]->getEntities();
        entities.insert(entities.end(), chunkEntities.begin(), chunkEntities.end());
    }
    return entities;
}

int ChunkManager::getChunkEntitySpawnCooldown(ChunkPosition chunk)
{
    uint64_t time = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);

    if (!chunkLastEntitySpawnTime.contains(chunk))
    {
        chunkLastEntitySpawnTime[chunk] = time;
    }

    return (time - chunkLastEntitySpawnTime.at(chunk));
}

void ChunkManager::resetChunkEntitySpawnCooldown(ChunkPosition chunk)
{
    uint64_t time = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
    chunkLastEntitySpawnTime[chunk] = time;
}

PacketDataEntities ChunkManager::getEntityPacketDatas(ChunkViewRange chunkViewRange)
{
    PacketDataEntities entityPacketData;
    entityPacketData.planetType = planetType;

    for (auto iter = chunkViewRange.begin(); iter != chunkViewRange.end(); iter++)
    {
        ChunkPosition chunkPos = iter.get(worldSize);
        if (!loadedChunks.contains(chunkPos))
        {
            continue;
        }
        
        std::vector<PacketDataEntities::EntityPacketData> packetEntities = loadedChunks[chunkPos]->getEntityPacketDatas();
        if (packetEntities.size() > 0)
        {
            entityPacketData.entities.insert(entityPacketData.entities.end(), packetEntities.begin(), packetEntities.end());
        }
    }

    return entityPacketData;
}

void ChunkManager::loadEntityPacketDatas(const PacketDataEntities& entityPacketDatas)
{
    if (entityPacketDatas.planetType != planetType)
    {
        printf(("ERROR: Received entity packet for incorrect planet type " + std::to_string(entityPacketDatas.planetType) + "\n").c_str());
        return;
    }

    // Clear loaded chunk entities
    for (auto& chunk : loadedChunks)
    {
        chunk.second->clearEntities();
    }

    for (const auto& entityPacketData : entityPacketDatas.entities)
    {
        if (!loadedChunks.contains(entityPacketData.chunkPosition))
        {
            continue;
        }

        loadedChunks[entityPacketData.chunkPosition]->loadEntityPacketData(entityPacketData);

        // Update entities with ping time
        // loadedChunks[chunkEntityData.first]->updateChunkEntities(entityPacketDatas.pingTime, worldSize, nullptr, *this, nullptr, true);
    }
}

std::optional<ItemPickupReference> ChunkManager::addItemPickup(const ItemPickup& itemPickup, NetworkHandler* networkHandler, std::optional<uint64_t> idOverride)
{
    if (itemPickup.getItemCount() <= 0)
    {
        return std::nullopt;
    }

    Chunk* chunkPtr = getChunk(itemPickup.getChunkInside(worldSize));

    if (!chunkPtr)
    {
        return std::nullopt;
    }

    // Request from host if is client
    if (networkHandler && networkHandler->isClient())
    {
        PacketDataItemPickupsCreateRequest packetData;
        packetData.locationState = LocationState::createFromPlanetType(planetType);

        ItemPickupRequest request;
        request.chunk = itemPickup.getChunkInside(worldSize);
        request.itemType = itemPickup.getItemType();
        request.count = itemPickup.getItemCount();
        request.positionRelative = itemPickup.getPosition() - chunkPtr->getWorldPosition();

        packetData.pickupRequests.push_back(request);

        Packet packet;
        packet.set(packetData);
        networkHandler->sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);

        return std::nullopt;
    }
    
    // Create item pickup
    ItemPickupReference itemPickupReference{chunkPtr->getChunkPosition(), chunkPtr->addItemPickup(itemPickup, idOverride)};
    
    if (networkHandler && networkHandler->getIsLobbyHost())
    {
        // Alert clients of item pickup created
        PacketDataItemPickupsCreated packetData;
        packetData.locationState = LocationState::createFromPlanetType(planetType);

        if (!chunkPtr)
        {
            std::cout << "ERROR: Attempted to send item pickup creation data for null chunk (" << itemPickupReference.chunk.x
                << ", " << itemPickupReference.chunk.y << ")\n";
            return std::nullopt;
        }

        const ItemPickup* itemPickupPtr = chunkPtr->getItemPickup(itemPickupReference.id);
        if (!itemPickupPtr)
        {
            std::cout << "ERROR: Attempted to send item pickup creation data for null pickup ID " << itemPickupReference.id << "\n";
            return std::nullopt;
        }

        ItemPickup itemPickup = *itemPickupPtr;

        // Normalise item pickup position relative to chunk before sending over network
        itemPickup.setPosition(itemPickup.getPosition() - chunkPtr->getWorldPosition());
        
        packetData.createdPickups.push_back({itemPickupReference, itemPickup});

        Packet packet;
        packet.set(packetData);
        networkHandler->sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
    }

    return itemPickupReference;
}

std::optional<ItemPickupReference> ChunkManager::getCollidingItemPickup(const CollisionRect& playerCollision, float gameTime)
{
    // Get chunk player is in
    ChunkPosition chunk = WorldObject::getChunkInside(pl::Vector2f(playerCollision.x, playerCollision.y), worldSize);

    // Check for pickups colliding in 3x3 chunk area around player
    for (int x = chunk.x - 1; x <= chunk.x + 1; x++)
    {
        for (int y = chunk.y - 1; y <= chunk.y + 1; y++)
        {
            Chunk* chunkPtr = getChunk(ChunkPosition(Helper::wrap(x, worldSize), Helper::wrap(y, worldSize)));

            if (chunkPtr == nullptr)
            {
                continue;
            }

            std::optional<ItemPickupReference> pickupCollided = chunkPtr->getCollidingItemPickup(playerCollision, gameTime, worldSize);

            if (pickupCollided.has_value())
            {
                return pickupCollided;
            }
        }
    }

    return std::nullopt;
}

void ChunkManager::reduceItemPickupCount(const ItemPickupReference& itemPickupReference, int count)
{
    Chunk* chunkPtr = getChunk(itemPickupReference.chunk);

    if (!chunkPtr)
    {
        return;
    }

    ItemPickup* itemPickupPtr = chunkPtr->getItemPickup(itemPickupReference.id);

    if (!itemPickupPtr)
    {
        return;
    }

    int newCount = static_cast<int>(itemPickupPtr->getItemCount()) - count;

    if (newCount <= 0)
    {
        chunkPtr->deleteItemPickup(itemPickupReference.id);
    }
    else
    {
        itemPickupPtr->setItemCount(newCount);
    }
}

std::vector<WorldObject*> ChunkManager::getItemPickups(ChunkViewRange chunkViewRange)
{
    std::vector<WorldObject*> itemPickupWorldObjects;

    for (auto iter = chunkViewRange.begin(); iter != chunkViewRange.end(); iter++)
    {
        ChunkPosition chunkPos = iter.get(worldSize);

        if (!loadedChunks.contains(chunkPos))
        {
            continue;
        }

        std::vector<WorldObject*> itemPickupChunkWorldObjects = loadedChunks[chunkPos]->getItemPickups();

        itemPickupWorldObjects.insert(itemPickupWorldObjects.end(), itemPickupChunkWorldObjects.begin(), itemPickupChunkWorldObjects.end());
    }

    return itemPickupWorldObjects;
}

bool ChunkManager::collisionRectChunkStaticCollisionX(CollisionRect& collisionRect, float dx) const
{
    bool collision = false;

    ChunkPosition centreChunk = WorldObject::getChunkInside(pl::Vector2f(collisionRect.x, collisionRect.y), worldSize);
    for (int y = centreChunk.y - 1; y <= centreChunk.y + 1; y++)
    {
        for (int x = centreChunk.x - 1; x <= centreChunk.x + 1; x++)
        {
            ChunkPosition wrappedChunk;
            wrappedChunk.x = Helper::wrap(x, worldSize);
            wrappedChunk.y = Helper::wrap(y, worldSize);

            if (!loadedChunks.contains(wrappedChunk))
            {
                continue;
            }

            if (loadedChunks.at(wrappedChunk)->collisionRectStaticCollisionX(collisionRect, dx, worldSize))
            {
                collision = true;
            }
        }
    }
    
    return collision;
}

bool ChunkManager::collisionRectChunkStaticCollisionY(CollisionRect& collisionRect, float dy) const
{
    bool collision = false;

    ChunkPosition centreChunk = WorldObject::getChunkInside(pl::Vector2f(collisionRect.x, collisionRect.y), worldSize);
    for (int y = centreChunk.y - 1; y <= centreChunk.y + 1; y++)
    {
        for (int x = centreChunk.x - 1; x <= centreChunk.x + 1; x++)
        {
            ChunkPosition wrappedChunk;
            wrappedChunk.x = Helper::wrap(x, worldSize);
            wrappedChunk.y = Helper::wrap(y, worldSize);

            if (!loadedChunks.contains(wrappedChunk))
            {
                continue;
            }

            if (loadedChunks.at(wrappedChunk)->collisionRectStaticCollisionY(collisionRect, dy, worldSize))
            {
                collision = true;
            }
        }
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

bool ChunkManager::canPlaceLand(ChunkPosition chunk, pl::Vector2<int> tile)
{
    // Chunk not loaded
    Chunk* chunkPtr = getChunk(chunk);
    if (!chunkPtr)
    {
        return false;
    }
    
    return chunkPtr->canPlaceLand(tile);
}

void ChunkManager::placeLand(ChunkPosition chunk, pl::Vector2<int> tile, NetworkHandler* networkHandler)
{
    if (networkHandler && networkHandler->isMultiplayerGame())
    {
        PacketDataLandPlaced packetData;
        packetData.planetType = planetType;
        packetData.chunk = chunk;
        packetData.tile = tile;

        Packet packet(packetData);

        if (networkHandler->getIsLobbyHost())
        {
            networkHandler->sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);
        }
        else
        {
            networkHandler->sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
            return;
        }
    }

    // Chunk not loaded
    Chunk* chunkPtr = getChunk(chunk);
    if (!chunkPtr)
    {
        return;
    }
    
    // Cannot place land
    if (!chunkPtr->canPlaceLand(tile))
        return;
    
    // Place land and update visual tiles for chunk
    chunkPtr->placeLand(tile, worldSize, heightNoise, biomeNoise, planetType, *this, pathfindingEngine);

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

std::optional<ChunkPosition> ChunkManager::isPlayerInStructureEntrance(pl::Vector2f playerPos)
{
    for (auto chunkIter = loadedChunks.begin(); chunkIter != loadedChunks.end(); chunkIter++)
    {
        if (chunkIter->second->isPlayerInStructureEntrance(playerPos))
        {
            return chunkIter->first;
        }
    }

    return std::nullopt;
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
        std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(pod.chunkPosition, 0.0f);
        chunk->loadFromChunkPOD(pod, game, *this);

        storedChunks[pod.chunkPosition] = std::move(chunk);
    }
}

// -- Networking --

PacketDataChunkDatas::ChunkData ChunkManager::getChunkDataAndGenerate(ChunkPosition chunk, Game& game)
{
    Chunk* chunkPtr = getChunk(chunk);
    
    // Not generated - generate before getting chunk data
    if (!chunkPtr)
    {
        chunkPtr = generateChunk(chunk, game, false);
    }

    ChunkPOD chunkPODNoEntities = chunkPtr->getChunkPOD(false);

    PacketDataChunkDatas::ChunkData chunkData;
    chunkData.setFromPOD(chunkPODNoEntities);

    // Get item pickups
    chunkData.itemPickupsRelative = chunkPtr->getItemPickupsMap();

    // Normalise item pickup positions to chunk-relative
    for (auto& itemPickup : chunkData.itemPickupsRelative)
    {
        itemPickup.second.setPosition(itemPickup.second.getPosition() - chunkPtr->getWorldPosition());
    }

    return chunkData;
}

void ChunkManager::setChunkData(const PacketDataChunkDatas::ChunkData& chunkData, Game& game)
{
    Chunk* chunkPtr = getChunk(chunkData.chunkPosition);

    bool newObjectFlash = true;

    // Chunk does not exist - create blank chunk
    if (!chunkPtr)
    {
        std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(chunkData.chunkPosition, 0.0f);
        chunkPtr = chunk.get();
        storedChunks[chunkData.chunkPosition] = std::move(chunk);

        // Chunk data is new - objects are not "new" so don't flash
        newObjectFlash = false;
    }
    
    chunkPtr->loadFromChunkPOD(chunkData.createPOD(), game, *this, newObjectFlash);

    std::unordered_map<uint64_t, ItemPickup> itemPickups = chunkData.itemPickupsRelative;

    // Normalise item pickup positions to world-relative
    for (auto& itemPickup : itemPickups)
    {
        itemPickup.second.setPosition(itemPickup.second.getPosition() + chunkPtr->getWorldPosition());
    }

    chunkPtr->overwriteItemPickupsMap(itemPickups);
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
                    Chunk chunk(ChunkPosition(wrappedX, wrappedY), 0);
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
            // pl::Vector2f spawnPosition;
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

std::unordered_map<std::string, int> ChunkManager::getNearbyCraftingStationLevels(ChunkPosition playerChunk, pl::Vector2<int> playerTile, int searchArea)
{
    std::unordered_map<std::string, int> craftingStationLevels;

    // Search area
    for (int xOffset = -searchArea; xOffset <= searchArea; xOffset++)
    {
        for (int yOffset = -searchArea; yOffset <= searchArea; yOffset++)
        {
            // Get chunk and tile
            std::pair<ChunkPosition, pl::Vector2<int>> chunkTile = getChunkTileFromOffset(playerChunk, playerTile, xOffset, yOffset, worldSize);

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

// pl::Vector2f ChunkManager::translatePositionAroundWorld(pl::Vector2f position, pl::Vector2f originPosition) const
// {
//     return translatePositionAroundWorld(position, originPosition, worldSize);
// }

// pl::Vector2f ChunkManager::translatePositionAroundWorld(pl::Vector2f position, pl::Vector2f originPosition, int worldSize)
// {
//     int worldPixelSize = worldSize * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
//     float halfWorldPixelSize = worldPixelSize / 2.0f;

//     if (std::abs(originPosition.x - position.x) >= halfWorldPixelSize)
//     {
//         if (originPosition.x >= halfWorldPixelSize)
//         {
//             if (position.x < halfWorldPixelSize)
//             {
//                 position.x += worldPixelSize;
//             }
//         }
//         else
//         {
//             if (position.x >= halfWorldPixelSize)
//             {
//                 position.x -= worldPixelSize;
//             }
//         }
//     }

//     if (std::abs(originPosition.y - position.y) >= halfWorldPixelSize)
//     {
//         if (originPosition.y >= halfWorldPixelSize)
//         {
//             if (position.y < halfWorldPixelSize)
//             {
//                 position.y += worldPixelSize;
//             }
//         }
//         else
//         {
//             if (position.y >= halfWorldPixelSize)
//             {
//                 position.y -= worldPixelSize;
//             }
//         }
//     }

//     return position;
// }

Chunk* ChunkManager::generateChunk(const ChunkPosition& chunkPosition, Game& game, float gameTime, bool putInLoaded)
{
    std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(chunkPosition, gameTime);

    // pl::Vector2f chunkWorldPos;
    // chunkWorldPos.x = chunkPosition.x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;
    // chunkWorldPos.y = chunkPosition.y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;

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
    // chunkPtr->setWorldPosition(chunkWorldPos, *this);

    resetChunkEntitySpawnCooldown(chunkPosition);

    bool initialiseChunk = putInLoaded;

    // Generate
    chunkPtr->generateChunk(heightNoise, biomeNoise, riverNoise, planetType, game, *this, pathfindingEngine, true, std::nullopt, true, initialiseChunk);

    return chunkPtr;
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
std::pair<ChunkPosition, pl::Vector2<int>> ChunkManager::getChunkTileFromOffset(ChunkPosition chunk, pl::Vector2<int> tile, int xOffset, int yOffset, int worldSize)
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

pl::Vector2<int> ChunkManager::getChunksSizeInView(const Camera& camera)
{
    pl::Vector2f screenTopLeft = camera.screenToWorldTransform({0, 0}, 0);
    pl::Vector2f screenBottomRight = camera.screenToWorldTransform(static_cast<pl::Vector2f>(ResolutionHandler::getResolution()), 0);

    // Convert screen bounds to chunk units
    pl::Vector2<int> screenTopLeftGrid;
    pl::Vector2<int> screenBottomRightGrid;

    screenTopLeftGrid.y = std::floor(screenTopLeft.y / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));
    screenTopLeftGrid.x = std::floor(screenTopLeft.x / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));
    screenBottomRightGrid.x = std::ceil(screenBottomRight.x / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));
    screenBottomRightGrid.y = std::ceil(screenBottomRight.y / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE));

    pl::Vector2<int> chunkSizeInView;
    chunkSizeInView.x = screenBottomRightGrid.x + CHUNK_VIEW_LOAD_BORDER - (screenTopLeftGrid.x - CHUNK_VIEW_LOAD_BORDER);
    chunkSizeInView.y = screenBottomRightGrid.y + CHUNK_VIEW_LOAD_BORDER - (screenTopLeftGrid.y - CHUNK_VIEW_LOAD_BORDER);
    return chunkSizeInView;
}

pl::Vector2f ChunkManager::topLeftChunkPosInView(const Camera& camera)
{
    pl::Vector2f screenTopLeft = camera.screenToWorldTransform({0, 0}, 0);
    screenTopLeft.y = (std::floor(screenTopLeft.y / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE)) - CHUNK_VIEW_LOAD_BORDER) * TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE;
    screenTopLeft.x = (std::floor(screenTopLeft.x / (TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE)) - CHUNK_VIEW_LOAD_BORDER) * TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE;
    return screenTopLeft;
}