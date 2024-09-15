#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <vector>

#include "Core/TextureManager.hpp"

#include "GameConstants.hpp"

// struct TileMapAdjacents
// {
//     std::vector<TileMap>* upTiles = nullptr;
//     std::vector<TileMap>* downTiles = nullptr;
//     std::vector<TileMap>* leftTiles = nullptr;
//     std::vector<TileMap>* rightTiles = nullptr;
// };

class TileMap
{
public:
    TileMap();
    TileMap(sf::Vector2i offset, int variation = 1);

    void setTilesetOffset(sf::Vector2i offset);
    void setTilesetVariation(int variation);

    void setTile(int x, int y, TileMap* upTiles = nullptr, TileMap* downTiles = nullptr, TileMap* leftTiles = nullptr, TileMap* rightTiles = nullptr);
    void removeTile(int x, int y, TileMap* upTiles = nullptr, TileMap* downTiles = nullptr, TileMap* leftTiles = nullptr, TileMap* rightTiles = nullptr);

    // Ensure buildVertexArray is called at the end of tile modification
    void setTileWithoutGraphicsUpdate(int x, int y, TileMap* upTiles = nullptr, TileMap* downTiles = nullptr, TileMap* leftTiles = nullptr, TileMap* rightTiles = nullptr);

    void draw(sf::RenderTarget& window, sf::Vector2f position, sf::Vector2f scale);

    void refreshTile(int x, int y, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);
    void refreshTopEdge(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);
    void refreshBottomEdge(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);
    void refreshLeftEdge(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);
    void refreshRightEdge(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);

    void updateAllTiles(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);

    void buildVertexArray();

private:
    void updateTiles(int xModified, int yModified, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles, bool rebuildVertices = true);
    void updateTileFromAdjacent(int x, int y, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);

    void refreshVerticiesForTile(int x, int y);

    sf::Vector2i getTextureOffsetForTile(int x, int y);

    bool isTilePresent(int x, int y);
    bool isTilePresent(uint8_t tileValue);

private:
    sf::VertexArray tileVertexArray;

    sf::Vector2i tilesetOffset;

    int variation;

    // y, x
    // MSB stores whether tile is present, 4 LSB stores tile type/neighbours
    // Bits between store random variation in chosen tileset (up to 8 variations)
    std::array<std::array<uint8_t, static_cast<int>(CHUNK_TILE_SIZE)>, static_cast<int>(CHUNK_TILE_SIZE)> tiles;

};