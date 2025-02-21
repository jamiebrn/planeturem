#include "World/Chunk.hpp"
#include "Game.hpp"

Chunk::Chunk(ChunkPosition chunkPosition)
{
    this->chunkPosition = chunkPosition;

    itemPickupCounter = 0;

    reset();
}

void Chunk::reset(bool fullReset)
{
    containsWater = false;
    modified = false;

    for (int i = 0; i < groundTileGrid.size(); i++)
    {
        groundTileGrid[i].fill(0);
    }

    for (int i = 0; i < visualTileGrid.size(); i++)
    {
        visualTileGrid[i].fill(TileType::Visual_BLANK);
    }

    if (!fullReset)
    {
        return;
    }

    // Full reset
    collisionRects.clear();
    tileMaps.clear();
    entities.clear();
    structureObject = std::nullopt;

    for (int y = 0; y < objectGrid.size(); y++)
    {
        for (int x = 0; x < objectGrid[y].size(); x++)
        {
            objectGrid[y][x].reset();
        }
    }
}

void Chunk::generateChunk(const FastNoise& heightNoise, const FastNoise& biomeNoise, const FastNoise& riverNoise, PlanetType planetType, Game& game, ChunkManager& chunkManager,
    PathfindingEngine& pathfindingEngine, bool allowStructureGen, bool spawnEntities)
{
    RandInt randGen = generateTilesAndStructure(heightNoise, biomeNoise, riverNoise, planetType, chunkManager, allowStructureGen);

    generateObjectsAndEntities(heightNoise, biomeNoise, riverNoise, planetType, randGen, game, chunkManager, spawnEntities);

    generateTilemapsAndInit(chunkManager, pathfindingEngine);
}

RandInt Chunk::generateTilesAndStructure(const FastNoise& heightNoise, const FastNoise& biomeNoise, const FastNoise& riverNoise, PlanetType planetType,
    ChunkManager& chunkManager, bool allowStructureGen)
{
    sf::Vector2i worldNoisePosition = sf::Vector2i(chunkPosition.x, chunkPosition.y) * static_cast<int>(CHUNK_TILE_SIZE);

    // Create random generator for chunk
    unsigned long int randSeed = (chunkManager.getSeed() + planetType) ^ chunkPosition.hash();
    RandInt randGen(randSeed);

    // Store tile types in tile array
    for (int y = 0; y < CHUNK_TILE_SIZE; y++)
    {
        for (int x = 0; x < CHUNK_TILE_SIZE; x++)
        {
            const TileGenData* tileGenData = getTileGenAtWorldTile(
                sf::Vector2i(worldNoisePosition.x + x, worldNoisePosition.y + y), chunkManager.getWorldSize(), heightNoise, biomeNoise, riverNoise, planetType
                );
            
            int tileType = 0;
            if (tileGenData)
            {
                // Not nullptr, so set tiletype to gen data tile ID
                tileType = tileGenData->tileID;
            }

            groundTileGrid[y][x] = tileType;
            
            if (tileType == 0)
            {
                containsWater = true;
            }
        }
    }

    // Create structure if required
    if (!containsWater)
    {
        generateRandomStructure(chunkManager.getWorldSize(), biomeNoise, randGen, planetType, allowStructureGen);
    }

    return randGen;
}

void Chunk::generateObjectsAndEntities(const FastNoise& heightNoise, const FastNoise& biomeNoise, const FastNoise& riverNoise, PlanetType planetType, RandInt& randGen,
    Game& game, ChunkManager& chunkManager, bool spawnEntities)
{
    sf::Vector2i worldNoisePosition = sf::Vector2i(chunkPosition.x, chunkPosition.y) * static_cast<int>(CHUNK_TILE_SIZE);

    // Set tile maps / spawn objects
    for (int y = 0; y < CHUNK_TILE_SIZE; y++)
    {
        for (int x = 0; x < CHUNK_TILE_SIZE; x++)
        {
            int tileType = groundTileGrid[y][x];

            // Tile is water
            if (tileType == 0)
            {
                continue;
            }

            if (objectGrid[y][x])
                continue;

            // Create random object
            ObjectType objectSpawnType = getRandomObjectToSpawnAtWorldTile(sf::Vector2i(worldNoisePosition.x + x, worldNoisePosition.y + y),
                chunkManager.getWorldSize(), heightNoise, biomeNoise, riverNoise, randGen, planetType);

            if (objectSpawnType >= 0)
            {
                setObject(sf::Vector2i(x, y), objectSpawnType, game, chunkManager, nullptr, false, true);
            }
            else
            {
                objectGrid[y][x] = nullptr;
            }
        }
    }

    if (spawnEntities)
    {
        spawnChunkEntities(chunkManager.getWorldSize(), heightNoise, biomeNoise, riverNoise, planetType);
    }
}

void Chunk::spawnChunkEntities(int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise, const FastNoise& riverNoise, PlanetType planetType)
{
    sf::Vector2i worldNoisePosition = sf::Vector2i(chunkPosition.x, chunkPosition.y) * static_cast<int>(CHUNK_TILE_SIZE);

    for (int y = 0; y < CHUNK_TILE_SIZE; y++)
    {
        for (int x = 0; x < CHUNK_TILE_SIZE; x++)
        {
            // Create random entity
            EntityType entitySpawnType = getRandomEntityToSpawnAtWorldTile(sf::Vector2i(worldNoisePosition.x + x, worldNoisePosition.y + y),
                worldSize, heightNoise, biomeNoise, riverNoise, planetType);
            
            if (entitySpawnType >= 0)
            {
                sf::Vector2f entityPos;
                entityPos.x = worldPosition.x + (x + 0.5) * TILE_SIZE_PIXELS_UNSCALED;
                entityPos.y = worldPosition.y + (y + 0.5) * TILE_SIZE_PIXELS_UNSCALED;

                std::unique_ptr<Entity> entity = std::make_unique<Entity>(entityPos, entitySpawnType);
                entities.push_back(std::move(entity));
            }
        }
    }
}

