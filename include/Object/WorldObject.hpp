#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>

#include "World/ChunkPosition.hpp"

class WorldObject
{
public:
    WorldObject(sf::Vector2f position) : position(position) {}

    // General world object functionality

    inline sf::Vector2f getPosition() {return position;}
    inline void setPosition(sf::Vector2f pos) {position = pos;}

    inline ChunkPosition getChunkInside(int worldSize)
    {
        ChunkPosition chunk;
        chunk.x = ((static_cast<int>(std::floor(position.x / (CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED))) % worldSize) + worldSize) % worldSize;
        chunk.y = ((static_cast<int>(std::floor(position.y / (CHUNK_TILE_SIZE * TILE_SIZE_PIXELS_UNSCALED))) % worldSize) + worldSize) % worldSize;
        return chunk;
    }

    inline sf::Vector2i getChunkTileInside()
    {
        sf::Vector2i chunkTile;
        chunkTile.x = static_cast<int>((static_cast<int>(position.x / TILE_SIZE_PIXELS_UNSCALED) % static_cast<int>(CHUNK_TILE_SIZE)) + CHUNK_TILE_SIZE) % static_cast<int>(CHUNK_TILE_SIZE);
        chunkTile.y = static_cast<int>((static_cast<int>(position.y / TILE_SIZE_PIXELS_UNSCALED) % static_cast<int>(CHUNK_TILE_SIZE)) + CHUNK_TILE_SIZE) % static_cast<int>(CHUNK_TILE_SIZE);
        return chunkTile;
    }

    inline int getDrawLayer() {return drawLayer;}

    // Overriden by inherited classes (specific)
    virtual void draw(sf::RenderTarget& window, float dt, const sf::Color& color) = 0;

protected:
    sf::Vector2f position;
    int drawLayer = 0;

};