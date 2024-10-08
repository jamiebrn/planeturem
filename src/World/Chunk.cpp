#include "World/Chunk.hpp"

Chunk::Chunk(ChunkPosition chunkPosition)
{
    this->chunkPosition = chunkPosition;

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
}

void Chunk::generateChunk(const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType, int worldSize, ChunkManager& chunkManager)
{
    RandInt randGen = generateTilesAndStructure(heightNoise, biomeNoise, planetType, worldSize, chunkManager);

    generateObjectsAndEntities(heightNoise, biomeNoise, planetType, randGen, worldSize, chunkManager);

    generateTilemapsAndInit(heightNoise, biomeNoise, planetType, worldSize, chunkManager);
}

RandInt Chunk::generateTilesAndStructure(const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType, int worldSize, ChunkManager& chunkManager)
{
    sf::Vector2i worldNoisePosition = sf::Vector2i(chunkPosition.x, chunkPosition.y) * static_cast<int>(CHUNK_TILE_SIZE);

    // Create random generator for chunk
    unsigned long int randSeed = chunkManager.getSeed() ^ chunkPosition.hash();
    RandInt randGen(randSeed);

    // Store tile types in tile array
    for (int y = 0; y < CHUNK_TILE_SIZE; y++)
    {
        for (int x = 0; x < CHUNK_TILE_SIZE; x++)
        {
            const TileGenData* tileGenData = getTileGenAtWorldTile(
                sf::Vector2i(worldNoisePosition.x + x, worldNoisePosition.y + y), worldSize, heightNoise, biomeNoise, planetType
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
        generateRandomStructure(worldSize, biomeNoise, randGen, planetType);
    }

    return randGen;
}

void Chunk::generateObjectsAndEntities(const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType, RandInt& randGen,
    int worldSize, ChunkManager& chunkManager)
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
                worldSize, heightNoise, biomeNoise, randGen, planetType);

            if (objectSpawnType >= 0)
            {
                setObject(sf::Vector2i(x, y), objectSpawnType, worldSize, chunkManager, false, true);
            }
            else
            {
                objectGrid[y][x] = nullptr;
            }

            // Create random entity
            EntityType entitySpawnType = getRandomEntityToSpawnAtWorldTile(sf::Vector2i(worldNoisePosition.x + x, worldNoisePosition.y + y),
                worldSize, heightNoise, biomeNoise, randGen, planetType);
            
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

void Chunk::generateTilemapsAndInit(const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType, int worldSize, ChunkManager& chunkManager)
{
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

    generateVisualEffectTiles(heightNoise, biomeNoise, planetType, worldSize, chunkManager);

    recalculateCollisionRects(chunkManager);

    generatedFromPOD = false;
}

void Chunk::generateRandomStructure(int worldSize, const FastNoise& biomeNoise, RandInt& randGen, PlanetType planetType)
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

            // Create actual structure object
            if (!spawnedStructureObject && bitmaskColor != sf::Color(0, 0, 0, 0))
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

const TileGenData* Chunk::getTileGenAtWorldTile(sf::Vector2i worldTile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType)
{
    const BiomeGenData* biomeGenData = getBiomeGenAtWorldTile(worldTile, worldSize, biomeNoise, planetType);
    
    if (!biomeGenData)
        return nullptr;
    
    float heightNoiseValue = heightNoise.GetNoiseSeamless2D(worldTile.x, worldTile.y, worldSize * CHUNK_TILE_SIZE, worldSize * CHUNK_TILE_SIZE);
    heightNoiseValue = FastNoise::Normalise(heightNoiseValue);
    
    for (const TileGenData& tileGenData : biomeGenData->tileGenDatas)
    {
        if (tileGenData.noiseRangeMin <= heightNoiseValue && tileGenData.noiseRangeMax >= heightNoiseValue)
        {
            return &tileGenData;
        }
    }
    
    return nullptr;
}

ObjectType Chunk::getRandomObjectToSpawnAtWorldTile(sf::Vector2i worldTile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise,
    RandInt& randGen, PlanetType planetType)
{
    const TileGenData* tileGenData = getTileGenAtWorldTile(worldTile, worldSize, heightNoise, biomeNoise, planetType);

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
    RandInt& randGen, PlanetType planetType)
{
    const TileGenData* tileGenData = getTileGenAtWorldTile(worldTile, worldSize, heightNoise, biomeNoise, planetType);

    if (!tileGenData->objectsCanSpawn)
        return -1;

    const BiomeGenData* biomeGenData = getBiomeGenAtWorldTile(worldTile, worldSize, biomeNoise, planetType);

    float cumulativeChance = 0;
    float randomSpawn = static_cast<float>(randGen.generate(0, 10000)) / 10000.0f;
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

void Chunk::generateVisualEffectTiles(const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType, int worldSize, ChunkManager& chunkManager)
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
                                ChunkManager& chunkManager, const FastNoise& heightNoise, const FastNoise& biomeNoise,
                                PlanetType planetType, int worldSize)
    {
        std::pair<ChunkPosition, sf::Vector2i> wrappedChunkTile = ChunkManager::getChunkTileFromOffset(chunk, tile, xOffset, yOffset, worldSize);

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
                getTileTypeAt(chunkPosition, tilePos, -1, -1, chunkManager, heightNoise, biomeNoise, planetType, worldSize),
                getTileTypeAt(chunkPosition, tilePos, 0, -1, chunkManager, heightNoise, biomeNoise, planetType, worldSize),
                getTileTypeAt(chunkPosition, tilePos, 1, -1, chunkManager, heightNoise, biomeNoise, planetType, worldSize),
                getTileTypeAt(chunkPosition, tilePos, -1, 0, chunkManager, heightNoise, biomeNoise, planetType, worldSize),
                getTileTypeAt(chunkPosition, tilePos, 1, 0, chunkManager, heightNoise, biomeNoise, planetType, worldSize),
                getTileTypeAt(chunkPosition, tilePos, -1, 1, chunkManager, heightNoise, biomeNoise, planetType, worldSize),
                getTileTypeAt(chunkPosition, tilePos, 0, 1, chunkManager, heightNoise, biomeNoise, planetType, worldSize),
                getTileTypeAt(chunkPosition, tilePos, 1, 1, chunkManager, heightNoise, biomeNoise, planetType, worldSize)
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

void Chunk::drawChunkTerrain(sf::RenderTarget& window, float time)
{
    // Get tile size and scale
    float scale = ResolutionHandler::getScale();

    for (auto iter = tileMaps.begin(); iter != tileMaps.end(); ++iter)
    {
        if (!DebugOptions::tileMapsVisible[iter->first])
            continue;

        iter->second.draw(window, Camera::worldToScreenTransform(worldPosition), sf::Vector2f(scale, scale));
    }


    // DEBUG DRAW LINE TO ENTITIES
    if (DebugOptions::drawEntityChunkParents)
    {
        for (auto& entity : entities)
        {
            sf::VertexArray lines(sf::Lines, 2);
            lines[0].position = Camera::worldToScreenTransform(worldPosition);
            lines[1].position = Camera::worldToScreenTransform(entity->getPosition());
            window.draw(lines);
        }
    }

    // DEBUG CHUNK OUTLINE DRAW
    if (DebugOptions::drawChunkBoundaries)
    {
        sf::VertexArray lines(sf::Lines, 8);
        lines[0].position = Camera::worldToScreenTransform(worldPosition);
        lines[1].position = Camera::worldToScreenTransform(worldPosition + sf::Vector2f(CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED, 0));
        lines[2].position = Camera::worldToScreenTransform(worldPosition);
        lines[3].position = Camera::worldToScreenTransform(worldPosition + sf::Vector2f(0, CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED));
        lines[4].position = Camera::worldToScreenTransform(worldPosition + sf::Vector2f(CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED, 0));
        lines[5].position = Camera::worldToScreenTransform(worldPosition + sf::Vector2f(CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED, CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED));
        lines[6].position = Camera::worldToScreenTransform(worldPosition + sf::Vector2f(0, CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED));
        lines[7].position = Camera::worldToScreenTransform(worldPosition + sf::Vector2f(CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED, CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED));
        
        window.draw(lines);
    }

    // DRAW COLLISIONS
    if (DebugOptions::drawCollisionRects)
    {
        for (auto& collisionRect : collisionRects)
        {
            collisionRect->debugDraw(window);
        }
    }
}

void Chunk::drawChunkTerrainVisual(sf::RenderTarget& window, SpriteBatch& spriteBatch, float time)
{
    // Get tile size and scale
    float scale = ResolutionHandler::getScale();
    // float tileSize = ResolutionHandler::getTileSize();

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
                    textureRect = sf::IntRect(64 + cliffWaterFrame * 16, 0, 16, 16);
                    tileWorldPosition.y -= 2;
                    break;
                case TileType::Visual_LCliff:
                    textureRect = sf::IntRect(64 + cliffWaterFrame * 16, 16, 16, 16);
                    tileWorldPosition.y -= 2;
                    break;
                case TileType::Visual_RCliff:
                    textureRect = sf::IntRect(64 + cliffWaterFrame * 16, 32, 16, 16);
                    tileWorldPosition.y -= 2;
                    break;
                case TileType::Visual_LRCliff:
                    textureRect = sf::IntRect(64 + cliffWaterFrame * 16, 48, 16, 16);
                    tileWorldPosition.y -= 2;
                    break;
            }

            // sf::Shader* cliffShader = Shaders::getShader(ShaderType::Cliff);

            spriteBatch.draw(window, {TextureType::GroundTiles, Camera::worldToScreenTransform(tileWorldPosition), 0, {scale, scale}},
                textureRect);

            // TextureManager::drawSubTexture(window, {TextureType::GroundTiles, Camera::worldToScreenTransform(tileWorldPosition), 0, {scale, scale}}, textureRect);
        }
    }
}

