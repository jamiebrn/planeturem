#include "Core/SpriteBatch.hpp"

SpriteBatch::SpriteBatch()
{
    vertexArray.setPrimitiveType(sf::Quads);
}

void SpriteBatch::beginDrawing()
{
    resetBatchValues();
}

void SpriteBatch::draw(sf::RenderTarget& window, const TextureDrawData& drawData, const sf::IntRect& textureRect, std::optional<ShaderType> shaderType)
{
    if (shaderType.has_value())
    {
        // If shader used, end batch
        endDrawing(window);
        batchTextureType = drawData.type;
    }
    else
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
    }

    sf::Vertex vertices[4];
    
    sf::Vector2f size;
    size.x = textureRect.width * drawData.scale.x;
    size.y = textureRect.height * drawData.scale.y;

    float centreRatioX = drawData.centerRatio.x;
    float centreRatioY = drawData.centerRatio.y;

    if (drawData.useCentreAbsolute)
    {
        centreRatioX /= textureRect.width;
        centreRatioY /= textureRect.height;
    }
    
    if (drawData.rotation == 0)
    {
        // Simple case, no rotation
        // Separate from rotation calculation in order to save performance
        sf::Vector2f topLeft;
        topLeft.x = drawData.position.x - (size.x * centreRatioX);
        topLeft.y = drawData.position.y - (size.y * centreRatioY);

        vertices[0].position = topLeft;
        vertices[1].position = topLeft + sf::Vector2f(size.x, 0);
        vertices[2].position = topLeft + sf::Vector2f(size.x, size.y);
        vertices[3].position = topLeft + sf::Vector2f(0, size.y);
    }
    else
    {
        // Apply rotation
        float angleRadians = M_PI * drawData.rotation / 180.0f;

        float nX = -size.x * centreRatioX;
        float pX = (1.0f - centreRatioX) * size.x;
        float nY = -size.y * centreRatioY;
        float pY = (1.0f - centreRatioY) * size.y;

        vertices[0].position = Helper::rotateVector(sf::Vector2f(nX, nY), angleRadians) + drawData.position;
        vertices[1].position = Helper::rotateVector(sf::Vector2f(pX, nY), angleRadians) + drawData.position;
        vertices[2].position = Helper::rotateVector(sf::Vector2f(pX, pY), angleRadians) + drawData.position;
        vertices[3].position = Helper::rotateVector(sf::Vector2f(nX, pY), angleRadians) + drawData.position;
    }

    // Set UV coords
    vertices[0].texCoords = static_cast<sf::Vector2f>(textureRect.getPosition());
    vertices[1].texCoords = static_cast<sf::Vector2f>(textureRect.getPosition()) + sf::Vector2f(textureRect.width, 0);
    vertices[2].texCoords = static_cast<sf::Vector2f>(textureRect.getPosition() + textureRect.getSize());
    vertices[3].texCoords = static_cast<sf::Vector2f>(textureRect.getPosition()) + sf::Vector2f(0, textureRect.height);

    for (int i = 0; i < 4; i++)
    {
        vertices[i].color = drawData.colour;
        vertexArray.append(vertices[i]);
    }

    // If shader is used, draw using shader (do not batch)
    if (shaderType.has_value())
    {
        endDrawing(window, shaderType);
    }
}

void SpriteBatch::endDrawing(sf::RenderTarget& window, std::optional<ShaderType> shaderType)
{
    drawVertexArray(window, shaderType);
    resetBatchValues();
}

void SpriteBatch::drawVertexArray(sf::RenderTarget& window, std::optional<ShaderType> shaderType)
{
    if (!batchTextureType.has_value())
        return;
    
    sf::RenderStates renderState;
    renderState.texture = TextureManager::getTexture(batchTextureType.value());

    if (shaderType.has_value())
    {
        sf::Shader* shader = Shaders::getShader(shaderType.value());
        renderState.shader = shader;
    }

    window.draw(vertexArray, renderState);
}

void SpriteBatch::resetBatchValues()
{
    vertexArray.clear();
    batchTextureType = std::nullopt;
}