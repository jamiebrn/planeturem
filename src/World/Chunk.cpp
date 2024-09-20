#include "World/Chunk.hpp"

Chunk::Chunk(ChunkPosition chunkPosition)
{
    this->chunkPosition = chunkPosition;

    containsWater = false;

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
    sf::Vector2i worldNoisePosition = sf::Vector2i(chunkPosition.x, chunkPosition.y) * static_cast<int>(CHUNK_TILE_SIZE);

    // Get adjacent chunk tiles
    Chunk* upChunk = chunkManager.getChunk(ChunkPosition(chunkPosition.x, ((chunkPosition.y - 1) % worldSize + worldSize) % worldSize));
    Chunk* downChunk = chunkManager.getChunk(ChunkPosition(chunkPosition.x, ((chunkPosition.y + 1) % worldSize + worldSize) % worldSize));
    Chunk* leftChunk = chunkManager.getChunk(ChunkPosition(((chunkPosition.x - 1) % worldSize + worldSize) % worldSize, chunkPosition.y));
    Chunk* rightChunk = chunkManager.getChunk(ChunkPosition(((chunkPosition.x + 1) % worldSize + worldSize) % worldSize, chunkPosition.y));

    std::set<int> tileMapsModified;

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
        // FIX: This shit
        generateRandomStructure();
    }

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

            // Set tile without graphics update
            std::set<int> additional_tileMapsModified = chunkManager.setChunkTile(chunkPosition, tileType, sf::Vector2i(x, y), false);
            tileMapsModified.insert(additional_tileMapsModified.begin(), additional_tileMapsModified.end());

            if (objectGrid[y][x].has_value())
                continue;

            // Create random object
            ObjectType objectSpawnType = getRandomObjectToSpawnAtWorldTile(sf::Vector2i(worldNoisePosition.x + x, worldNoisePosition.y + y),
                worldSize, heightNoise, biomeNoise, planetType);

            if (objectSpawnType >= 0)
            {
                setObject(sf::Vector2i(x, y), objectSpawnType, worldSize, chunkManager);
            }
            else
            {
                objectGrid[y][x] = std::nullopt;
            }

            // Create random entity
            EntityType entitySpawnType = getRandomEntityToSpawnAtWorldTile(sf::Vector2i(worldNoisePosition.x + x, worldNoisePosition.y + y),
                worldSize, heightNoise, biomeNoise, planetType);
            
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

    chunkManager.performChunkSetTileUpdate(chunkPosition, tileMapsModified);

    generateVisualEffectTiles(heightNoise, biomeNoise, planetType, worldSize, chunkManager);

    recalculateCollisionRects(chunkManager);
}