void Chunk::drawChunkWater(sf::RenderTarget& window)
{
    // Get scale
    float scale = ResolutionHandler::getScale();

    // Draw water
    sf::Vector2f waterPos = Camera::worldToScreenTransform(worldPosition);
    sf::IntRect waterRect(0, 0, TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE, TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE);

    sf::Shader* waterShader = Shaders::getShader(ShaderType::Water);

    TextureManager::drawSubTexture(window, {TextureType::Water, waterPos, 0, {scale, scale}}, waterRect, waterShader);
}

void Chunk::updateChunkObjects(float dt, int worldSize, ChunkManager& chunkManager)
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
                
                object->update(dt, onWater);
                if (!object->isAlive())
                    deleteObject(sf::Vector2i(x, y), chunkManager);
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

void Chunk::setObject(sf::Vector2i position, ObjectType objectType, int worldSize, ChunkManager& chunkManager, bool recalculateCollision, bool calledWhileGenerating)
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
    objectGrid[position.y][position.x] = BuildableObjectFactory::create(objectPos, objectType);

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

                auto chunkTile = ChunkManager::getChunkTileFromOffset(chunkPosition, position, x, y, worldSize);
                chunkManager.setObjectReference(chunkTile.first, objectReference, chunkTile.second);
            }
        }
    }

    if (recalculateCollision)
    {
        recalculateCollisionRects(chunkManager);
    }
}

