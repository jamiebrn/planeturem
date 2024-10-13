#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <cmath>

#include "Core/Helper.hpp"
#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"

class SpriteBatch
{
public:
    SpriteBatch();

    void beginDrawing();

    void draw(sf::RenderTarget& window, const TextureDrawData& drawData, const sf::IntRect& textureRect, std::optional<ShaderType> shaderType = std::nullopt);

    void endDrawing(sf::RenderTarget& window, std::optional<ShaderType> shaderType = std::nullopt);

private:
    void drawVertexArray(sf::RenderTarget& window, std::optional<ShaderType> shaderType = std::nullopt);

    void resetBatchValues();

private:
    sf::VertexArray vertexArray;

    std::optional<TextureType> batchTextureType = std::nullopt;

};