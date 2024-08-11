#include "World/Chunk.hpp"

Chunk::Chunk(sf::Vector2i worldGridPosition)
{
    this->worldGridPosition = worldGridPosition;
    groundVertexArray = sf::VertexArray(sf::Quads, 8 * 8 * 4);
}

void Chunk::generateChunk(const FastNoise& noise, int worldSize, ChunkManager& chunkManager)
{
    // Get tile size
    // float tileSize = ResolutionHandler::getTileSize();
    
    sf::Vector2f worldNoisePosition = static_cast<sf::Vector2f>(worldGridPosition) * CHUNK_TILE_SIZE;

    float noiseSize = CHUNK_TILE_SIZE * worldSize;

    // std::array<std::array<sf::Uint8, 8 * 4>, 8> heightMapData;

    bool chunkHasWater = false;

    for (int y = 0; y < CHUNK_TILE_SIZE; y++)
    {
        for (int x = 0; x < CHUNK_TILE_SIZE; x++)
        {
            float height = noise.GetNoiseSeamless2D(worldNoisePosition.x + (float)x, worldNoisePosition.y + (float)y, noiseSize, noiseSize);
            height = FastNoise::Normalise(height);
            // std::cout << height << std::endl;
            // static const float sqrt2Div2 = 0.70710678119f;
            // float heightNormalised = (height - sqrt2Div2) / (sqrt2Div2 * 2);

            TileType tileType = getTileTypeFromNoiseHeight(height);

            groundTileGrid[y][x] = tileType;

            if (tileType == TileType::Water)
            {
                chunkHasWater = true;
                continue;
            }

            int vertexArrayIndex = (x + y * CHUNK_TILE_SIZE) * 4;
            groundVertexArray[vertexArrayIndex].position = sf::Vector2f(x * TILE_SIZE_PIXELS_UNSCALED, y * TILE_SIZE_PIXELS_UNSCALED);
            groundVertexArray[vertexArrayIndex + 1].position = sf::Vector2f(x * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED, y * TILE_SIZE_PIXELS_UNSCALED);
            groundVertexArray[vertexArrayIndex + 3].position = sf::Vector2f(x * TILE_SIZE_PIXELS_UNSCALED, y * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED);
            groundVertexArray[vertexArrayIndex + 2].position = sf::Vector2f(x * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED, y * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED);

            int textureVariation;

            switch (tileType)
            {
                case TileType::DarkGrass:
                    textureVariation = rand() % 3;
                    groundVertexArray[vertexArrayIndex].texCoords = {1 * 16, 0 + textureVariation * 16.0f};
                    groundVertexArray[vertexArrayIndex + 1].texCoords = {1 * 16 + 16, 0 + textureVariation * 16.0f};
                    groundVertexArray[vertexArrayIndex + 3].texCoords = {1 * 16, 16 + textureVariation * 16.0f};
                    groundVertexArray[vertexArrayIndex + 2].texCoords = {1 * 16 + 16, 16 + textureVariation * 16.0f};
                    break;
                case TileType::Sand:
                    textureVariation = rand() % 3;
                    groundVertexArray[vertexArrayIndex].texCoords = {2 * 16, 0 + textureVariation * 16.0f};
                    groundVertexArray[vertexArrayIndex + 1].texCoords = {2 * 16 + 16, 0 + textureVariation * 16.0f};
                    groundVertexArray[vertexArrayIndex + 3].texCoords = {2 * 16, 16 + textureVariation * 16.0f};
                    groundVertexArray[vertexArrayIndex + 2].texCoords = {2 * 16 + 16, 16 + textureVariation * 16.0f};
                    break;
                case TileType::Grass:
                    textureVariation = rand() % 4;
                    groundVertexArray[vertexArrayIndex].texCoords = {0 * 16, 0 + textureVariation * 16.0f};
                    groundVertexArray[vertexArrayIndex + 1].texCoords = {0 * 16 + 16, 0 + textureVariation * 16.0f};
                    groundVertexArray[vertexArrayIndex + 3].texCoords = {0 * 16, 16 + textureVariation * 16.0f};
                    groundVertexArray[vertexArrayIndex + 2].texCoords = {0 * 16 + 16, 16 + textureVariation * 16.0f};
                    break;
            }

            objectGrid[y][x] = std::nullopt;

            bool canSpawnObject = tileType == TileType::Grass || tileType == TileType::DarkGrass;
            if (!canSpawnObject)
                continue;

            int spawn_chance = rand() % 40;
            sf::Vector2f objectPos = worldPosition + sf::Vector2f(x * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED / 2.0f, y * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED / 2.0f);

            if (spawn_chance < 4)
            {
                // Make tree
                objectGrid[y][x] = BuildableObject(objectPos, 0);
            }
            else if (spawn_chance == 4)
            {
                // Make bush
                objectGrid[y][x] = BuildableObject(objectPos, 1);
            }
            else if (spawn_chance == 5)
            {
                // Make rock
                objectGrid[y][x] = BuildableObject(objectPos, 4);
            }
        }
    }

    // Spawn entities
    int spawnEnemyChance = rand() % 10;

    if (spawnEnemyChance < 3 && !chunkHasWater)
    {
        int entityCount = rand() % 3;
        for (int i = 0; i < entityCount; i++)
        {
            sf::Vector2f spawnPos;
            spawnPos.x = worldPosition.x + rand() % (static_cast<int>(TILE_SIZE_PIXELS_UNSCALED) * static_cast<int>(CHUNK_TILE_SIZE));
            spawnPos.y = worldPosition.y + rand() % (static_cast<int>(TILE_SIZE_PIXELS_UNSCALED) * static_cast<int>(CHUNK_TILE_SIZE));
            unsigned int entityType = rand() % 2;
            std::unique_ptr<Entity> entity = std::make_unique<Entity>(spawnPos, entityType);
            entities.push_back(std::move(entity));
        }
    }

    generateVisualEffectTiles(noise, worldSize, chunkManager);

    recalculateCollisionRects(chunkManager);
}

