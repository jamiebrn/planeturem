#pragma once

#include <SFML/Graphics.hpp>
#include <array>

#include "Core/TextureManager.hpp"

#include "GameConstants.hpp"

class TileMap
{
public:
    TileMap();

    void setTilesetOffset(sf::Vector2i offset);
    void setTilesetVariation(int variation);

    void setTile(int x, int y, TileMap* upTiles = nullptr, TileMap* downTiles = nullptr, TileMap* leftTiles = nullptr, TileMap* rightTiles = nullptr);
    void removeTile(int x, int y, TileMap* upTiles = nullptr, TileMap* downTiles = nullptr, TileMap* leftTiles = nullptr, TileMap* rightTiles = nullptr);

    void draw(sf::RenderTarget& window, sf::Vector2f position, sf::Vector2f scale);

private:
    void updateTiles(int xModified, int yModified);
    void updateTileFromAdjacent(int x, int y);

    sf::Vector2i getTextureOffsetForTile(int x, int y);

    bool isTilePresent(uint8_t tileValue);
    
    void buildVertexArray();

private:
    sf::VertexArray tileVertexArray;

    sf::Vector2i tilesetOffset;

    int variation = 1;

    // y, x
    // MSB stores whether tile is present, 4 LSB stores tile type/neighbours
    // Bits between store random variation in chosen tileset (up to 8 variations)
    std::array<std::array<uint8_t, 40>, 22> tiles;

};