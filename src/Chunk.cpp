#include "Chunk.hpp"

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
            }
            else if (height > 0.7)
            {
                groundVertexArray[vertexArrayIndex].texCoords = {1 * 16, 0};
                groundVertexArray[vertexArrayIndex + 1].texCoords = {1 * 16 + 16, 0};
                groundVertexArray[vertexArrayIndex + 3].texCoords = {1 * 16, 16};
                groundVertexArray[vertexArrayIndex + 2].texCoords = {1 * 16 + 16, 16};
            }
            else if (height >=0 && height < 0.2)
            {
                groundVertexArray[vertexArrayIndex].texCoords = {3 * 16, 0};
                groundVertexArray[vertexArrayIndex + 1].texCoords = {3 * 16 + 16, 0};
                groundVertexArray[vertexArrayIndex + 3].texCoords = {3 * 16, 16};
                groundVertexArray[vertexArrayIndex + 2].texCoords = {3 * 16 + 16, 16};
            }
            else
            {
                groundVertexArray[vertexArrayIndex].texCoords = {0 * 16, 0};
                groundVertexArray[vertexArrayIndex + 1].texCoords = {0 * 16 + 16, 0};
                groundVertexArray[vertexArrayIndex + 3].texCoords = {0 * 16, 16};
                groundVertexArray[vertexArrayIndex + 2].texCoords = {0 * 16 + 16, 16};
            }

            sf::Vector2f objectPos = worldPosition + sf::Vector2f(x * 48.0f + 24.0f, y * 48.0f + 24.0f);
            objectGrid[y][x] = NULL;

            int spawn_chance = rand() % 40;

            if (spawn_chance < 4 && height >= 0.2)
            {
                objectGrid[y][x] = std::move(std::make_unique<Tree>(objectPos));
            }
            else if (spawn_chance == 4 && height >= 0.2)
            {
                objectGrid[y][x] = std::move(std::make_unique<Bush>(objectPos));
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
    sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            if (objectGrid[y][x])
                objectGrid[y][x]->draw(window);
        }
    }
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

bool Chunk::isPointInChunk(sf::Vector2f position)
{
    sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;
    sf::Vector2f chunkExtentPosition = worldPosition + sf::Vector2f(8.0f * 48.0f, 8.0f * 48.0f);

    return (position.x >= worldPosition.x && position.x <= chunkExtentPosition.x
        && position.y >= worldPosition.y && position.y <= chunkExtentPosition.y);
}