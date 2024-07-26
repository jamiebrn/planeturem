#include "World/Chunk.hpp"

Chunk::Chunk(sf::Vector2i worldGridPosition)
{
    this->worldGridPosition = worldGridPosition;
    groundVertexArray = sf::VertexArray(sf::Quads, 8 * 8 * 4);
}

void Chunk::generateChunk(const FastNoiseLite& noise)
{
    sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;
    sf::Vector2f worldNoisePosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f;

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            float height = noise.GetNoise(worldNoisePosition.x + (float)x, worldNoisePosition.y + (float)y);

            int vertexArrayIndex = (x + y * 8) * 4;
            groundVertexArray[vertexArrayIndex].position = sf::Vector2f(x * 48, y * 48);
            groundVertexArray[vertexArrayIndex + 1].position = sf::Vector2f(x * 48 + 48, y * 48);
            groundVertexArray[vertexArrayIndex + 3].position = sf::Vector2f(x * 48, y * 48 + 48);
            groundVertexArray[vertexArrayIndex + 2].position = sf::Vector2f(x * 48 + 48, y * 48 + 48);

            if (height < 0)
            {
                groundVertexArray[vertexArrayIndex].texCoords = {2 * 16, 0};
                groundVertexArray[vertexArrayIndex + 1].texCoords = {2 * 16 + 16, 0};
                groundVertexArray[vertexArrayIndex + 3].texCoords = {2 * 16, 16};
                groundVertexArray[vertexArrayIndex + 2].texCoords = {2 * 16 + 16, 16};
                groundTileGrid[y][x] = TileType::Water;
            }
            else if (height > 0.7)
            {
                groundVertexArray[vertexArrayIndex].texCoords = {1 * 16, 0};
                groundVertexArray[vertexArrayIndex + 1].texCoords = {1 * 16 + 16, 0};
                groundVertexArray[vertexArrayIndex + 3].texCoords = {1 * 16, 16};
                groundVertexArray[vertexArrayIndex + 2].texCoords = {1 * 16 + 16, 16};
                groundTileGrid[y][x] = TileType::DarkGrass;
            }
            else if (height >=0 && height < 0.2)
            {
                groundVertexArray[vertexArrayIndex].texCoords = {3 * 16, 0};
                groundVertexArray[vertexArrayIndex + 1].texCoords = {3 * 16 + 16, 0};
                groundVertexArray[vertexArrayIndex + 3].texCoords = {3 * 16, 16};
                groundVertexArray[vertexArrayIndex + 2].texCoords = {3 * 16 + 16, 16};
                groundTileGrid[y][x] = TileType::Sand;
            }
            else
            {
                groundVertexArray[vertexArrayIndex].texCoords = {0 * 16, 0};
                groundVertexArray[vertexArrayIndex + 1].texCoords = {0 * 16 + 16, 0};
                groundVertexArray[vertexArrayIndex + 3].texCoords = {0 * 16, 16};
                groundVertexArray[vertexArrayIndex + 2].texCoords = {0 * 16 + 16, 16};
                groundTileGrid[y][x] = TileType::Grass;
            }

            sf::Vector2f objectPos = worldPosition + sf::Vector2f(x * 48.0f + 24.0f, y * 48.0f + 24.0f);
            objectGrid[y][x] = NULL;

            int spawn_chance = rand() % 40;

            if (spawn_chance < 4 && height >= 0.2)
            {
                // Make tree
                objectGrid[y][x] = std::move(std::make_unique<BuildableObject>(objectPos, 0));
            }
            else if (spawn_chance == 4 && height >= 0.2)
            {
                // Make bush
                objectGrid[y][x] = std::move(std::make_unique<BuildableObject>(objectPos, 1));
            }
        }
    }
}

void Chunk::drawChunkTerrain(sf::RenderWindow& window)
{
    sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;

    sf::Transform transform;
    transform.translate(Camera::getIntegerDrawOffset() + worldPosition);

    sf::RenderStates state;
    state.texture = TextureManager::getTexture(TextureType::GroundTiles);
    state.transform = transform;
    window.draw(groundVertexArray, state);
}

void Chunk::drawChunkObjects(sf::RenderWindow& window)
{
    // sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;

    // for (int y = 0; y < 8; y++)
    // {
    //     for (int x = 0; x < 8; x++)
    //     {
    //         if (objectGrid[y][x])
    //             objectGrid[y][x]->draw(window, dt);
    //     }
    // }
}

void Chunk::updateChunkObjects(float dt)
{
    for (auto& object_row : objectGrid)
    {
        for (auto& object : object_row)
        {
            if (object)
            {
                object->update(dt);
                if (!object->isAlive())
                    object = nullptr;
            }
        }
    }
}

std::vector<WorldObject*> Chunk::getObjects()
{
    std::vector<WorldObject*> objects;
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            if (objectGrid[y][x])
                objects.push_back(objectGrid[y][x].get());
        }
    }
    return objects;
}

void Chunk::setObject(sf::Vector2i position, unsigned int objectType)
{
    sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;
    sf::Vector2f objectPos = worldPosition + sf::Vector2f(position.x * 48.0f + 24.0f, position.y * 48.0f + 24.0f);

    std::unique_ptr<BuildableObject> object = std::make_unique<BuildableObject>(objectPos, objectType);

    objectGrid[position.y][position.x] = std::move(object);
}

bool Chunk::canPlaceObject(sf::Vector2i selected_tile)
{
    // Test terrain
    if (groundTileGrid[selected_tile.y][selected_tile.x] == TileType::Water)
        return false;
    
    // Test objects
    if (objectGrid[selected_tile.y][selected_tile.x] != nullptr)
        return false;
    
    return true;
}

std::vector<std::unique_ptr<CollisionRect>> Chunk::getCollisionRects()
{
    sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;

    auto createCollisionRect = [worldPosition](std::vector<std::unique_ptr<CollisionRect>>& rects, int x, int y) -> void
    {
        std::unique_ptr<CollisionRect> collisionRect = std::make_unique<CollisionRect>();

        collisionRect->x = worldPosition.x + x * 48.0f;
        collisionRect->y = worldPosition.y + y * 48.0f;
        collisionRect->width = 48.0f;
        collisionRect->height = 48.0f;

        rects.push_back(std::move(collisionRect));
    };

    std::vector<std::unique_ptr<CollisionRect>> collisionRects;

    // Get collisions for tiles
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            if (groundTileGrid[y][x] == TileType::Water)
                createCollisionRect(collisionRects, x, y);
        }
    }

    // Get collisions for objects
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            if (objectGrid[y][x] == nullptr)
                continue;
            
            const ObjectData& objectData = ObjectDataLoader::getObjectData(objectGrid[y][x]->getObjectType());
            
            if (objectData.hasCollision)
                createCollisionRect(collisionRects, x, y);
        }
    }

    return collisionRects;
}

bool Chunk::isPointInChunk(sf::Vector2f position)
{
    sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;
    sf::Vector2f chunkExtentPosition = worldPosition + sf::Vector2f(8.0f * 48.0f, 8.0f * 48.0f);

    return (position.x >= worldPosition.x && position.x <= chunkExtentPosition.x
        && position.y >= worldPosition.y && position.y <= chunkExtentPosition.y);
}