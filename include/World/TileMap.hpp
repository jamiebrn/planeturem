#pragma once

// #include <SFML/Graphics.hpp>
#include <array>
#include <vector>

#include <Graphics/RenderTarget.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/VertexArray.hpp>
#include <Graphics/Shader.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"

#include "GameConstants.hpp"

class TileMap
{
public:
    TileMap();
    TileMap(pl::Vector2<int> offset, int variation = 1);

    void setTilesetOffset(pl::Vector2<int> offset);
    void setTilesetVariation(int variation);

    void setTile(int x, int y, TileMap* upTiles = nullptr, TileMap* downTiles = nullptr, TileMap* leftTiles = nullptr, TileMap* rightTiles = nullptr);
    void removeTile(int x, int y, TileMap* upTiles = nullptr, TileMap* downTiles = nullptr, TileMap* leftTiles = nullptr, TileMap* rightTiles = nullptr);

    // Ensure buildVertexArray is called at the end of tile modification
    void setTileWithoutGraphicsUpdate(int x, int y, TileMap* upTiles = nullptr, TileMap* downTiles = nullptr, TileMap* leftTiles = nullptr, TileMap* rightTiles = nullptr);

    void draw(pl::RenderTarget& window, pl::Vector2f position, pl::Vector2f scale);

    void refreshTile(int x, int y, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);
    void refreshTopEdge(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);
    void refreshBottomEdge(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);
    void refreshLeftEdge(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);
    void refreshRightEdge(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);

    void updateAllTiles(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);

    void buildVertexArray();

    pl::Vector2<int> getTextureOffset();

private:
    void updateTiles(int xModified, int yModified, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles, bool rebuildVertices = true);
    void updateTileFromAdjacent(int x, int y, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles);

    void refreshVerticiesForTile(int x, int y);

    pl::Vector2<int> getTextureOffsetForTile(int x, int y);

    bool isTilePresent(int x, int y);
    bool isTilePresent(uint8_t tileValue);

private:
    pl::VertexArray tileVertexArray;

    pl::Vector2<int> tilesetOffset;

    int variation;

    // y, x
    // MSB stores whether tile is present, 4 LSB stores tile type/neighbours
    // Bits between store random variation in chosen tileset (up to 8 variations)
    std::array<std::array<uint8_t, static_cast<int>(CHUNK_TILE_SIZE)>, static_cast<int>(CHUNK_TILE_SIZE)> tiles;

};