void Chunk::deleteObject(sf::Vector2i position, ChunkManager& chunkManager)
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
        deleteSingleObject(position, chunkManager);
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

void Chunk::deleteSingleObject(sf::Vector2i position, ChunkManager& chunkManager)
{
    objectGrid[position.y][position.x].reset();
    recalculateCollisionRects(chunkManager);
}

void Chunk::setObjectReference(const ObjectReference& objectReference, sf::Vector2i tile, ChunkManager& chunkManager)
{
    modified = true;

    objectGrid[tile.y][tile.x] = std::make_unique<BuildableObject>(objectReference);

    recalculateCollisionRects(chunkManager);
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

void Chunk::updateChunkEntities(float dt, int worldSize, ChunkManager& chunkManager)
{
    for (std::vector<std::unique_ptr<Entity>>::iterator entityIter = entities.begin(); entityIter != entities.end();)
    {
        std::unique_ptr<Entity>& entity = *entityIter;

        // Determine whether on water
        bool onWater = (getTileType(entity->getChunkTileInside(worldSize)) == 0);

        entity->update(dt, chunkManager, onWater);

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

void Chunk::recalculateCollisionRects(ChunkManager& chunkManager)
{
    // Clear previously calculated collision rects
    collisionRects.clear();

    auto createCollisionRect = [this](std::vector<std::unique_ptr<CollisionRect>>& rects, int x, int y) -> void
    {
        std::unique_ptr<CollisionRect> collisionRect = std::make_unique<CollisionRect>();

        collisionRect->x = this->worldPosition.x + x * TILE_SIZE_PIXELS_UNSCALED;
        collisionRect->y = this->worldPosition.y + y * TILE_SIZE_PIXELS_UNSCALED;
        collisionRect->width = TILE_SIZE_PIXELS_UNSCALED;
        collisionRect->height = TILE_SIZE_PIXELS_UNSCALED;

        rects.push_back(std::move(collisionRect));
    };

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
                createCollisionRect(collisionRects, x, y);
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
                }
                continue;
            }

            const ObjectData& objectData = ObjectDataLoader::getObjectData(object->getObjectType());
            
            if (objectData.hasCollision)
                createCollisionRect(collisionRects, x, y);
        }
    }
}