void Chunk::generateTilemapsAndInit(ChunkManager& chunkManager, PathfindingEngine& pathfindingEngine)
{
    int worldSize = chunkManager.getWorldSize();

    Chunk* upChunk = chunkManager.getChunk(ChunkPosition(chunkPosition.x, ((chunkPosition.y - 1) % worldSize + worldSize) % worldSize));
    Chunk* downChunk = chunkManager.getChunk(ChunkPosition(chunkPosition.x, ((chunkPosition.y + 1) % worldSize + worldSize) % worldSize));
    Chunk* leftChunk = chunkManager.getChunk(ChunkPosition(((chunkPosition.x - 1) % worldSize + worldSize) % worldSize, chunkPosition.y));
    Chunk* rightChunk = chunkManager.getChunk(ChunkPosition(((chunkPosition.x + 1) % worldSize + worldSize) % worldSize, chunkPosition.y));

    std::set<int> tileMapsModified;

    for (int y = 0; y < CHUNK_TILE_SIZE; y++)
    {
        for (int x = 0; x < CHUNK_TILE_SIZE; x++)
        {
            int tileType = groundTileGrid[y][x];

            // Tile is water
            if (tileType == 0)
            {
                continue;
            }

            // Set tile without graphics update
            std::set<int> additional_tileMapsModified = chunkManager.setChunkTile(chunkPosition, tileType, sf::Vector2i(x, y), false);
            tileMapsModified.insert(additional_tileMapsModified.begin(), additional_tileMapsModified.end());
        }
    }

    chunkManager.performChunkSetTileUpdate(chunkPosition, tileMapsModified);

    generateVisualEffectTiles(chunkManager);

    recalculateCollisionRects(chunkManager, &pathfindingEngine);

    generatedFromPOD = false;
}

