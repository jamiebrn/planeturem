#include "World/TileMap.hpp"

TileMap::TileMap()
{
    TileMap(sf::Vector2i(0, 0), 1);
}

TileMap::TileMap(sf::Vector2i offset, int variation)
{
    for (int i = 0; i < tiles.size(); i++)
    {
        tiles[i].fill(0);
    }

    tilesetOffset = offset;
    this->variation = variation;

    buildVertexArray();
}

void TileMap::setTilesetOffset(sf::Vector2i offset)
{
    tilesetOffset = offset;
}

void TileMap::setTilesetVariation(int variation)
{
    this->variation = variation;
}

void TileMap::setTile(int x, int y, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles)
{
    if (isTilePresent(x, y))
        return;

    tiles[y][x] = 0b1 << 7;

    // Randomise variation
    uint8_t tileVariation = (rand() % variation) & 0b111;
    tiles[y][x] |= tileVariation << 4;

    updateTiles(x, y, upTiles, downTiles, leftTiles, rightTiles);
}

void TileMap::removeTile(int x, int y, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles)
{
    tiles[y][x] = 0;
    updateTiles(x, y, upTiles, downTiles, leftTiles, rightTiles);
}

void TileMap::setTileWithoutGraphicsUpdate(int x, int y, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles)
{
    tiles[y][x] = 0b1 << 7;

    // Randomise variation
    uint8_t tileVariation = (rand() % variation) & 0b111;
    tiles[y][x] |= tileVariation << 4;

    updateTiles(x, y, upTiles, downTiles, leftTiles, rightTiles, false);
}

void TileMap::draw(sf::RenderTarget& window, sf::Vector2f position, sf::Vector2f scale)
{
    if (tileVertexArray.getVertexCount() <= 0)
        return;

    sf::RenderStates renderState;
    renderState.transform.translate(position);
    renderState.transform.scale(scale);
    renderState.texture = TextureManager::getTexture(TextureType::GroundTiles);

    window.draw(&(tileVertexArray[0]), tileVertexArray.getVertexCount(), sf::Quads, renderState);
}

void TileMap::updateTiles(int xModified, int yModified, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles, bool rebuildVertices)
{
    for (int y = yModified - 1; y <= yModified + 1; y++)
    {
        if (y < 0 || y >= CHUNK_TILE_SIZE)
            continue;
        
        for (int x = xModified - 1; x <= xModified + 1; x++)
        {
            if (x < 0 || x >= CHUNK_TILE_SIZE)
                continue;
            
            // Skip corners
            if ((y == yModified - 1 || y == yModified + 1) && (x == xModified - 1 || x == xModified + 1))
                continue;
            
            updateTileFromAdjacent(x, y, upTiles, downTiles, leftTiles, rightTiles);

            if (rebuildVertices)
            {
                refreshVerticiesForTile(x, y);
            }
        }
    }

    /*if (!rebuildVertices)
        return;

    buildVertexArray();*/
}

void TileMap::refreshTile(int x, int y, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles)
{
    updateTileFromAdjacent(x, y, upTiles, downTiles, leftTiles, rightTiles);
    refreshVerticiesForTile(x, y);
}

void TileMap::refreshTopEdge(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles)
{
    for (int x = 0; x < tiles[0].size(); x++)
    {
        refreshTile(x, 0, upTiles, downTiles, leftTiles, rightTiles);
    }
}

void TileMap::refreshBottomEdge(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles)
{
    for (int x = 0; x < tiles[0].size(); x++)
    {
        refreshTile(x, tiles.size() - 1, upTiles, downTiles, leftTiles, rightTiles);
    }
}

void TileMap::refreshLeftEdge(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles)
{
    for (int y = 0; y < tiles.size(); y++)
    {
        refreshTile(0, y, upTiles, downTiles, leftTiles, rightTiles);
    }
}

void TileMap::refreshRightEdge(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles)
{
    for (int y = 0; y < tiles.size(); y++)
    {
        refreshTile(tiles[0].size() - 1, y, upTiles, downTiles, leftTiles, rightTiles);
    }
}

void TileMap::refreshVerticiesForTile(int x, int y)
{
    if (!isTilePresent(tiles[y][x]))
        return;
            
    sf::Vector2i textureOffset = getTextureOffsetForTile(x, y);

    int vertexIndex = (y * tiles[0].size() + x) * 4;

    //if (tileVertexArray.getVertexCount() < vertexIndex + 4)
        //return;

    tileVertexArray[vertexIndex].texCoords = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 0, static_cast<float>(tilesetOffset.y) + textureOffset.y + 0};
    tileVertexArray[vertexIndex + 1].texCoords = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 16, static_cast<float>(tilesetOffset.y) + textureOffset.y + 0};
    tileVertexArray[vertexIndex + 3].texCoords = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 0, static_cast<float>(tilesetOffset.y) + textureOffset.y + 16};
    tileVertexArray[vertexIndex + 2].texCoords = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 16, static_cast<float>(tilesetOffset.y) + textureOffset.y + 16};

    for (int i = 0; i < 4; i++)
    {
        tileVertexArray[vertexIndex + i].color = sf::Color(255, 255, 255, 255);
    }
}

