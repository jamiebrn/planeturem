#include "Chunk.hpp"

Chunk::Chunk(sf::Vector2i worldGridPosition)
{
    this->worldGridPosition = worldGridPosition;
    groundVertexArray = sf::VertexArray(sf::Quads, 8 * 8 * 4);
}

void Chunk::generateChunk(const FastNoiseLite& noise)
{
    sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f;

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            float height = noise.GetNoise(worldPosition.x + (float)x, worldPosition.y + (float)y);

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

            objectGrid[y][x] = 0;
            if (rand() % 10 == 0 && height >= 0.2)
                objectGrid[y][x] = 1;
        }
    }
}

void Chunk::drawChunk(sf::RenderWindow& window, sf::Vector2f cameraPos)
{
    sf::Vector2f worldPosition = static_cast<sf::Vector2f>(worldGridPosition) * 8.0f * 48.0f;

    sf::Transform transform;
    transform.translate(-cameraPos + worldPosition);

    sf::RenderStates state;
    state.texture = TextureManager::getTexture(TextureType::GroundTiles);
    state.transform = transform;
    window.draw(groundVertexArray, state);

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            if (objectGrid[y][x] == 1)
            {
                TextureManager::drawTexture(window, {TextureType::Tree, 
                    sf::Vector2f(x * 48.0f + 24.0f + worldPosition.x, y * 48.0f + 24.0f + worldPosition.y) - cameraPos
                    , 0, 3, {0.5, 0.9}
                });
            }
        }
    }
}