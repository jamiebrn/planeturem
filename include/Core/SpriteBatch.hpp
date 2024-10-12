#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <cmath>

#include "Core/Helper.hpp"
#include "Core/TextureManager.hpp"

class SpriteBatch
{
public:
    SpriteBatch();

    void beginDrawing();

    void draw(sf::RenderTarget& window, TextureDrawData drawData, sf::IntRect textureRect);

    void endDrawing(sf::RenderTarget& window);

private:
    void drawVertexArray(sf::RenderTarget& window);

    void resetBatchValues();

private:
    sf::VertexArray vertexArray;

    std::optional<TextureType> batchTextureType = std::nullopt;

};