void Chunk::generateRandomStructure()
{
    int chance = rand() % 20;

    if (chance > 10)
        return;
    
    // Generate structure
    const StructureData& structureData = StructureDataLoader::getStructureData(0);

    // Read collision bitmask and create dummy objects in required positions
    sf::RenderTexture bitmaskTexture;
    bitmaskTexture.create(structureData.size.x, structureData.size.y);
    bitmaskTexture.clear();
    TextureManager::drawSubTexture(bitmaskTexture, {.type = TextureType::CollisionBitmask}, sf::IntRect(structureData.collisionBitmaskOffset, structureData.size));
    bitmaskTexture.display();

    sf::Image bitmaskImage = bitmaskTexture.getTexture().copyToImage();

    bool spawnedStructureObject = false;

    for (int x = 0; x < structureData.size.x; x++)
    {
        // Iterate over y backwards to place structure object in bottom left of dummy objects
        for (int y = structureData.size.y - 1; y >= 0; y--)
        {
            sf::Color bitmaskColor = bitmaskImage.getPixel(x, y);

            sf::Vector2f tileWorldPos;
            tileWorldPos.x = worldPosition.x + (x + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
            tileWorldPos.y = worldPosition.y + (y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

            // Create actual structure object
            if (!spawnedStructureObject && bitmaskColor != sf::Color(0, 0, 0, 0))
            {
                structureObject = StructureObject(tileWorldPos, 0);

                spawnedStructureObject = true;
            }

            // Collision
            if (bitmaskColor == sf::Color(255, 0, 0))
            {
                objectGrid[y][x] = BuildableObject(tileWorldPos, -10);
            }
        }
    }
}

const BiomeGenData* Chunk::getBiomeGenAtWorldTile(sf::Vector2i worldTile, int worldSize, const FastNoise& biomeNoise, PlanetType planetType)
{
    const PlanetGenData& planetGenData = PlanetGenDataLoader::genPlanetGenData(planetType);

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

ObjectType Chunk::getRandomObjectToSpawnAtWorldTile(sf::Vector2i worldTile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType)
{
    const TileGenData* tileGenData = getTileGenAtWorldTile(worldTile, worldSize, heightNoise, biomeNoise, planetType);

    if (!tileGenData->objectsCanSpawn)
        return -1;

    const BiomeGenData* biomeGenData = getBiomeGenAtWorldTile(worldTile, worldSize, biomeNoise, planetType);

    float cumulativeChance = 0;
    float randomSpawn = static_cast<float>(rand() % 10000) / 10000.0f;
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

EntityType Chunk::getRandomEntityToSpawnAtWorldTile(sf::Vector2i worldTile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise, PlanetType planetType)
{
    const TileGenData* tileGenData = getTileGenAtWorldTile(worldTile, worldSize, heightNoise, biomeNoise, planetType);

    if (!tileGenData->objectsCanSpawn)
        return -1;

    const BiomeGenData* biomeGenData = getBiomeGenAtWorldTile(worldTile, worldSize, biomeNoise, planetType);

    float cumulativeChance = 0;
    float randomSpawn = static_cast<float>(rand() % 10000) / 10000.0f;
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
        
        // If chunk has changed due to offset, check tile in different chunk
        //if (wrappedChunkTile.first != chunk)
        //{
        //    if (chunkManager.isChunkGenerated(wrappedChunkTile.first))
        //    {
        //        return chunkManager.getChunkTileType(wrappedChunkTile.first, wrappedChunkTile.second);
        //    }
        //    else
        //    {
        //        // Take tile type from noise value as chunk has not been generated yet
        //        const TileGenData* tileGenData = getTileGenAtWorldTile(
        //            sf::Vector2i(wrappedChunkTile.first.x * CHUNK_TILE_SIZE + wrappedChunkTile.second.x,
        //                         wrappedChunkTile.first.y * CHUNK_TILE_SIZE + wrappedChunkTile.second.y),
        //            worldSize, heightNoise, biomeNoise, planetType);
        //        
        //        if (!tileGenData)
        //        {
        //            return 0; // tile gen data is nullptr, so tile will not be generated there, i.e. water
        //        }

        //        return tileGenData->tileID; // return non-zero value by default (non-water tile)
        //    }
        //}
        //else
        //{
        //    return static_cast<int>(this->groundTileGrid[wrappedChunkTile.second.y][wrappedChunkTile.second.x]);
        //}
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
    // float tileSize = ResolutionHandler::getTileSize();

    // sf::Vector2f worldPosition = static_cast<sf::Vector2f>(chunkPosition) * 8.0f * tileSize;
    
    // Draw terrain tiles
    // sf::Transform transform;
    // transform.translate(Camera::worldToScreenTransform(worldPosition));
    // transform.scale(scale, scale);

    // sf::RenderStates state;
    // state.texture = TextureManager::getTexture(TextureType::GroundTiles);
    // state.transform = transform;
    // window.draw(groundVertexArray, state);

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
            std::optional<BuildableObject>& object = object_row[x];
            if (object.has_value())
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
            if (!objectGrid[y][x].has_value())
                continue;

            // If object is object reference, do not draw
            if (objectGrid[y][x]->isObjectReference())
                continue;
            
            // Is valid object

            // Add object to vector
            objects.push_back(&objectGrid[y][x].value());
        }
    }

    // Structure object if required
    if (structureObject.has_value())
    {
        objects.push_back(&structureObject.value());
    }

    return objects;
}

std::optional<BuildableObject>& Chunk::getObject(sf::Vector2i position)
{
    return objectGrid[position.y][position.x];
}

int Chunk::getTileType(sf::Vector2i position) const
{
    return groundTileGrid[position.y][position.x];
}

void Chunk::setObject(sf::Vector2i position, ObjectType objectType, int worldSize, ChunkManager& chunkManager)
{
    // Get tile size
    // float tileSize = ResolutionHandler::getTileSize();

    // sf::Vector2f worldPosition = static_cast<sf::Vector2f>(chunkPosition) * 8.0f * tileSize;
    sf::Vector2f objectPos;
    objectPos.x = worldPosition.x + position.x * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED / 2.0f;
    objectPos.y = worldPosition.y + position.y * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED / 2.0f;

    // std::unique_ptr<BuildableObject> object = std::make_unique<BuildableObject>(objectPos, objectType);
    BuildableObject object(objectPos, objectType);

    sf::Vector2i objectSize(1, 1);
    if (objectType >= 0)
    {
        objectSize = ObjectDataLoader::getObjectData(objectType).size;
    }

    // Set object in chunk
    objectGrid[position.y][position.x] = object;

    // Create object reference objects if object is larger than one tile
    if (objectSize != sf::Vector2i(1, 1))
    {
        ObjectReference objectReference;
        objectReference.chunk = ChunkPosition(chunkPosition.x, chunkPosition.y);
        objectReference.tile = position;

        // Iterate over all tiles which object occupies and add object reference
        for (int y = position.y; y < std::min(position.y + objectSize.y, (int)objectGrid.size()); y++)
        {
            for (int x = position.x; x < std::min(position.x + objectSize.x, (int)objectGrid[0].size()); x++)
            {
                // If tile is actual object tile, don't place object reference
                if (x == position.x && y == position.y)
                    continue;
                
                objectGrid[y][x] = BuildableObject(objectReference);
            }
        }

        // Calculate remaining tiles to place, if placed across multiple chunks
        int x_remaining = objectSize.x - ((int)objectGrid[0].size() - 1 - position.x) - 1;
        int y_remaining = objectSize.y - ((int)objectGrid.size() - 1 - position.y) - 1;

        // Calculate next chunk index
        int chunkNextPosX = (((chunkPosition.x + 1) % worldSize) + worldSize) % worldSize;
        int chunkNextPosY = (((chunkPosition.y + 1) % worldSize) + worldSize) % worldSize;

        // Add tiles to right (direction) chunk if required
        for (int y = position.y; y < std::min(position.y + objectSize.y, (int)objectGrid.size()); y++)
        {
            for (int x = 0; x < x_remaining; x++)
            {
                chunkManager.setObjectReference(ChunkPosition(chunkNextPosX, chunkPosition.y), objectReference, sf::Vector2i(x, y));
            }
        }

        // Add tiles to down (direction) chunk if required
        for (int y = 0; y < y_remaining; y++)
        {
            for (int x = position.x; x < std::min(position.x + objectSize.x, (int)objectGrid[0].size()); x++)
            {
                chunkManager.setObjectReference(ChunkPosition(chunkPosition.x, chunkNextPosY), objectReference, sf::Vector2i(x, y));
            }
        }

        // Add tiles to down-right (direction) chunk if required
        for (int y = 0; y < y_remaining; y++)
        {
            for (int x = 0; x < x_remaining; x++)
            {
                chunkManager.setObjectReference(ChunkPosition(chunkNextPosX, chunkNextPosY), objectReference, sf::Vector2i(x, y));
            }
        }
    }

    recalculateCollisionRects(chunkManager);
}

void Chunk::deleteObject(sf::Vector2i position, ChunkManager& chunkManager)
{
    // Get size of object to handle different deletion cases
    ObjectType objectType = objectGrid[position.y][position.x]->getObjectType();

    sf::Vector2i objectSize(1, 1);
    if (objectType >= 0)
    {
        const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
        sf::Vector2i objectSize = objectData.size;
    }

    // Object is single tile
    if (objectSize == sf::Vector2i(1, 1))
    {
        objectGrid[position.y][position.x].reset();
        recalculateCollisionRects(chunkManager);
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
            chunkManager.deleteObject(ChunkPosition(chunkPosition.x + 1, chunkPosition.y), sf::Vector2i(x, y));
        }
    }

    // Delete tiles to down (direction) chunk if required
    for (int y = 0; y < y_remaining; y++)
    {
        for (int x = position.x; x < std::min(position.x + objectSize.x, (int)objectGrid[0].size()); x++)
        {
            chunkManager.deleteObject(ChunkPosition(chunkPosition.x, chunkPosition.y + 1), sf::Vector2i(x, y));
        }
    }

    // Delete tiles to down-right (direction) chunk if required
    for (int y = 0; y < y_remaining; y++)
    {
        for (int x = 0; x < x_remaining; x++)
        {
            chunkManager.deleteObject(ChunkPosition(chunkPosition.x + 1, chunkPosition.y + 1), sf::Vector2i(x, y));
        }
    }

    recalculateCollisionRects(chunkManager);
}

void Chunk::setObjectReference(const ObjectReference& objectReference, sf::Vector2i tile, ChunkManager& chunkManager)
{
    objectGrid[tile.y][tile.x] = BuildableObject(objectReference);

    recalculateCollisionRects(chunkManager);
}

bool Chunk::canPlaceObject(sf::Vector2i position, ObjectType objectType, int worldSize, ChunkManager& chunkManager)
{
    // Get data of object type to test
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    // Test all tiles taken up by object

    // Test tiles within current chunk
    for (int y = position.y; y < std::min(position.y + objectData.size.y, (int)objectGrid.size()); y++)
    {
        for (int x = position.x; x < std::min(position.x + objectData.size.x, (int)objectGrid[0].size()); x++)
        {
            // Test object
            if (objectGrid[y][x].has_value())
                return false;

            // Test tile
            if (objectData.placeOnWater)
            {
                return getTileType(sf::Vector2i(x, y)) == 0;
            }

            if (getTileType(sf::Vector2i(x, y)) == 0)
                return false;
        }
    }

    // Calculate remaining tiles to test, if placed across multiple chunks
    int x_remaining = objectData.size.x - ((int)objectGrid[0].size() - 1 - position.x) - 1;
    int y_remaining = objectData.size.y - ((int)objectGrid.size() - 1 - position.y) - 1;

    // Calculate next chunk index
    int chunkNextPosX = (((chunkPosition.x + 1) % worldSize) + worldSize) % worldSize;
    int chunkNextPosY = (((chunkPosition.y + 1) % worldSize) + worldSize) % worldSize;

    // Test tiles to right (direction) chunk if required
    for (int y = position.y; y < std::min(position.y + objectData.size.y, (int)objectGrid.size()); y++)
    {
        for (int x = 0; x < x_remaining; x++)
        {
            // Test object
            std::optional<BuildableObject>& objectOptional = chunkManager.getChunkObject(ChunkPosition(chunkNextPosX, chunkPosition.y), sf::Vector2i(x, y));
            if (objectOptional.has_value())
                return false;

            // Test tile
            if (objectData.placeOnWater)
            {
                return (chunkManager.getLoadedChunkTileType(ChunkPosition(chunkNextPosX, chunkPosition.y), sf::Vector2i(x, y)) == 0);
            }
            
            if (chunkManager.getLoadedChunkTileType(ChunkPosition(chunkNextPosX, chunkPosition.y), sf::Vector2i(x, y)) == 0)
                return false;
        }
    }

    // Test tiles to down (direction) chunk if required
    for (int y = 0; y < y_remaining; y++)
    {
        for (int x = position.x; x < std::min(position.x + objectData.size.x, (int)objectGrid[0].size()); x++)
        {
            // Test object
            std::optional<BuildableObject>& objectOptional = chunkManager.getChunkObject(ChunkPosition(chunkPosition.x, chunkNextPosY), sf::Vector2i(x, y));
            if (objectOptional.has_value())
                return false;

            // Test tile
            if (objectData.placeOnWater)
            {
                return (chunkManager.getLoadedChunkTileType(ChunkPosition(chunkPosition.x, chunkNextPosY), sf::Vector2i(x, y)) == 0);
            }

            if (chunkManager.getLoadedChunkTileType(ChunkPosition(chunkPosition.x, chunkNextPosY), sf::Vector2i(x, y)) == 0)
                return false;
        }
    }

    // Test tiles to down-right (direction) chunk if required
    for (int y = 0; y < y_remaining; y++)
    {
        for (int x = 0; x < x_remaining; x++)
        {
            // Test object
            std::optional<BuildableObject>& objectOptional = chunkManager.getChunkObject(ChunkPosition(chunkNextPosX, chunkNextPosY), sf::Vector2i(x, y));
            if (objectOptional.has_value())
                return false;
            
            // Test tile
            if (objectData.placeOnWater)
            {
                return (chunkManager.getLoadedChunkTileType(ChunkPosition(chunkNextPosX, chunkNextPosY), sf::Vector2i(x, y)) == 0);
            }

            if (chunkManager.getLoadedChunkTileType(ChunkPosition(chunkNextPosX, chunkNextPosY), sf::Vector2i(x, y)) == 0)
                return false;
        }
    }
    
    // Passed all tests, so return true
    return true;
}

void Chunk::updateChunkEntities(float dt, int worldSize, ChunkManager& chunkManager)
{
    // float tileSize = ResolutionHandler::getTileSize();

    // Get world collision rects
    // std::vector<CollisionRect*> worldCollisionRects = chunkManager.getChunkCollisionRects();

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

    // Get tile size
    // float tileSize = ResolutionHandler::getTileSize();

    // sf::Vector2f worldPosition = static_cast<sf::Vector2f>(chunkPosition) * 8.0f * tileSize;

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

            std::optional<BuildableObject>& objectOptional = objectGrid[y][x];
            if (objectOptional.has_value())
            {
                int objectType = objectOptional.value().getObjectType();
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
            if (!objectGrid[y][x].has_value())
                continue;

            // Go through chunk manager in case of object reference
            std::optional<BuildableObject>& objectOptional = chunkManager.getChunkObject(chunkPosition, sf::Vector2i(x, y));

            if (!objectOptional.has_value())
                continue;
            
            if (objectOptional->getObjectType() == -10)
            {
                createCollisionRect(collisionRects, x, y);
                continue;
            }

            const ObjectData& objectData = ObjectDataLoader::getObjectData(objectOptional->getObjectType());
            
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
    if (objectGrid[tile.y][tile.x].has_value())
        return false;
    
    if (getTileType(tile) != 0)
        return false;
    
    return true;
}

void Chunk::placeLand(sf::Vector2i tile, int worldSize, const FastNoise& heightNoise, const FastNoise& biomeNoise,
    PlanetType planetType, ChunkManager& chunkManager)
{
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

    // setTile(tileGenData.tileID, tile, upChunk, downChunk, leftChunk, rightChunk);

    // Set tile
    // groundTileGrid[tile.y][tile.x] = TileType::Sand;

    // TODO: Make setting tiles modular / standardised

    // Set vertex array
    // int vertexArrayIndex = (tile.x + tile.y * CHUNK_TILE_SIZE) * 4;

    // int textureVariation = rand() % 3;
    // groundVertexArray[vertexArrayIndex].texCoords = {2 * 16, 0 + textureVariation * 16.0f};
    // groundVertexArray[vertexArrayIndex + 1].texCoords = {2 * 16 + 16, 0 + textureVariation * 16.0f};
    // groundVertexArray[vertexArrayIndex + 3].texCoords = {2 * 16, 16 + textureVariation * 16.0f};
    // groundVertexArray[vertexArrayIndex + 2].texCoords = {2 * 16 + 16, 16 + textureVariation * 16.0f};

    // groundVertexArray[vertexArrayIndex].color = sf::Color(255, 255, 255, 255);
    // groundVertexArray[vertexArrayIndex + 1].color = sf::Color(255, 255, 255, 255);
    // groundVertexArray[vertexArrayIndex + 3].color = sf::Color(255, 255, 255, 255);
    // groundVertexArray[vertexArrayIndex + 2].color = sf::Color(255, 255, 255, 255);

    chunkManager.setChunkTile(chunkPosition, tileGenData.tileID, tile);

    // Update visual tiles
    generateVisualEffectTiles(heightNoise, biomeNoise, planetType, worldSize, chunkManager);

    // Recalculate collision rects
    recalculateCollisionRects(chunkManager);
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
        structureObject->setPosition(position + relativePosition);
    }

    worldPosition = position;

    // float tileSize = ResolutionHandler::getTileSize();

    // Update all object positions
    for (int y = 0; y < objectGrid.size(); y++)
    {
        for (int x = 0; x < objectGrid[0].size(); x++)
        {
            // If no object at position, don't update position
            if (!objectGrid[y][x].has_value())
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

std::vector<WorldObject*> Chunk::getEntities()
{
    std::vector<WorldObject*> entities_worldObject;
    for (auto& entity : entities)
    {
        entities_worldObject.push_back(entity.get());
    }
    return entities_worldObject;
}