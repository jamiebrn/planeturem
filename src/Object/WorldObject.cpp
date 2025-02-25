#include "Object/WorldObject.hpp"

sf::Vector2f WorldObject::getPosition() const
{
    return position;
}

void WorldObject::setPosition(sf::Vector2f pos)
{
    position = pos;
}

ChunkPosition WorldObject::getChunkInside(sf::Vector2f position, int worldSize)
{
    ChunkPosition chunk;
    chunk.x = ((static_cast<int>(std::floor(position.x / (CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED))) % worldSize) + worldSize) % worldSize;
    chunk.y = ((static_cast<int>(std::floor(position.y / (CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED))) % worldSize) + worldSize) % worldSize;
    return chunk;
}

ChunkPosition WorldObject::getChunkInside(int worldSize) const
{
    return getChunkInside(position, worldSize);
}

sf::Vector2i WorldObject::getChunkTileInside(sf::Vector2f position, int worldSize)
{
    int worldTotalSize = worldSize * CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED;

    sf::Vector2f wrappedPosition;
    wrappedPosition.x = (static_cast<int>(position.x) % worldTotalSize + worldTotalSize) % worldTotalSize;
    wrappedPosition.y = (static_cast<int>(position.y) % worldTotalSize + worldTotalSize) % worldTotalSize;

    sf::Vector2i chunkTile;
    chunkTile.x = static_cast<int>((static_cast<int>(wrappedPosition.x / TILE_SIZE_PIXELS_UNSCALED) % static_cast<int>(CHUNK_TILE_SIZE)) + CHUNK_TILE_SIZE) % static_cast<int>(CHUNK_TILE_SIZE);
    chunkTile.y = static_cast<int>((static_cast<int>(wrappedPosition.y / TILE_SIZE_PIXELS_UNSCALED) % static_cast<int>(CHUNK_TILE_SIZE)) + CHUNK_TILE_SIZE) % static_cast<int>(CHUNK_TILE_SIZE);
    return chunkTile;
}

sf::Vector2i WorldObject::getChunkTileInside(int worldSize) const
{
    return getChunkTileInside(position, worldSize);
}

ObjectReference WorldObject::getObjectReferenceFromPosition(sf::Vector2f position, int worldSize)
{
    ObjectReference objectReference;
    objectReference.chunk = getChunkInside(position, worldSize);
    objectReference.tile = getChunkTileInside(position, worldSize);
    return objectReference;
}

ObjectReference WorldObject::getThisObjectReference(int worldSize) const
{
    return getObjectReferenceFromPosition(position, worldSize);
}

sf::Vector2i WorldObject::getTileInside(sf::Vector2f position)
{
    sf::Vector2i tile;
    tile.x = std::floor(position.x / TILE_SIZE_PIXELS_UNSCALED);
    tile.y = std::floor(position.y / TILE_SIZE_PIXELS_UNSCALED);
    return tile;
}

sf::Vector2i WorldObject::getTileInside() const
{
    return getTileInside(position);
}

sf::Vector2i WorldObject::getWorldTileInside(sf::Vector2f position, int worldSize)
{
    int worldTileSize = worldSize * static_cast<int>(CHUNK_TILE_SIZE);
    sf::Vector2i tile = getTileInside(position);
    tile.x = (tile.x % worldTileSize + worldTileSize) % worldTileSize;
    tile.y = (tile.y % worldTileSize + worldTileSize) % worldTileSize;
    return tile;
}

sf::Vector2i WorldObject::getWorldTileInside(int worldSize) const
{
    return getWorldTileInside(position, worldSize);
}

// Assumes on water
float WorldObject::getWaterBobYOffset(sf::Vector2f position, int worldSize, float gameTime)
{    
    ChunkPosition chunk = getChunkInside(position, worldSize);
    sf::Vector2i tile = getChunkTileInside(position, worldSize);

    static constexpr float xWavelength = 0.9f;
    static constexpr float yWavelength = 0.7f;
    static constexpr float frequency = 3.0f;

    int xPos = chunk.x * CHUNK_TILE_SIZE + tile.x;
    int yPos = chunk.y * CHUNK_TILE_SIZE + tile.y;

    // return -std::pow(std::sin(xPos * xWavelength + yPos * yWavelength + gameTime * frequency), 2.0f);
    return std::sin(xPos * xWavelength + yPos * yWavelength + gameTime * frequency) - 1.0f;
}

float WorldObject::getWaterBobYOffset(int worldSize, float gameTime) const
{
    if (!onWater)
        return 0.0f;

    return getWaterBobYOffset(position, worldSize, gameTime);   
}


int WorldObject::getDrawLayer() const
{
    return drawLayer;
}