void Chunk::generateRandomStructure(int worldSize, const FastNoise& biomeNoise, RandInt& randGen, PlanetType planetType, bool allowStructureGen)
{
    // Get biome gen data
    const BiomeGenData* biomeGenData = getBiomeGenAtWorldTile(sf::Vector2i(chunkPosition.x * CHUNK_TILE_SIZE, chunkPosition.y * CHUNK_TILE_SIZE),
        worldSize, biomeNoise, planetType);
    if (!biomeGenData)
        return;
    
    // Get random structure type
    StructureType structureType = -1;

    float cumulativeChance = 0;
    float randomSpawn = randGen.generate(0, 10000) / 10000.0f;
    for (const StructureGenData& structureGenData : biomeGenData->structureGenDatas)
    {
        cumulativeChance += structureGenData.spawnChance;

        if (randomSpawn <= cumulativeChance)
        {
            structureType = structureGenData.structure;
            break;
        }
    }

    // No structure chosen
    if (structureType < 0)
        return;
    
    // Generate structure

    // Read collision bitmask and create dummy objects in required positions
    const sf::Image& bitmaskImage = TextureManager::getBitmask(BitmaskType::Structures);

    const StructureData& structureData = StructureDataLoader::getStructureData(structureType);

    // Randomise spawn position in chunk
    sf::Vector2i spawnTile;
    spawnTile.x = randGen.generate(0, CHUNK_TILE_SIZE - 1 - structureData.size.x);
    spawnTile.y = randGen.generate(0, CHUNK_TILE_SIZE - 1 - structureData.size.y);

    // If not actually spawning structure, i.e. simply using this function to continue randgen sequence
    // then can skip actually generating the structure, as no more randgen for structures from this point
    if (!allowStructureGen)
    {
        return;
    }

    bool spawnedStructureObject = false;

    for (int x = 0; x < structureData.size.x; x++)
    {
        // Iterate over y backwards to place structure object in bottom left of dummy objects
        for (int y = structureData.size.y - 1; y >= 0; y--)
        {
            sf::Color bitmaskColor = bitmaskImage.getPixel(structureData.collisionBitmaskOffset.x + x, structureData.collisionBitmaskOffset.y + y);

            int tileX = x + spawnTile.x;
            int tileY = y + spawnTile.y;

            sf::Vector2f tileWorldPos;
            tileWorldPos.x = worldPosition.x + (tileX + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
            tileWorldPos.y = worldPosition.y + (tileY + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

            // Create actual structure object in bottom left of tiles
            if (!spawnedStructureObject)
            {
                structureObject = StructureObject(tileWorldPos, structureType);

                spawnedStructureObject = true;
            }

            // Collision
            if (bitmaskColor == sf::Color(255, 0, 0))
            {
                objectGrid[tileY][tileX] = std::make_unique<BuildableObject>(tileWorldPos, DUMMY_OBJECT_COLLISION);
            }

            // Dummy object with no collision (for warp / structure entrance)
            if (bitmaskColor == sf::Color(0, 255, 0))
            {
                objectGrid[tileY][tileX] = std::make_unique<BuildableObject>(tileWorldPos, DUMMY_OBJECT_NO_COLLISION);

                structureObject->createWarpRect(tileWorldPos - sf::Vector2f(0.5, 0.5) * TILE_SIZE_PIXELS_UNSCALED);
            }
        }
    }
}

const BiomeGenData* Chunk::getBiomeGenAtWorldTile(sf::Vector2i worldTile, int worldSize, const FastNoise& biomeNoise, PlanetType planetType)
{
    const PlanetGenData& planetGenData = PlanetGenDataLoader::getPlanetGenData(planetType);

    float biomeNoiseValue = biomeNoise.GetNoiseSeamless2D(worldTile.x, worldTile.y, worldSize * CHUNK_TILE_SIZE, worldSize * CHUNK_TILE_SIZE);
    biomeNoiseValue = FastNoise::Normalise(biomeNoiseValue);

    for (const BiomeGenData& biomeGenData : planetGenData.biomeGenDatas)
    {
        if (biomeGenData.noiseRangeMin <= biomeNoiseValue && biomeGenData.noiseRangeMax >= biomeNoiseValue)
        {
            return &biomeGenData;
        }
    }

    return nullptr;
}

const TileGenData* Chunk::getTileGenAtWorldTile(sf::Vector2i worldTile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise, const FastNoise& riverNoise,
    PlanetType planetType)
{
    int worldTileSize = worldSize * CHUNK_TILE_SIZE;

    const PlanetGenData& planetGenData = PlanetGenDataLoader::getPlanetGenData(planetType);

    float riverNoiseValue = riverNoise.GetNoiseSeamless2D(worldTile.x, worldTile.y, worldTileSize, worldTileSize);
    riverNoiseValue = FastNoise::Normalise(riverNoiseValue);
    if (riverNoiseValue >= planetGenData.riverNoiseRangeMin && riverNoiseValue <= planetGenData.riverNoiseRangeMax)
    {
        return nullptr;
    }

    const BiomeGenData* biomeGenData = getBiomeGenAtWorldTile(worldTile, worldSize, biomeNoise, planetType);
    
    if (!biomeGenData)
        return nullptr;

    
    float heightNoiseValue = heightNoise.GetNoiseSeamless2D(worldTile.x, worldTile.y, worldTileSize, worldTileSize);
    heightNoiseValue = FastNoise::Normalise(heightNoiseValue);

    const TileGenData* tileGenDataPtr = nullptr;
    
    for (const TileGenData& tileGenData : biomeGenData->tileGenDatas)
    {
        if (tileGenData.noiseRangeMin <= heightNoiseValue && tileGenData.noiseRangeMax >= heightNoiseValue)
        {
            tileGenDataPtr = &tileGenData;
        }
    }

    if (tileGenDataPtr != nullptr)
    {
        // Check river noise around surroundings - if is river, must use lowest biome tile
        // Prevents non-full tiles (e.g. grass) being shown on top of water
        std::vector<float> surroundingRiverNoiseValues;

        surroundingRiverNoiseValues.push_back(FastNoise::Normalise(
            riverNoise.GetNoiseSeamless2D(Helper::wrap(worldTile.x + 1, worldTileSize), worldTile.y, worldTileSize, worldTileSize)));
        surroundingRiverNoiseValues.push_back(FastNoise::Normalise(
            riverNoise.GetNoiseSeamless2D(Helper::wrap(worldTile.x - 1, worldTileSize), worldTile.y, worldTileSize, worldTileSize)));
        surroundingRiverNoiseValues.push_back(FastNoise::Normalise(
            riverNoise.GetNoiseSeamless2D(worldTile.x, Helper::wrap(worldTile.y + 1, worldTileSize), worldTileSize, worldTileSize)));
        surroundingRiverNoiseValues.push_back(FastNoise::Normalise(
            riverNoise.GetNoiseSeamless2D(worldTile.x, Helper::wrap(worldTile.y - 1, worldTileSize), worldTileSize, worldTileSize)));
        
        for (float noiseValue : surroundingRiverNoiseValues)
        {
            if (noiseValue >= planetGenData.riverNoiseRangeMin && noiseValue <= planetGenData.riverNoiseRangeMax)
            {
                // Surrounding is river, tile is river edge - return lowest biome tile
                return &biomeGenData->tileGenDatas[0];
            }
        }
    }

    return tileGenDataPtr;
}

ObjectType Chunk::getRandomObjectToSpawnAtWorldTile(sf::Vector2i worldTile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise,
    const FastNoise& riverNoise, RandInt& randGen, PlanetType planetType)
{
    const TileGenData* tileGenData = getTileGenAtWorldTile(worldTile, worldSize, heightNoise, biomeNoise, riverNoise, planetType);

    if (!tileGenData->objectsCanSpawn)
        return -1;

    const BiomeGenData* biomeGenData = getBiomeGenAtWorldTile(worldTile, worldSize, biomeNoise, planetType);

    float cumulativeChance = 0;
    float randomSpawn = static_cast<float>(randGen.generate(0, 10000)) / 10000.0f;
    for (const ObjectGenData& objectGenData : biomeGenData->objectGenDatas)
    {
        cumulativeChance += objectGenData.spawnChance;

        if (randomSpawn <= cumulativeChance)
        {
            return objectGenData.object;
        }
    }

    return -1;
}

EntityType Chunk::getRandomEntityToSpawnAtWorldTile(sf::Vector2i worldTile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise,
    const FastNoise& riverNoise, PlanetType planetType)
{
    const TileGenData* tileGenData = getTileGenAtWorldTile(worldTile, worldSize, heightNoise, biomeNoise, riverNoise, planetType);

    if (tileGenData == nullptr)
    {
        return -1;
    }

    if (!tileGenData->objectsCanSpawn)
        return -1;

    const BiomeGenData* biomeGenData = getBiomeGenAtWorldTile(worldTile, worldSize, biomeNoise, planetType);

    float cumulativeChance = 0;
    float randomSpawn = Helper::randFloat(0.0f, 1.0f);
    for (const EntityGenData& entityGenData : biomeGenData->entityGenDatas)
    {
        cumulativeChance += entityGenData.spawnChance;

        if (randomSpawn <= cumulativeChance)
        {
            return entityGenData.entity;
        }
    }

    return -1;
}

void Chunk::generateVisualEffectTiles(ChunkManager& chunkManager)
{
    // Get visual tile from 3x3 area, where centre is tile testing for
    // Assumes centre tile is water
    auto getVisualTileType = [](int topLeft,    int top,      int topRight,
                                int left,                          int right,
                                int bottomLeft, int bottom,   int bottomRight) -> TileType
    {
        // Cliffs
        if (top != 0)
        {
            // Straight cliff
            if (topLeft != 0 && topRight != 0)
                return TileType::Visual_Cliff;
            
            // Left cliff
            if (topLeft == 0 && topRight != 0)
                return TileType::Visual_LCliff;
            
            // Right cliff
            if (topLeft != 0 && topRight == 0)
                return TileType::Visual_RCliff;
            
            // Left-right cliff
            if (topLeft == 0 && topRight == 0)
                return TileType::Visual_LRCliff;
        }

        // Default case
        return TileType::Visual_BLANK;
    };

    auto getTileTypeAt = [this](ChunkPosition chunk, sf::Vector2i tile, int xOffset, int yOffset,
                                ChunkManager& chunkManager)
    {
        std::pair<ChunkPosition, sf::Vector2i> wrappedChunkTile = ChunkManager::getChunkTileFromOffset(chunk, tile, xOffset, yOffset, chunkManager.getWorldSize());

        if (wrappedChunkTile.first == chunk)
        {
            return static_cast<int>(groundTileGrid[wrappedChunkTile.second.y][wrappedChunkTile.second.x]);
        }

        return chunkManager.getChunkTileTypeOrPredicted(wrappedChunkTile.first, wrappedChunkTile.second);
    };

    // Clear previously generated visual tiles
    for (auto& row : visualTileGrid)
    {
        row.fill(TileType::Visual_BLANK);
    }

    // Generate visual tiles
    for (int y = 0; y < CHUNK_TILE_SIZE; y++)
    {
        for (int x = 0; x < CHUNK_TILE_SIZE; x++)
        {
            int groundTileType = groundTileGrid[y][x];
            if (groundTileType != 0) // water
                continue;

            sf::Vector2i tilePos(x, y);

            visualTileGrid[y][x] = getVisualTileType(
                getTileTypeAt(chunkPosition, tilePos, -1, -1, chunkManager),
                getTileTypeAt(chunkPosition, tilePos, 0, -1, chunkManager),
                getTileTypeAt(chunkPosition, tilePos, 1, -1, chunkManager),
                getTileTypeAt(chunkPosition, tilePos, -1, 0, chunkManager),
                getTileTypeAt(chunkPosition, tilePos, 1, 0, chunkManager),
                getTileTypeAt(chunkPosition, tilePos, -1, 1, chunkManager),
                getTileTypeAt(chunkPosition, tilePos, 0, 1, chunkManager),
                getTileTypeAt(chunkPosition, tilePos, 1, 1, chunkManager)
            );
        }
    }
}

void Chunk::setTile(int tileMap, sf::Vector2i position, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles, bool graphicsUpdate)
{
    if (!tileMaps.contains(tileMap))
    {
        tileMaps[tileMap] = PlanetGenDataLoader::getTileMapFromID(tileMap);
    }

    // Set tile for tilemap
    if (graphicsUpdate)
    {
        tileMaps[tileMap].setTile(position.x, position.y, upTiles, downTiles, leftTiles, rightTiles);
    }
    else
    {
        tileMaps[tileMap].setTileWithoutGraphicsUpdate(position.x, position.y, upTiles, downTiles, leftTiles, rightTiles);
    }
}

void Chunk::setTile(int tileMap, sf::Vector2i position, Chunk* upChunk, Chunk* downChunk, Chunk* leftChunk, Chunk* rightChunk, bool graphicsUpdate)
{
    if (!tileMaps.contains(tileMap))
    {
        tileMaps[tileMap] = PlanetGenDataLoader::getTileMapFromID(tileMap);
    }
    
    TileMap* upTiles = nullptr;
    if (upChunk != nullptr)
        upTiles = upChunk->getTileMap(tileMap);

    TileMap* downTiles = nullptr;
    if (downChunk != nullptr)
        downTiles = downChunk->getTileMap(tileMap);

    TileMap* leftTiles = nullptr;
    if (leftChunk != nullptr)
        leftTiles = leftChunk->getTileMap(tileMap);

    TileMap* rightTiles = nullptr;
    if (rightChunk != nullptr)
        rightTiles = rightChunk->getTileMap(tileMap);

    if (graphicsUpdate)
    {
        tileMaps[tileMap].setTile(position.x, position.y, upTiles, downTiles, leftTiles, rightTiles);
    }
    else
    {
        tileMaps[tileMap].setTileWithoutGraphicsUpdate(position.x, position.y, upTiles, downTiles, leftTiles, rightTiles);
    }
}

void Chunk::updateTileMap(int tileMap, int xRel, int yRel, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles)
{
    if (tileMaps.count(tileMap) <= 0)
        return;
    
    if (xRel < 0)
    {
        tileMaps[tileMap].refreshRightEdge(upTiles, downTiles, leftTiles, rightTiles);
    }
    else if (xRel > 0)
    {
        tileMaps[tileMap].refreshLeftEdge(upTiles, downTiles, leftTiles, rightTiles);
    }

    if (yRel < 0)
    {
        tileMaps[tileMap].refreshBottomEdge(upTiles, downTiles, leftTiles, rightTiles);
    }
    else if (yRel > 0)
    {
        tileMaps[tileMap].refreshTopEdge(upTiles, downTiles, leftTiles, rightTiles);
    }

    // tileMaps[tileMap].updateAllTiles(upTiles, downTiles, leftTiles, rightTiles);
}

TileMap* Chunk::getTileMap(int tileMap)
{
    if (tileMaps.count(tileMap) <= 0)
        return nullptr;
    
    return &(tileMaps[tileMap]);
}

void Chunk::drawChunkTerrain(sf::RenderTarget& window, const Camera& camera, float time)
{
    // Get tile size and scale
    float scale = ResolutionHandler::getScale();

    for (auto iter = tileMaps.begin(); iter != tileMaps.end(); ++iter)
    {
        #if (!RELEASE_BUILD)
        if (!DebugOptions::tileMapsVisible[iter->first])
            continue;
        #endif

        iter->second.draw(window, camera.worldToScreenTransform(worldPosition), sf::Vector2f(scale, scale));
    }


    #if (!RELEASE_BUILD)
    // DEBUG DRAW LINE TO ENTITIES
    if (DebugOptions::drawEntityChunkParents)
    {
        for (auto& entity : entities)
        {
            sf::VertexArray lines(sf::Lines, 2);
            lines[0].position = camera.worldToScreenTransform(worldPosition);
            lines[1].position = camera.worldToScreenTransform(entity->getPosition());
            window.draw(lines);
        }
    }

    // DEBUG CHUNK OUTLINE DRAW
    if (DebugOptions::drawChunkBoundaries)
    {
        sf::VertexArray lines(sf::Lines, 8);
        lines[0].position = camera.worldToScreenTransform(worldPosition);
        lines[1].position = camera.worldToScreenTransform(worldPosition + sf::Vector2f(CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED, 0));
        lines[2].position = camera.worldToScreenTransform(worldPosition);
        lines[3].position = camera.worldToScreenTransform(worldPosition + sf::Vector2f(0, CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED));
        lines[4].position = camera.worldToScreenTransform(worldPosition + sf::Vector2f(CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED, 0));
        lines[5].position = camera.worldToScreenTransform(worldPosition + sf::Vector2f(CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED, CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED));
        lines[6].position = camera.worldToScreenTransform(worldPosition + sf::Vector2f(0, CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED));
        lines[7].position = camera.worldToScreenTransform(worldPosition + sf::Vector2f(CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED, CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED));
        
        window.draw(lines);
    }

    // DRAW COLLISIONS
    if (DebugOptions::drawCollisionRects)
    {
        for (auto& collisionRect : collisionRects)
        {
            collisionRect.debugDraw(window, camera);
        }
    }
    #endif
}

void Chunk::drawChunkTerrainVisual(sf::RenderTarget& window, SpriteBatch& spriteBatch, const Camera& camera, PlanetType planetType, float time)
{
    // Get tile size and scale
    float scale = ResolutionHandler::getScale();
    // float tileSize = ResolutionHandler::getTileSize();

    const PlanetGenData& planetGenData = PlanetGenDataLoader::getPlanetGenData(planetType);

    // Draw visual stuff (edge of tiles/cliffs etc)
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            TileType visualTileType = visualTileGrid[y][x];

            if (visualTileType == TileType::Visual_BLANK)
                continue;

            sf::Vector2f tileWorldPosition(worldPosition.x + x * TILE_SIZE_PIXELS_UNSCALED, worldPosition.y + y * TILE_SIZE_PIXELS_UNSCALED);
            sf::IntRect textureRect;

            int cliffWaterFrame = static_cast<int>(std::min(std::max(std::sin(time / 2.0f + chunkPosition.x + x) * 3.5f / 2.0f + 3.5f / 2.0f, 0.0f), 3.0f));
            
            switch (visualTileType)
            {
                case TileType::Visual_Cliff:
                    textureRect = sf::IntRect(planetGenData.cliffTextureOffset.x + cliffWaterFrame * 16, planetGenData.cliffTextureOffset.y, 16, 16);
                    tileWorldPosition.y -= 2;
                    break;
                case TileType::Visual_LCliff:
                    textureRect = sf::IntRect(planetGenData.cliffTextureOffset.x + cliffWaterFrame * 16, planetGenData.cliffTextureOffset.y + 16, 16, 16);
                    tileWorldPosition.y -= 2;
                    break;
                case TileType::Visual_RCliff:
                    textureRect = sf::IntRect(planetGenData.cliffTextureOffset.x + cliffWaterFrame * 16, planetGenData.cliffTextureOffset.y + 32, 16, 16);
                    tileWorldPosition.y -= 2;
                    break;
                case TileType::Visual_LRCliff:
                    textureRect = sf::IntRect(planetGenData.cliffTextureOffset.x + cliffWaterFrame * 16, planetGenData.cliffTextureOffset.y + 48, 16, 16);
                    tileWorldPosition.y -= 2;
                    break;
            }

            // sf::Shader* cliffShader = Shaders::getShader(ShaderType::Cliff);

            spriteBatch.draw(window, {TextureType::GroundTiles, camera.worldToScreenTransform(tileWorldPosition), 0, {scale, scale}},
                textureRect);

            // TextureManager::drawSubTexture(window, {TextureType::GroundTiles, Camera::worldToScreenTransform(tileWorldPosition), 0, {scale, scale}}, textureRect);
        }
    }
}