void Chunk::generateVisualEffectTiles(const FastNoise& noise, int worldSize, ChunkManager& chunkManager)
{
    // Get visual tile from 3x3 area, where centre is tile testing for
    // Assumes centre tile is water
    auto getVisualTileType = [](TileType topLeft,    TileType top,      TileType topRight,
                                TileType left,                          TileType right,
                                TileType bottomLeft, TileType bottom,   TileType bottomRight) -> TileType
    {
        // Cliffs
        if (top != TileType::Water)
        {
            // Straight cliff
            if (topLeft != TileType::Water && topRight != TileType::Water)
                return TileType::Visual_Cliff;
            
            // Left cliff
            if (topLeft == TileType::Water && topRight != TileType::Water)
                return TileType::Visual_LCliff;
            
            // Right cliff
            if (topLeft != TileType::Water && topRight == TileType::Water)
                return TileType::Visual_RCliff;
            
            // Left-right cliff
            if (topLeft == TileType::Water && topRight == TileType::Water)
                return TileType::Visual_LRCliff;
        }

        // Default case
        return TileType::Visual_BLANK;
    };

    auto getTileTypeAt = [this](int x, int y, int offsetX, int offsetY, ChunkManager& chunkManager, const FastNoise& noise, const sf::Vector2i& worldGridPosition, int noiseSize)
    {
        int gridX = x + offsetX;
        int gridY = y + offsetY;
        
        if (gridX < 0 || gridX >= 8 || gridY < 0 || gridY >= 8) {
            ChunkPosition chunkPos(worldGridPosition.x, worldGridPosition.y);
            sf::Vector2i localPos(gridX, gridY);

            // Adjust for chunk boundaries
            if (gridX < 0)
            {
                chunkPos.x -= 1;
                localPos.x = 7;
            }
            else if (gridX >= 8)
            {
                chunkPos.x += 1;
                localPos.x = 0;
            }

            if (gridY < 0)
            {
                chunkPos.y -= 1;
                localPos.y = 7;
            }
            else if (gridY >= 8)
            {
                chunkPos.y += 1;
                localPos.y = 0;
            }

            if (chunkManager.isChunkGenerated(chunkPos))
            {
                return chunkManager.getChunkTileType(chunkPos, localPos);
            }
            else
            {
                float noiseValue = noise.GetNoiseSeamless2D(worldGridPosition.x * 8 + gridX, worldGridPosition.y * 8 + gridY, noiseSize, noiseSize);
                noiseValue = FastNoise::Normalise(noiseValue);
                return getTileTypeFromNoiseHeight(noiseValue);
            }
        }
        else
        {
            return this->groundTileGrid[gridY][gridX];
        }
    };

    float noiseSize = 8.0f * worldSize;

    // Generate visual tiles
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            TileType groundTileType = groundTileGrid[y][x];
            if (groundTileType != TileType::Water)
                continue;

            visualTileGrid[y][x] = getVisualTileType(
                getTileTypeAt(x, y, -1, -1, chunkManager, noise, worldGridPosition, noiseSize), // NW
                getTileTypeAt(x, y, 0, -1, chunkManager, noise, worldGridPosition, noiseSize),  // N
                getTileTypeAt(x, y, 1, -1, chunkManager, noise, worldGridPosition, noiseSize),  // NE
                getTileTypeAt(x, y, -1, 0, chunkManager, noise, worldGridPosition, noiseSize),  // W
                getTileTypeAt(x, y, 1, 0, chunkManager, noise, worldGridPosition, noiseSize),   // E
                getTileTypeAt(x, y, -1, 1, chunkManager, noise, worldGridPosition, noiseSize),  // SW
                getTileTypeAt(x, y, 0, 1, chunkManager, noise, worldGridPosition, noiseSize),   // S
                getTileTypeAt(x, y, 1, 1, chunkManager, noise, worldGridPosition, noiseSize)    // SE
            );
        }
    }
}

