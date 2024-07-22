#pragma once

#include <SFML/Graphics.hpp>
#include <FastNoiseLite.h>
#include <array>

#include "TileType.hpp"
#include "TextureManager.hpp"

class Chunk
{

public:
    Chunk(sf::Vector2i worldPosition);

    void generateChunk(const FastNoiseLite& noise);

    void drawChunk(sf::RenderWindow& window, sf::Vector2f cameraPos);

private:
    std::array<std::array<TileType, 8>, 8> groundTileGrid;
    sf::VertexArray groundVertexArray;

    std::array<std::array<int, 8>, 8> objectGrid;

    sf::Vector2i worldGridPosition;

};