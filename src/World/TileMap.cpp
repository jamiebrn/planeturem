#include "World/TileMap.hpp"

TileMap::TileMap()
{
    for (int i = 0; i < tiles.size(); i++)
    {
        tiles[i].fill(0);
    }
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
    tiles[y][x] = 0b1 << 7;
    updateTiles(x, y);
}

void TileMap::removeTile(int x, int y, TileMap* upTiles, TileMap* downTiles, TileMap* leftTiles, TileMap* rightTiles)
{
    tiles[y][x] = 0;
    updateTiles(x, y);
}

void TileMap::draw(sf::RenderTarget& window, sf::Vector2f position, sf::Vector2f scale)
{
    sf::RenderStates renderState;
    renderState.transform.scale(scale);
    renderState.transform.translate(position);
    renderState.texture = TextureManager::getTexture(TextureType::GroundTiles);

    window.draw(&(tileVertexArray[0]), tileVertexArray.getVertexCount(), sf::Quads, renderState);
}

void TileMap::updateTiles(int xModified, int yModified)
{
    for (int y = std::max(yModified - 1, 0); y <= std::min(yModified + 1, static_cast<int>(tiles.size()) - 1); y++)
    {
        for (int x = std::max(xModified - 1, 0); x <= std::min(xModified + 1, static_cast<int>(tiles[0].size()) - 1); x++)
        {
            updateTileFromAdjacent(x, y);
        }
    }

    buildVertexArray();
}

void TileMap::updateTileFromAdjacent(int x, int y)
{
    uint8_t& tile = tiles[y][x];
    if (!isTilePresent(tile))
    {
        // Tile is not filled, so do not update
        return;
    }

    // Reset tile adjacent values and variation
    tile &= 0b10000000;

    if (x > 0)
    {
        tile |= (isTilePresent(tiles[y][x - 1]) << 2);
    }
    if (x < tiles[0].size() - 1)
    {
        tile |= (isTilePresent(tiles[y][x + 1]) << 1);
    }

    if (y > 0)
    {
        tile |= (isTilePresent(tiles[y - 1][x]) << 3);
    }
    if (y < tiles.size() - 1)
    {
        tile |= (isTilePresent(tiles[y + 1][x]));
    }

    // Randomise variation
    uint8_t tileVariation = (rand() % variation) & 0b111;
    tile |= tileVariation << 4;
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
                continue;
            
            sf::Vector2i textureOffset = getTextureOffsetForTile(x, y);
            
            // Add tile to vertex array
            sf::Vertex vertices[4];
            vertices[0].position = sf::Vector2f(x * TILE_SIZE_PIXELS_UNSCALED, y * TILE_SIZE_PIXELS_UNSCALED);
            vertices[1].position = sf::Vector2f(x * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED, y * TILE_SIZE_PIXELS_UNSCALED);
            vertices[3].position = sf::Vector2f(x * TILE_SIZE_PIXELS_UNSCALED, y * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED);
            vertices[2].position = sf::Vector2f(x * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED, y * TILE_SIZE_PIXELS_UNSCALED + TILE_SIZE_PIXELS_UNSCALED);
            vertices[0].texCoords = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 0, static_cast<float>(tilesetOffset.y) + textureOffset.y + 0};
            vertices[1].texCoords = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 16, static_cast<float>(tilesetOffset.y) + textureOffset.y + 0};
            vertices[3].texCoords = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 0, static_cast<float>(tilesetOffset.y) + textureOffset.y + 16};
            vertices[2].texCoords = {static_cast<float>(tilesetOffset.x) + textureOffset.x + 16, static_cast<float>(tilesetOffset.y) + textureOffset.y + 16};

            for (int i = 0; i < 4; i++)
            {
                tileVertexArray.append(vertices[i]);
            }
        }
    }
}