TileType Chunk::getTileTypeFromNoiseHeight(float noiseValue)
{
    if (noiseValue < 0.5) return TileType::Water;
    if (noiseValue > 0.7) return TileType::DarkGrass;
    if (noiseValue < 0.53) return TileType::Sand;
    return TileType::Grass;
}

void Chunk::drawChunkTerrain(sf::RenderTarget& window, float time)
{
    // Get tile size and scale
    float scale = ResolutionHandler::getScale();
    float tileSize = ResolutionHandler::getTileSize();

    // sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * tileSize;
    
    // Draw terrain tiles
    sf::Transform transform;
    transform.translate(Camera::worldToScreenTransform(worldPosition));
    transform.scale(scale, scale);

    sf::RenderStates state;
    state.texture = TextureManager::getTexture(TextureType::GroundTiles);
    state.transform = transform;
    window.draw(groundVertexArray, state);


    // DEBUG
    #if DEBUG_DRAW
    // DEBUG DRAW LINE TO ENTITIES
    for (auto& entity : entities)
    {
        sf::VertexArray lines(sf::Lines, 2);
        lines[0].position = Camera::worldToScreenTransform(worldPosition);
        lines[1].position = Camera::worldToScreenTransform(entity->getPosition());
        window.draw(lines);
    }

    // DEBUG CHUNK OUTLINE DRAW
    sf::VertexArray lines(sf::Lines, 8);
    lines[0].position = Camera::worldToScreenTransform(worldPosition); lines[1].position = Camera::worldToScreenTransform(worldPosition) + sf::Vector2f(tileSize * 8, 0);
    lines[2].position = Camera::worldToScreenTransform(worldPosition); lines[3].position = Camera::worldToScreenTransform(worldPosition) + sf::Vector2f(0, tileSize * 8);
    lines[4].position = Camera::worldToScreenTransform(worldPosition) + sf::Vector2f(tileSize * 8, 0); lines[5].position = Camera::worldToScreenTransform(worldPosition) + sf::Vector2f(tileSize * 8, tileSize * 8);
    lines[6].position = Camera::worldToScreenTransform(worldPosition) + sf::Vector2f(0, tileSize * 8); lines[7].position = Camera::worldToScreenTransform(worldPosition) + sf::Vector2f(tileSize * 8, tileSize * 8);

    window.draw(lines);
    #endif
}