void Chunk::drawChunkWater(sf::RenderTarget& window, const Camera& camera, ChunkManager& chunkManager)
{
    // Get scale
    float scale = ResolutionHandler::getScale();

    // Set water colour
    const BiomeGenData* chunkBiome = chunkManager.getChunkBiome(chunkPosition);
    
    const BiomeGenData* surroundingChunkBiomes[8] = {
        chunkManager.getChunkBiome(ChunkPosition{chunkPosition.x, Helper::wrap(chunkPosition.y - 1, chunkManager.getWorldSize())}),
        chunkManager.getChunkBiome(ChunkPosition{Helper::wrap(chunkPosition.x + 1, chunkManager.getWorldSize()), chunkPosition.y}),
        chunkManager.getChunkBiome(ChunkPosition{chunkPosition.x, Helper::wrap(chunkPosition.y + 1, chunkManager.getWorldSize())}),
        chunkManager.getChunkBiome(ChunkPosition{Helper::wrap(chunkPosition.x - 1, chunkManager.getWorldSize()), chunkPosition.y}),
        chunkManager.getChunkBiome(
        ChunkPosition{Helper::wrap(chunkPosition.x - 1, chunkManager.getWorldSize()), Helper::wrap(chunkPosition.y - 1, chunkManager.getWorldSize())}),
        chunkManager.getChunkBiome(
        ChunkPosition{Helper::wrap(chunkPosition.x + 1, chunkManager.getWorldSize()), Helper::wrap(chunkPosition.y - 1, chunkManager.getWorldSize())}),
        chunkManager.getChunkBiome(
        ChunkPosition{Helper::wrap(chunkPosition.x + 1, chunkManager.getWorldSize()), Helper::wrap(chunkPosition.y + 1, chunkManager.getWorldSize())}),
        chunkManager.getChunkBiome(
        ChunkPosition{Helper::wrap(chunkPosition.x - 1, chunkManager.getWorldSize()), Helper::wrap(chunkPosition.y + 1, chunkManager.getWorldSize())})
    };

    if (chunkBiome == nullptr)
    {
        return;
    }

    sf::Glsl::Vec4 surroundingWaterColors[8];
    
    for (int i = 0; i < 8; i++)
    {
        if (surroundingChunkBiomes[i] == nullptr)
        {
            return;
        }

        surroundingWaterColors[i] = surroundingChunkBiomes[i]->waterColour;
    }
    
    sf::Shader* waterShader = Shaders::getShader(ShaderType::Water);

    waterShader->setUniform("waterColor", sf::Glsl::Vec4(chunkBiome->waterColour));
    waterShader->setUniformArray("surroundingWaterColors", surroundingWaterColors, 8);
    waterShader->setUniform("spriteSheetSize", sf::Glsl::Vec2(TextureManager::getTextureSize(TextureType::Water)));

    const PlanetGenData& planetGenData = PlanetGenDataLoader::getPlanetGenData(chunkManager.getPlanetType());

    waterShader->setUniform("textureRect", sf::Glsl::Vec4(planetGenData.waterTextureOffset.x, planetGenData.waterTextureOffset.y, 32, 32));

    // Draw water
    sf::Vector2f waterPos = camera.worldToScreenTransform(worldPosition);
    sf::IntRect waterRect(0, 0, TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE, TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE);

    TextureManager::drawSubTexture(window, {TextureType::Water, waterPos, 0, {scale, scale}}, waterRect, waterShader);
}

