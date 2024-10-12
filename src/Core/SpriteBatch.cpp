#include "Core/SpriteBatch.hpp"

SpriteBatch::SpriteBatch()
{
    vertexArray.setPrimitiveType(sf::Quads);
}

void SpriteBatch::beginDrawing()
{
    resetBatchValues();
}

void SpriteBatch::draw(sf::RenderTarget& window, TextureDrawData drawData, sf::IntRect textureRect)
{
    if (batchTextureType.has_value())
    {
        if (batchTextureType.value() != drawData.type)
        {
            // End current batch and start new
            endDrawing(window);
            batchTextureType = drawData.type;
        }
    }
    else
    {
        batchTextureType = drawData.type;
    }

    sf::Vertex vertices[4];
    
    sf::Vector2f size;
    size.x = textureRect.width * drawData.scale.x;
    size.y = textureRect.height * drawData.scale.y;
    
    if (drawData.rotation == 0)
    {
        // Simple case, no rotation
        // Separate from rotation calculation in order to save performance
        sf::Vector2f topLeft;
        topLeft.x = drawData.position.x - (size.x * drawData.centerRatio.x);
        topLeft.y = drawData.position.y - (size.y * drawData.centerRatio.y);

        vertices[0].position = topLeft;
        vertices[1].position = topLeft + sf::Vector2f(size.x, 0);
        vertices[2].position = topLeft + sf::Vector2f(size.x, size.y);
        vertices[3].position = topLeft + sf::Vector2f(0, size.y);
    }
    else
    {
        // Apply rotation
        float angleRadians = M_PI * drawData.rotation / 180.0f;

        float nX = -size.x * drawData.centerRatio.x;
        float pX = (1.0f - drawData.centerRatio.x) * size.x;
        float nY = -size.y * drawData.centerRatio.y;
        float pY = (1.0f - drawData.centerRatio.y) * size.y;

        vertices[0].position = Helper::rotateVector(sf::Vector2f(nX, nY), angleRadians) + drawData.position;
        vertices[1].position = Helper::rotateVector(sf::Vector2f(pX, nY), angleRadians) + drawData.position;
        vertices[2].position = Helper::rotateVector(sf::Vector2f(pX, pY), angleRadians) + drawData.position;
        vertices[3].position = Helper::rotateVector(sf::Vector2f(nX, pY), angleRadians) + drawData.position;
    }

    // Draw to vertex array
    vertices[0].texCoords = static_cast<sf::Vector2f>(textureRect.getPosition());
    vertices[0].color = drawData.colour;
    vertices[1].texCoords = static_cast<sf::Vector2f>(textureRect.getPosition()) + sf::Vector2f(textureRect.width, 0);
    vertices[1].color = drawData.colour;
    vertices[2].texCoords = static_cast<sf::Vector2f>(textureRect.getPosition() + textureRect.getSize());
    vertices[2].color = drawData.colour;
    vertices[3].texCoords = static_cast<sf::Vector2f>(textureRect.getPosition()) + sf::Vector2f(0, textureRect.height);
    vertices[3].color = drawData.colour;

    for (int i = 0; i < 4; i++)
    {
        vertexArray.append(vertices[i]);
    }
}

void SpriteBatch::endDrawing(sf::RenderTarget& window)
{
    drawVertexArray(window);
    resetBatchValues();
}

void SpriteBatch::drawVertexArray(sf::RenderTarget& window)
{
    if (!batchTextureType.has_value())
        return;

    sf::Texture* texture = TextureManager::getTexture(batchTextureType.value());
    window.draw(vertexArray, texture);
}

void SpriteBatch::resetBatchValues()
{
    vertexArray.clear();
    batchTextureType = std::nullopt;
}