void Chunk::drawChunkTerrainVisual(sf::RenderTarget& window, float time)
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

            int cliffWaterFrame = static_cast<int>(std::min(std::max(std::sin(time / 2.0f + worldGridPosition.x + x) * 3.5f / 2.0f + 3.5f / 2.0f, 0.0f), 3.0f));
            
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

            TextureManager::drawSubTexture(window, {TextureType::GroundTiles, Camera::worldToScreenTransform(tileWorldPosition), 0, {scale, scale}}, textureRect);
        }
    }
}

void Chunk::drawChunkWater(sf::RenderTarget& window, float time)
{
    // Get tile size and scale
    float scale = ResolutionHandler::getScale();
    // float tileSize = ResolutionHandler::getTileSize();

    // Draw water
    sf::Vector2f waterPos = Camera::worldToScreenTransform(worldPosition);
    sf::IntRect waterRect(0, 0, TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE, TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE);

    sf::Shader* waterShader = Shaders::getShader(ShaderType::Water);
    waterShader->setUniform("time", time);
    waterShader->setUniform("worldOffset", sf::Vector2f{(float)worldGridPosition.x, (float)worldGridPosition.y});

    TextureManager::drawSubTexture(window, {TextureType::Water, waterPos, 0, {scale, scale}}, waterRect, waterShader);
}

void Chunk::updateChunkObjects(float dt, ChunkManager& chunkManager)
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
                
                object->update(dt);
                if (!object->isAlive())
                    deleteObject(sf::Vector2i(x, y), chunkManager);
            }
        }
    }

    recalculateCollisionRects(chunkManager);
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
    return objects;
}

std::optional<BuildableObject>& Chunk::getObject(sf::Vector2i position)
{
    return objectGrid[position.y][position.x];
}

TileType Chunk::getTileType(sf::Vector2i position) const
{
    return groundTileGrid[position.y][position.x];
}

void Chunk::setObject(sf::Vector2i position, unsigned int objectType, int worldSize, ChunkManager& chunkManager)
{
    // Get tile size
    float tileSize = ResolutionHandler::getTileSize();

    // sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * tileSize;
    sf::Vector2f objectPos = worldPosition + sf::Vector2f(position.x * tileSize + tileSize / 2.0f, position.y * tileSize + tileSize / 2.0f);

    // std::unique_ptr<BuildableObject> object = std::make_unique<BuildableObject>(objectPos, objectType);
    BuildableObject object(objectPos, objectType);

    sf::Vector2i objectSize = ObjectDataLoader::getObjectData(objectType).size;

    // Create object reference objects if object is larger than one tile
    if (objectSize != sf::Vector2i(1, 1))
    {
        ObjectReference objectReference;
        objectReference.chunk = ChunkPosition(worldGridPosition.x, worldGridPosition.y);
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
        int chunkNextPosX = (((worldGridPosition.x + 1) % worldSize) + worldSize) % worldSize;
        int chunkNextPosY = (((worldGridPosition.y + 1) % worldSize) + worldSize) % worldSize;

        // Add tiles to right (direction) chunk if required
        for (int y = position.y; y < std::min(position.y + objectSize.y, (int)objectGrid.size()); y++)
        {
            for (int x = 0; x < x_remaining; x++)
            {
                chunkManager.setObjectReference(ChunkPosition(chunkNextPosX, worldGridPosition.y), objectReference, sf::Vector2i(x, y));
            }
        }

        // Add tiles to down (direction) chunk if required
        for (int y = 0; y < y_remaining; y++)
        {
            for (int x = position.x; x < std::min(position.x + objectSize.x, (int)objectGrid[0].size()); x++)
            {
                chunkManager.setObjectReference(ChunkPosition(worldGridPosition.x, chunkNextPosY), objectReference, sf::Vector2i(x, y));
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

    objectGrid[position.y][position.x] = object;

    recalculateCollisionRects(chunkManager);
}

void Chunk::deleteObject(sf::Vector2i position, ChunkManager& chunkManager)
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
            chunkManager.deleteObject(ChunkPosition(worldGridPosition.x + 1, worldGridPosition.y), sf::Vector2i(x, y));
        }
    }

    // Delete tiles to down (direction) chunk if required
    for (int y = 0; y < y_remaining; y++)
    {
        for (int x = position.x; x < std::min(position.x + objectSize.x, (int)objectGrid[0].size()); x++)
        {
            chunkManager.deleteObject(ChunkPosition(worldGridPosition.x, worldGridPosition.y + 1), sf::Vector2i(x, y));
        }
    }

    // Delete tiles to down-right (direction) chunk if required
    for (int y = 0; y < y_remaining; y++)
    {
        for (int x = 0; x < x_remaining; x++)
        {
            chunkManager.deleteObject(ChunkPosition(worldGridPosition.x + 1, worldGridPosition.y + 1), sf::Vector2i(x, y));
        }
    }

    recalculateCollisionRects(chunkManager);
}