void Chunk::updateChunkObjects(Game& game, float dt, int worldSize, ChunkManager& chunkManager, PathfindingEngine& pathfindingEngine)
{
    for (int y = 0; y < objectGrid.size(); y++)
    {
        auto& object_row = objectGrid[y];
        for (int x = 0; x < object_row.size(); x++)
        {
            BuildableObject* object = object_row[x].get();
            if (object)
            {
                // OccupiedTileObject* occupiedTile = dynamic_cast<OccupiedTileObject*>(object.get());
                // If object is object reference, do not update
                if (object->isObjectReference())
                    continue;
                
                // Determine whether on water
                bool onWater = (getTileType(object->getChunkTileInside(worldSize)) == 0);
                
                object->update(game, dt, onWater);
                if (!object->isAlive())
                    deleteObject(sf::Vector2i(x, y), chunkManager, pathfindingEngine);
            }
        }
    }

    // recalculateCollisionRects(chunkManager);
}

std::vector<WorldObject*> Chunk::getObjects()
{
    std::vector<WorldObject*> objects;
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            // OccupiedTileObject* occupiedTile = dynamic_cast<OccupiedTileObject*>(objectGrid[y][x].get());
            if (!objectGrid[y][x])
                continue;

            // If object is object reference, do not draw
            if (objectGrid[y][x]->isObjectReference())
                continue;
            
            // Is valid object

            // Add object to vector
            objects.push_back(objectGrid[y][x].get());
        }
    }

    // Structure object if required
    if (structureObject.has_value())
    {
        objects.push_back(&structureObject.value());
    }

    return objects;
}

BuildableObject* Chunk::getObject(sf::Vector2i position)
{
    return objectGrid[position.y][position.x].get();
}

int Chunk::getTileType(sf::Vector2i position) const
{
    return groundTileGrid[position.y][position.x];
}