std::vector<CollisionRect*> Chunk::getCollisionRects()
{
    std::vector<CollisionRect*> collisionRectPtrs;
    for (auto& collisionRect : collisionRects)
    {
        collisionRectPtrs.push_back(collisionRect.get());
    }
    return collisionRectPtrs;
}

bool Chunk::collisionRectStaticCollisionX(CollisionRect& collisionRect, float dx)
{
    bool collision = false;
    for (auto& rect : collisionRects)
    {
        if (collisionRect.handleStaticCollisionX(*rect, dx))
            collision = true;
    }
    return collision;
}

bool Chunk::collisionRectStaticCollisionY(CollisionRect& collisionRect, float dy)
{
    bool collision = false;
    for (auto& rect : collisionRects)
    {
        if (collisionRect.handleStaticCollisionY(*rect, dy))
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
    PlanetType planetType, ChunkManager& chunkManager)
{
    modified = true;

    Chunk* upChunk = chunkManager.getChunk(ChunkPosition(chunkPosition.x, ((chunkPosition.y - 1) % worldSize + worldSize) % worldSize));
    Chunk* downChunk = chunkManager.getChunk(ChunkPosition(chunkPosition.x, ((chunkPosition.y + 1) % worldSize + worldSize) % worldSize));
    Chunk* leftChunk = chunkManager.getChunk(ChunkPosition(((chunkPosition.x - 1) % worldSize + worldSize) % worldSize, chunkPosition.y));
    Chunk* rightChunk = chunkManager.getChunk(ChunkPosition(((chunkPosition.x + 1) % worldSize + worldSize) % worldSize, chunkPosition.y));

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
    generateVisualEffectTiles(heightNoise, biomeNoise, planetType, worldSize, chunkManager);

    // Recalculate collision rects
    recalculateCollisionRects(chunkManager);
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

void Chunk::loadFromChunkPOD(const ChunkPOD& pod)
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
                objectPos.x = worldPosition.x + (x + 0.5f) * CHUNK_TILE_SIZE;
                objectPos.y = worldPosition.y + (y + 0.5f) * CHUNK_TILE_SIZE;

                std::unique_ptr<BuildableObject> object = BuildableObjectFactory::create(objectPos, objectPOD->objectType);

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

    recalculateCollisionRects(chunkManager);
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

std::vector<WorldObject*> Chunk::getEntities()
{
    std::vector<WorldObject*> entities_worldObject;
    for (auto& entity : entities)
    {
        entities_worldObject.push_back(entity.get());
    }
    return entities_worldObject;
}