void Chunk::setObjectReference(const ObjectReference& objectReference, sf::Vector2i tile, ChunkManager& chunkManager)
{
    objectGrid[tile.y][tile.x] = BuildableObject(objectReference);

    recalculateCollisionRects(chunkManager);
}

bool Chunk::canPlaceObject(sf::Vector2i position, unsigned int objectType, int worldSize, ChunkManager& chunkManager)
{
    // Get data of object type to test
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    // Test all tiles taken up by object

    // Test tiles within current chunk
    for (int y = position.y; y < std::min(position.y + objectData.size.y, (int)objectGrid.size()); y++)
    {
        for (int x = position.x; x < std::min(position.x + objectData.size.x, (int)objectGrid[0].size()); x++)
        {
            // Test tile
            if (!objectData.waterPlaceable)
            {
                if (getTileType(sf::Vector2i(x, y)) == TileType::Water)
                    return false;
            }
            
            // Test object
            if (objectGrid[y][x].has_value())
                return false;
        }
    }

    // Calculate remaining tiles to test, if placed across multiple chunks
    int x_remaining = objectData.size.x - ((int)objectGrid[0].size() - 1 - position.x) - 1;
    int y_remaining = objectData.size.y - ((int)objectGrid.size() - 1 - position.y) - 1;

    // Calculate next chunk index
    int chunkNextPosX = (((worldGridPosition.x + 1) % worldSize) + worldSize) % worldSize;
    int chunkNextPosY = (((worldGridPosition.y + 1) % worldSize) + worldSize) % worldSize;

    // Test tiles to right (direction) chunk if required
    for (int y = position.y; y < std::min(position.y + objectData.size.y, (int)objectGrid.size()); y++)
    {
        for (int x = 0; x < x_remaining; x++)
        {
            // Test tile
            if (!objectData.waterPlaceable)
            {
                if (chunkManager.getLoadedChunkTileType(ChunkPosition(chunkNextPosX, worldGridPosition.y), sf::Vector2i(x, y)) == TileType::Water)
                    return false;
            }
            
            // Test object
            std::optional<BuildableObject>& objectOptional = chunkManager.getChunkObject(ChunkPosition(chunkNextPosX, worldGridPosition.y), sf::Vector2i(x, y));
            if (objectOptional.has_value())
                return false;
        }
    }

    // Test tiles to down (direction) chunk if required
    for (int y = 0; y < y_remaining; y++)
    {
        for (int x = position.x; x < std::min(position.x + objectData.size.x, (int)objectGrid[0].size()); x++)
        {
            // Test tile
            if (!objectData.waterPlaceable)
            {
                if (chunkManager.getLoadedChunkTileType(ChunkPosition(worldGridPosition.x, chunkNextPosY), sf::Vector2i(x, y)) == TileType::Water)
                    return false;
            }

            // Test object
            std::optional<BuildableObject>& objectOptional = chunkManager.getChunkObject(ChunkPosition(worldGridPosition.x, chunkNextPosY), sf::Vector2i(x, y));
            if (objectOptional.has_value())
                return false;
        }
    }

    // Test tiles to down-right (direction) chunk if required
    for (int y = 0; y < y_remaining; y++)
    {
        for (int x = 0; x < x_remaining; x++)
        {
            // Test tile
            if (!objectData.waterPlaceable)
            {
                if (chunkManager.getLoadedChunkTileType(ChunkPosition(chunkNextPosX, chunkNextPosY), sf::Vector2i(x, y)) == TileType::Water)
                    return false;
            }

            // Test object
            std::optional<BuildableObject>& objectOptional = chunkManager.getChunkObject(ChunkPosition(chunkNextPosX, chunkNextPosY), sf::Vector2i(x, y));
            if (objectOptional.has_value())
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

        entity->update(dt, chunkManager);

        // Check if requires deleting (not alive)
        if (!entity->isAlive())
        {
            entityIter = entities.erase(entityIter);
            continue;
        }

        // Check if requires moving to different chunk
        sf::Vector2f relativePosition = entity->getPosition() - worldPosition;

        bool requiresMove = false;
        ChunkPosition newChunk(worldGridPosition.x, worldGridPosition.y);

        if (relativePosition.x < 0)
        {
            newChunk.x = (((worldGridPosition.x - 1) % worldSize) + worldSize) % worldSize;
            requiresMove = true;
        }
        else if (relativePosition.x > TILE_SIZE_PIXELS_UNSCALED * 8)
        {
            newChunk.x = (((worldGridPosition.x + 1) % worldSize) + worldSize) % worldSize;
            requiresMove = true;
        }
        else if (relativePosition.y < 0)
        {
            newChunk.y = (((worldGridPosition.y - 1) % worldSize) + worldSize) % worldSize;
            requiresMove = true;
        }
        else if (relativePosition.y > TILE_SIZE_PIXELS_UNSCALED * 8)
        {
            newChunk.y = (((worldGridPosition.y + 1) % worldSize) + worldSize) % worldSize;
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

    // sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * tileSize;

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
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            bool bridgeObjectOnWater = false;

            std::optional<BuildableObject>& objectOptional = objectGrid[y][x];
            if (objectOptional.has_value())
            {
                int objectType = objectOptional.value().getObjectType();
                const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
                bridgeObjectOnWater = !objectData.hasCollision;
            }

            if (bridgeObjectOnWater)
                continue;

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
                // Get referenced object
                const ObjectReference& objectReference = objectGrid[y][x]->getObjectReference().value();
                std::optional<BuildableObject>& objectOptional = chunkManager.getChunkObject(objectReference.chunk, objectReference.tile);

                // If valid object, add collision from object if required
                if (objectOptional.has_value())
                {
                    BuildableObject& object = objectOptional.value();

                    unsigned int objectType = object.getObjectType();
                    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

                    if (objectData.hasCollision)
                        createCollisionRect(collisionRects, x, y);
                }
            }

            const ObjectData& objectData = ObjectDataLoader::getObjectData(objectGrid[y][x]->getObjectType());
            
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

void Chunk::setWorldPosition(sf::Vector2f position)
{
    // Update all entity positions
    for (auto& entity : entities)
    {
        // Get position relative to chunk before updating chunk position
        sf::Vector2f relativePosition = entity->getPosition() - worldPosition;
        // Set entity position to new chunk position + relative
        entity->setPosition(position + relativePosition);
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