void Chunk::setObject(sf::Vector2i position, ObjectType objectType, Game& game, ChunkManager& chunkManager, PathfindingEngine* pathfindingEngine,
    bool recalculateCollision, bool calledWhileGenerating)
{
    if (!calledWhileGenerating)
    {
        modified = true;
    }

    // sf::Vector2f worldPosition = static_cast<sf::Vector2f>(chunkPosition) * 8.0f * tileSize;
    sf::Vector2f objectPos;
    objectPos.x = worldPosition.x + position.x * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED / 2.0f;
    objectPos.y = worldPosition.y + position.y * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED / 2.0f;

    sf::Vector2i objectSize(1, 1);
    if (objectType >= 0)
    {
        objectSize = ObjectDataLoader::getObjectData(objectType).size;
    }

    // Set object in chunk
    objectGrid[position.y][position.x] = BuildableObjectFactory::create(objectPos, objectType, &game, !calledWhileGenerating);

    // Create object reference objects if object is larger than one tile
    if (objectSize != sf::Vector2i(1, 1))
    {
        ObjectReference objectReference;
        objectReference.chunk = ChunkPosition(chunkPosition.x, chunkPosition.y);
        objectReference.tile = position;

        for (int y = 0; y < objectSize.y; y++)
        {
            for (int x = 0; x < objectSize.x; x++)
            {
                // Don't place object reference on actual object tile
                if (x == 0 && y == 0)
                {
                    continue;
                }

                auto chunkTile = ChunkManager::getChunkTileFromOffset(chunkPosition, position, x, y, chunkManager.getWorldSize());
                chunkManager.setObjectReference(chunkTile.first, objectReference, chunkTile.second);
            }
        }
    }

    if (recalculateCollision)
    {
        recalculateCollisionRects(chunkManager, pathfindingEngine);
    }
}

void Chunk::deleteObject(sf::Vector2i position, ChunkManager& chunkManager, PathfindingEngine& pathfindingEngine)
{
    modified = true;

    // Get size of object to handle different deletion cases
    ObjectType objectType = objectGrid[position.y][position.x]->getObjectType();

    sf::Vector2i objectSize(1, 1);
    if (objectType >= 0)
    {
        const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
        objectSize = objectData.size;
    }

    // Object is single tile
    if (objectSize == sf::Vector2i(1, 1))
    {
        deleteSingleObject(position, chunkManager, pathfindingEngine);
        return;
    }

    int worldSize = chunkManager.getWorldSize();

    for (int y = 0; y < objectSize.y; y++)
    {
        for (int x = 0; x < objectSize.x; x++)
        {
            auto chunkTile = ChunkManager::getChunkTileFromOffset(chunkPosition, position, x, y, worldSize);
            chunkManager.deleteSingleObject(chunkTile.first, chunkTile.second);
        }
    }
}

void Chunk::deleteSingleObject(sf::Vector2i position, ChunkManager& chunkManager, PathfindingEngine& pathfindingEngine)
{
    objectGrid[position.y][position.x].reset();
    recalculateCollisionRects(chunkManager, &pathfindingEngine);
}

void Chunk::setObjectReference(const ObjectReference& objectReference, sf::Vector2i tile, ChunkManager& chunkManager, PathfindingEngine& pathfindingEngine)
{
    modified = true;

    objectGrid[tile.y][tile.x] = std::make_unique<BuildableObject>(objectReference);

    recalculateCollisionRects(chunkManager, &pathfindingEngine);
}

bool Chunk::canPlaceObject(sf::Vector2i position, ObjectType objectType, int worldSize, ChunkManager& chunkManager)
{
    // Get data of object type to test
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    // Test all tiles taken up by object
    for (int y = 0; y < objectData.size.y; y++)
    {
        for (int x = 0; x < objectData.size.x; x++)
        {
            auto chunkTile = ChunkManager::getChunkTileFromOffset(chunkPosition, position, x, y, worldSize);

            // Test object
            BuildableObject* object = chunkManager.getChunkObject(chunkTile.first, chunkTile.second);
            if (object)
            {
                return false;
            }

            bool tileIsWater = (chunkManager.getLoadedChunkTileType(chunkTile.first, chunkTile.second) == 0);

            // Test tile
            if (objectData.placeOnWater)
            {
                return (tileIsWater);
            }
            
            if (tileIsWater)
                return false;
        }
    }
    
    // Passed all tests, so return true
    return true;
}

void Chunk::updateChunkEntities(float dt, int worldSize, ProjectileManager& projectileManager, ChunkManager& chunkManager, Game& game)
{
    for (std::vector<std::unique_ptr<Entity>>::iterator entityIter = entities.begin(); entityIter != entities.end();)
    {
        std::unique_ptr<Entity>& entity = *entityIter;

        // Determine whether on water
        bool onWater = (getTileType(entity->getChunkTileInside(worldSize)) == 0);

        entity->update(dt, projectileManager, chunkManager, game, onWater, game.getGameTime());

        // Check if requires deleting (not alive)
        if (!entity->isAlive())
        {
            entityIter = entities.erase(entityIter);
            continue;
        }

        // Check if requires moving to different chunk
        sf::Vector2f relativePosition = entity->getPosition() - worldPosition;

        bool requiresMove = false;
        ChunkPosition newChunk(chunkPosition.x, chunkPosition.y);

        if (relativePosition.x < 0)
        {
            newChunk.x = (((chunkPosition.x - 1) % worldSize) + worldSize) % worldSize;
            requiresMove = true;
        }
        else if (relativePosition.x > TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE)
        {
            newChunk.x = (((chunkPosition.x + 1) % worldSize) + worldSize) % worldSize;
            requiresMove = true;
        }
        else if (relativePosition.y < 0)
        {
            newChunk.y = (((chunkPosition.y - 1) % worldSize) + worldSize) % worldSize;
            requiresMove = true;
        }
        else if (relativePosition.y > TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE)
        {
            newChunk.y = (((chunkPosition.y + 1) % worldSize) + worldSize) % worldSize;
            requiresMove = true;
        }

        if (requiresMove)
        {
            chunkManager.moveEntityToChunkFromChunk(std::move(entity), newChunk);
            entityIter = entities.erase(entityIter);
            continue;
        }

        entityIter++;
    }
}

void Chunk::testEntityHitCollision(const std::vector<HitRect>& hitRects, ChunkManager& chunkManager, Game& game, float gameTime)
{
    for (auto& entity : entities)
    {
        entity->testHitCollision(hitRects, chunkManager, game, gameTime);
    }
}

void Chunk::moveEntityToChunk(std::unique_ptr<Entity> entity)
{
    entities.push_back(std::move(entity));
}

Entity* Chunk::getSelectedEntity(sf::Vector2f cursorPos)
{
    for (auto& entity : entities)
    {
        if (entity->isSelectedWithCursor(cursorPos))
            return entity.get();
    }
    return nullptr;
}