void TileMap::updateAllTiles(TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles)
{
    for (int y = 0; y < tiles.size(); y++)
    {
        for (int x = 0; x < tiles[0].size(); x++)
        {
            updateTileFromAdjacent(x, y, upTiles, downTiles, leftTiles, rightTiles);
        }
    }

    buildVertexArray();
}

void TileMap::updateTileFromAdjacent(int x, int y, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles)
{
    uint8_t& tile = tiles[y][x];
    if (!isTilePresent(tile))
    {
        // Tile is not filled, so do not update
        return;
    }

    // Reset tile adjacent values
    tile &= 0b11110000;

    if (x > 0)
    {
        tile |= (static_cast<int>(isTilePresent(tiles[y][x - 1])) << 2);
    }
    else if (leftTiles != nullptr)
    {
        tile |= (static_cast<int>(leftTiles->isTilePresent(CHUNK_TILE_SIZE - 1, y)) << 2);
    }

    if (x < tiles[0].size() - 1)
    {
        tile |= (static_cast<int>(isTilePresent(tiles[y][x + 1])) << 1);
    }
    else if (rightTiles != nullptr)
    {
        tile |= (static_cast<int>(rightTiles->isTilePresent(0, y)) << 1);
    }

    if (y > 0)
    {
        tile |= (static_cast<int>(isTilePresent(tiles[y - 1][x])) << 3);
    }
    else if (upTiles != nullptr)
    {
        tile |= (static_cast<int>(upTiles->isTilePresent(x, CHUNK_TILE_SIZE - 1)) << 3);
    }

    if (y < tiles.size() - 1)
    {
        tile |= (static_cast<int>(isTilePresent(tiles[y + 1][x])));
    }
    else if (downTiles != nullptr)
    {
        tile |= (static_cast<int>(downTiles->isTilePresent(x, 0)));
    }
}

sf::Vector2i TileMap::getTextureOffsetForTile(int x, int y)
{
    uint8_t tileVariation = (tiles[y][x] >> 4) & 0b111;
    sf::Vector2i variationOffset(64 * tileVariation, 0);

    switch(tiles[y][x] & 0b1111)
    {
        case 0: return sf::Vector2i(48, 48) + variationOffset;
        case 1: return sf::Vector2i(48, 0) + variationOffset;
        case 2: return sf::Vector2i(0, 48) + variationOffset;
        case 3: return sf::Vector2i(0, 0) + variationOffset;
        case 4: return sf::Vector2i(32, 48) + variationOffset;
        case 5: return sf::Vector2i(32, 0) + variationOffset;
        case 6: return sf::Vector2i(16, 48) + variationOffset;
        case 7: return sf::Vector2i(16, 0) + variationOffset;
        case 8: return sf::Vector2i(48, 32) + variationOffset;
        case 9: return sf::Vector2i(48, 16) + variationOffset;
        case 10: return sf::Vector2i(0, 32) + variationOffset;
        case 11: return sf::Vector2i(0, 16) + variationOffset;
        case 12: return sf::Vector2i(32, 32) + variationOffset;
        case 13: return sf::Vector2i(32, 16) + variationOffset;
        case 14: return sf::Vector2i(16, 32) + variationOffset;
        case 15: return sf::Vector2i(16, 16) + variationOffset;
    }

    return sf::Vector2i(0, 0);
}

bool TileMap::isTilePresent(int x, int y)
{
    return isTilePresent(tiles[y][x]);
}

bool TileMap::isTilePresent(uint8_t tileValue)
{
    return ((tileValue >> 7) & 0b1) != 0;
}

void TileMap::buildVertexArray()
{
    tileVertexArray.clear();

    for (int y = 0; y < tiles.size(); y++)
    {
        for (int x = 0; x < tiles[0].size(); x++)
        {
            // Add tile to vertex array
            sf::Vertex vertices[4];
            vertices[0].position = sf::Vector2f(x * TILE_SIZE_PIXELS_UNSCALED, y * TILE_SIZE_PIXELS_UNSCALED);
            vertices[1].position = sf::Vector2f(x * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED, y * TILE_SIZE_PIXELS_UNSCALED);
            vertices[3].position = sf::Vector2f(x * TILE_SIZE_PIXELS_UNSCALED, y * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED);
            vertices[2].position = sf::Vector2f(x * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED, y * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED);

            if (isTilePresent(tiles[y][x]))
            {
                // Set vertex texture coordinates
                sf::Vector2i textureOffset = getTextureOffsetForTile(x, y);
                
                vertices[0].texCoords = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 0, static_cast<float>(tilesetOffset.y) + textureOffset.y + 0};
                vertices[1].texCoords = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 16, static_cast<float>(tilesetOffset.y) + textureOffset.y + 0};
                vertices[3].texCoords = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 0, static_cast<float>(tilesetOffset.y) + textureOffset.y + 16};
                vertices[2].texCoords = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 16, static_cast<float>(tilesetOffset.y) + textureOffset.y + 16};
            }
            else
            {
                for (int i = 0; i < 4; i++)
                {
                    vertices[i].color = sf::Color(0, 0, 0, 0);
                }
            }

            for (int i = 0; i < 4; i++)
            {
                tileVertexArray.append(vertices[i]);
            }
        }
    }
}