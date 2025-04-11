#include "World/TileMap.hpp"

TileMap::TileMap()
{
    TileMap(pl::Vector2<int>(0, 0), 1);
}

TileMap::TileMap(pl::Vector2<int> offset, int variation)
{
    for (int i = 0; i < tiles.size(); i++)
    {
        tiles[i].fill(0);
    }

    tilesetOffset = offset;
    this->variation = variation;

    buildVertexArray();
}

void TileMap::setTilesetOffset(pl::Vector2<int> offset)
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

void TileMap::draw(pl::RenderTarget& window, pl::Vector2f position, pl::Vector2f scale)
{
    if (tileVertexArray.size() <= 0)
    {
        return;
    }

    pl::Shader* shader = Shaders::getShader(ShaderType::TileMap);

    shader->setUniform2f("position", (position.x - window.getWidth() / 2) / window.getWidth(), -(position.y - window.getHeight() / 2) / window.getHeight());

    // TODO: Probably fix this (opengl coordinate system etc)
    shader->setUniform2f("scale", scale.x, scale.y);

    window.draw(tileVertexArray, *shader, TextureManager::getTexture(TextureType::GroundTiles), pl::BlendMode::Alpha);

    // sf::RenderStates renderState;
    // renderState.transform.translate(position);
    // renderState.transform.scale(scale);
    // renderState.texture = TextureManager::getTexture(TextureType::GroundTiles);

    // window.draw(&(tileVertexArray[0]), tileVertexArray.getVertexCount(), sf::Quads, renderState);
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
            
    pl::Vector2<int> textureOffset = getTextureOffsetForTile(x, y);

    int vertexIndex = (y * tiles[0].size() + x) * 6;

    //if (tileVertexArray.getVertexCount() < vertexIndex + 4)
        //return;

    tileVertexArray[vertexIndex].textureUV = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 0, static_cast<float>(tilesetOffset.y) + textureOffset.y + 0};
    tileVertexArray[vertexIndex + 1].textureUV = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 16, static_cast<float>(tilesetOffset.y) + textureOffset.y + 0};
    tileVertexArray[vertexIndex + 2].textureUV = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 0, static_cast<float>(tilesetOffset.y) + textureOffset.y + 16};
    tileVertexArray[vertexIndex + 3].textureUV = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 16, static_cast<float>(tilesetOffset.y) + textureOffset.y + 0};
    tileVertexArray[vertexIndex + 4].textureUV = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 16, static_cast<float>(tilesetOffset.y) + textureOffset.y + 16};
    tileVertexArray[vertexIndex + 5].textureUV = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 0, static_cast<float>(tilesetOffset.y) + textureOffset.y + 16};
    
    for (int i = 0; i < 6; i++)
    {
        tileVertexArray[vertexIndex + i].color = pl::Color(255, 255, 255, 255);
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

pl::Vector2<int> TileMap::getTextureOffsetForTile(int x, int y)
{
    uint8_t tileVariation = (tiles[y][x] >> 4) & 0b111;
    pl::Vector2<int> variationOffset(64 * tileVariation, 0);

    switch(tiles[y][x] & 0b1111)
    {
        case 0: return pl::Vector2<int>(48, 48) + variationOffset;
        case 1: return pl::Vector2<int>(48, 0) + variationOffset;
        case 2: return pl::Vector2<int>(0, 48) + variationOffset;
        case 3: return pl::Vector2<int>(0, 0) + variationOffset;
        case 4: return pl::Vector2<int>(32, 48) + variationOffset;
        case 5: return pl::Vector2<int>(32, 0) + variationOffset;
        case 6: return pl::Vector2<int>(16, 48) + variationOffset;
        case 7: return pl::Vector2<int>(16, 0) + variationOffset;
        case 8: return pl::Vector2<int>(48, 32) + variationOffset;
        case 9: return pl::Vector2<int>(48, 16) + variationOffset;
        case 10: return pl::Vector2<int>(0, 32) + variationOffset;
        case 11: return pl::Vector2<int>(0, 16) + variationOffset;
        case 12: return pl::Vector2<int>(32, 32) + variationOffset;
        case 13: return pl::Vector2<int>(32, 16) + variationOffset;
        case 14: return pl::Vector2<int>(16, 32) + variationOffset;
        case 15: return pl::Vector2<int>(16, 16) + variationOffset;
    }

    return pl::Vector2<int>(0, 0);
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
            if (!isTilePresent(tiles[y][x]))
            {
                tileVertexArray.addQuad(pl::Rect<float>(pl::Vector2f(x, y) * TILE_SIZE_PIXELS_UNSCALED, pl::Vector2f(1, 1) * TILE_SIZE_PIXELS_UNSCALED),
                    pl::Color(0, 0, 0, 0), pl::Rect<float>());
                continue;
            }

            pl::Vector2<int> textureOffset = getTextureOffsetForTile(x, y);
            
            tileVertexArray.addQuad(pl::Rect<float>(pl::Vector2f(x, y) * TILE_SIZE_PIXELS_UNSCALED, pl::Vector2f(1, 1) * TILE_SIZE_PIXELS_UNSCALED),
                pl::Color(255, 255, 255), pl::Rect<float>(pl::Vector2f(textureOffset.x, textureOffset.y), pl::Vector2f(16, 16)));
        }
    }
}

pl::Vector2<int> TileMap::getTextureOffset()
{
    return tilesetOffset;
}