uint64_t Chunk::addItemPickup(const ItemPickup& itemPickup, std::optional<uint64_t> idOverride)
{
    if (idOverride.has_value())
    {
        itemPickups[idOverride.value()] = itemPickup;
        return idOverride.value();
    }

    itemPickups[itemPickupCounter] = itemPickup;
    return itemPickupCounter++;
}
    
std::optional<ItemPickupReference> Chunk::getCollidingItemPickup(const CollisionRect& playerCollision, float gameTime)
{
    for (auto iter = itemPickups.begin(); iter != itemPickups.end();)
    {
        if (iter->second.isBeingPickedUp(playerCollision, gameTime))
        {
            // iter->second.resetSpawnTime(gameTime);
            return ItemPickupReference{chunkPosition, iter->first};
        }

        iter++;
    }

    return std::nullopt;
}

void Chunk::deleteItemPickup(uint64_t id)
{
    if (!itemPickups.contains(id))
    {
        return;
    }

    itemPickups.erase(id);
}

ItemPickup* Chunk::getItemPickup(uint64_t id)
{
    if (!itemPickups.contains(id))
    {
        return nullptr;
    }

    return &itemPickups[id];
}

std::vector<WorldObject*> Chunk::getItemPickups()
{
    std::vector<WorldObject*> itemPickupWorldObjects;

    for (auto& itemPickup : itemPickups)
    {
        itemPickupWorldObjects.push_back(&itemPickup.second);
    }

    return itemPickupWorldObjects;
}

void Chunk::recalculateCollisionRects(ChunkManager& chunkManager, PathfindingEngine* pathfindingEngine)
{
    auto createCollisionRect = [this](std::vector<CollisionRect>& rects, int x, int y) -> void
    {
        CollisionRect collisionRect;

        collisionRect.x = this->worldPosition.x + x * TILE_SIZE_PIXELS_UNSCALED;
        collisionRect.y = this->worldPosition.y + y * TILE_SIZE_PIXELS_UNSCALED;
        collisionRect.width = TILE_SIZE_PIXELS_UNSCALED;
        collisionRect.height = TILE_SIZE_PIXELS_UNSCALED;

        rects.push_back(collisionRect);
    };

    // Clear previously calculated collision rects
    collisionRects.clear();

    int pathfindingTopLeftX = chunkPosition.x * static_cast<int>(CHUNK_TILE_SIZE);
    int pathfindingTopLeftY = chunkPosition.y * static_cast<int>(CHUNK_TILE_SIZE);

    // Clear pathfinding for chunk if pathfinding engine is passed in
    if (pathfindingEngine)
    {
        for (int y = 0; y < static_cast<int>(CHUNK_TILE_SIZE); y++)
        {
            for (int x = 0; x < static_cast<int>(CHUNK_TILE_SIZE); x++)
            {
                pathfindingEngine->setObstacle(pathfindingTopLeftX + x, pathfindingTopLeftY + y, false);
            }
        }
    }

    // Get collisions for tiles
    for (int y = 0; y < CHUNK_TILE_SIZE; y++)
    {
        for (int x = 0; x < CHUNK_TILE_SIZE; x++)
        {
            bool bridgeObjectOnWater = false;

            BuildableObject* object = objectGrid[y][x].get();
            if (object)
            {
                int objectType = object->getObjectType();
                if (objectType >= 0)
                {
                    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
                    bridgeObjectOnWater = !objectData.hasCollision;
                }
            }

            if (bridgeObjectOnWater)
                continue;

            if (groundTileGrid[y][x] == 0)
            {
                createCollisionRect(collisionRects, x, y);
                if (pathfindingEngine)
                {
                    pathfindingEngine->setObstacle(pathfindingTopLeftX + x, pathfindingTopLeftY + y, true);
                }
            }
        }
    }

    // Get collisions for objects
    for (int y = 0; y < CHUNK_TILE_SIZE; y++)
    {
        for (int x = 0; x < CHUNK_TILE_SIZE; x++)
        {
            if (!objectGrid[y][x])
                continue;

            // Go through chunk manager in case of object reference
            BuildableObject* object = chunkManager.getChunkObject(chunkPosition, sf::Vector2i(x, y));

            if (!object)
                continue;
            
            // Dummy object
            if (object->isDummyObject())
            {
                if (object->dummyHasCollision())
                {
                    createCollisionRect(collisionRects, x, y);
                    if (pathfindingEngine)
                    {
                        pathfindingEngine->setObstacle(pathfindingTopLeftX + x, pathfindingTopLeftY + y, true);
                    }
                }
                continue;
            }

            const ObjectData& objectData = ObjectDataLoader::getObjectData(object->getObjectType());
            
            if (objectData.hasCollision)
            {
                createCollisionRect(collisionRects, x, y);
                if (pathfindingEngine)
                {
                    pathfindingEngine->setObstacle(pathfindingTopLeftX + x, pathfindingTopLeftY + y, true);
                }
            }
        }
    }
}

std::vector<CollisionRect*> Chunk::getCollisionRects()
{
    std::vector<CollisionRect*> collisionRectPtrs;
    for (auto& collisionRect : collisionRects)
    {
        collisionRectPtrs.push_back(&collisionRect);
    }
    return collisionRectPtrs;
}

bool Chunk::collisionRectStaticCollisionX(CollisionRect& collisionRect, float dx)
{
    bool collision = false;
    for (auto& rect : collisionRects)
    {
        if (collisionRect.handleStaticCollisionX(rect, dx))
            collision = true;
    }
    return collision;
}

bool Chunk::collisionRectStaticCollisionY(CollisionRect& collisionRect, float dy)
{
    bool collision = false;
    for (auto& rect : collisionRects)
    {
        if (collisionRect.handleStaticCollisionY(rect, dy))
            collision = true;
    }
    return collision;
}

bool Chunk::isCollisionRectCollidingWithEntities(const CollisionRect& collisionRect)
{
    for (auto& entity : entities)
    {
        const CollisionRect& entityCollisionRect = entity->getCollisionRect();
        if (entityCollisionRect.isColliding(collisionRect))
            return true;
    }
    return false;
}

bool Chunk::canPlaceLand(sf::Vector2i tile)
{
    if (objectGrid[tile.y][tile.x])
        return false;
    
    if (getTileType(tile) != 0)
        return false;
    
    return true;
}

void Chunk::placeLand(sf::Vector2i tile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise,
    PlanetType planetType, ChunkManager& chunkManager, PathfindingEngine& pathfindingEngine)
{
    modified = true;

    // Chunk* upChunk = chunkManager.getChunk(ChunkPosition(chunkPosition.x, ((chunkPosition.y - 1) % worldSize + worldSize) % worldSize));
    // Chunk* downChunk = chunkManager.getChunk(ChunkPosition(chunkPosition.x, ((chunkPosition.y + 1) % worldSize + worldSize) % worldSize));
    // Chunk* leftChunk = chunkManager.getChunk(ChunkPosition(((chunkPosition.x - 1) % worldSize + worldSize) % worldSize, chunkPosition.y));
    // Chunk* rightChunk = chunkManager.getChunk(ChunkPosition(((chunkPosition.x + 1) % worldSize + worldSize) % worldSize, chunkPosition.y));

    sf::Vector2i worldNoisePosition = sf::Vector2i(chunkPosition.x, chunkPosition.y) * static_cast<int>(CHUNK_TILE_SIZE);

    const BiomeGenData* biomeGenData = getBiomeGenAtWorldTile(sf::Vector2i(worldNoisePosition.x + tile.x, worldNoisePosition.y + tile.y),
        worldSize, biomeNoise, planetType);
    
    if (!biomeGenData)
        return;
    
    if (biomeGenData->tileGenDatas.size() <= 0)
        return;
    
    const TileGenData& tileGenData = biomeGenData->tileGenDatas[0];

    groundTileGrid[tile.y][tile.x] = tileGenData.tileID;

    if (!tileMaps.contains(tileGenData.tileID))
    {
        tileMaps[tileGenData.tileID] = TileMap(tileGenData.tileMap.textureOffset, tileGenData.tileMap.variation);
    }

    chunkManager.setChunkTile(chunkPosition, tileGenData.tileID, tile);

    // Update visual tiles
    generateVisualEffectTiles(chunkManager);

    // Recalculate collision rects
    recalculateCollisionRects(chunkManager, &pathfindingEngine);
}

bool Chunk::isPlayerInStructureEntrance(sf::Vector2f playerPos, StructureEnterEvent& enterEvent)
{
    if (!structureObject.has_value())
        return false;
   
    bool inEntrance = structureObject->isPlayerInEntrance(playerPos, enterEvent);

    if (inEntrance)
    {
        // Set chunk modified to true if entered structure as player has then interacted with structure
        // e.g. taken items etc
        modified = true;
    }

    return inEntrance;
}

bool Chunk::hasStructure()
{
    return (structureObject.has_value());
}

ChunkPOD Chunk::getChunkPOD()
{
    ChunkPOD pod;
    pod.chunkPosition = chunkPosition;
    pod.groundTileGrid = groundTileGrid;
    pod.modified = modified;

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            if (objectGrid[y][x])
            {
                pod.objectGrid[y][x] = objectGrid[y][x]->getPOD();
            }
            else
            {
                pod.objectGrid[y][x] = std::nullopt;
            }
        }
    }

    for (auto& entity : entities)
    {
        pod.entities.push_back(entity->getPOD(worldPosition));
    }

    if (structureObject.has_value())
    {
        pod.structureObject = structureObject->getPOD(worldPosition);
    }
    else
    {
        pod.structureObject = std::nullopt;
    }

    return pod;
}

void Chunk::loadFromChunkPOD(const ChunkPOD& pod, Game& game)
{
    generatedFromPOD = true;
    modified = pod.modified;

    groundTileGrid = pod.groundTileGrid;
    worldPosition = sf::Vector2f(chunkPosition.x * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED, chunkPosition.y * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED);

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            const std::optional<BuildableObjectPOD>& objectPOD = pod.objectGrid[y][x];
            if (objectPOD.has_value())
            {
                sf::Vector2f objectPos;
                objectPos.x = worldPosition.x + (x + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
                objectPos.y = worldPosition.y + (y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

                std::unique_ptr<BuildableObject> object = BuildableObjectFactory::create(objectPos, objectPOD->objectType, &game);

                object->loadFromPOD(objectPOD.value());

                objectGrid[y][x] = std::move(object);
            }
            else
            {
                objectGrid[y][x] = nullptr;
            }
        }
    }

    for (const EntityPOD& entityPOD : pod.entities)
    {
        std::unique_ptr<Entity> entity = std::make_unique<Entity>(sf::Vector2f(0, 0), 0);
        entity->loadFromPOD(entityPOD, worldPosition);
        entities.push_back(std::move(entity));
    }

    if (pod.structureObject.has_value())
    {
        StructureObject structure(sf::Vector2f(0, 0), 0);
        structure.loadFromPOD(pod.structureObject.value(), worldPosition);
        structureObject = structure;
    }
}

bool Chunk::wasGeneratedFromPOD()
{
    return generatedFromPOD;
}

void Chunk::setWorldPosition(sf::Vector2f position, ChunkManager& chunkManager)
{
    // Update all entity positions
    for (auto& entity : entities)
    {
        // Get position relative to chunk before updating chunk position
        sf::Vector2f relativePosition = entity->getPosition() - worldPosition;
        // Set entity position to new chunk position + relative
        entity->setWorldPosition(position + relativePosition);
    }

    // Update all item pickup positions
    for (auto& itemPickup : itemPickups)
    {
        // Get position relative to chunk before updating chunk position
        sf::Vector2f relativePosition = itemPickup.second.getPosition() - worldPosition;
        // Set entity position to new chunk position + relative
        itemPickup.second.setPosition(position + relativePosition);
    }

    // Update structure position if necessary
    if (structureObject.has_value())
    {
        sf::Vector2f relativePosition = structureObject->getPosition() - worldPosition;
        structureObject->setWorldPosition(position + relativePosition);
    }

    worldPosition = position;

    // float tileSize = ResolutionHandler::getTileSize();

    // Update all object positions
    for (int y = 0; y < objectGrid.size(); y++)
    {
        for (int x = 0; x < objectGrid[0].size(); x++)
        {
            // If no object at position, don't update position
            if (!objectGrid[y][x])
                continue;
            
            // Calculate updated object position
            sf::Vector2f objectPos = worldPosition + sf::Vector2f(x * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED / 2.0f, y * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED / 2.0f);

            objectGrid[y][x]->setWorldPosition(objectPos);
        }
    }

    recalculateCollisionRects(chunkManager, nullptr);
}

sf::Vector2f Chunk::getWorldPosition()
{
    return worldPosition;
}

bool Chunk::getContainsWater()
{
    return containsWater;
}

bool Chunk::hasBeenModified()
{
    return modified;
}

ChunkPosition Chunk::getChunkPosition()
{
    return chunkPosition;
}

std::vector<WorldObject*> Chunk::getEntities()
{
    std::vector<WorldObject*> entities_worldObject;
    for (auto& entity : entities)
    {
        entities_worldObject.push_back(entity.get());
    }
    return entities_worldObject;
}