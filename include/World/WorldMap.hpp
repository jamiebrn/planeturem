#pragma once

#include <vector>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "World/ChunkPosition.hpp"

#include "GameConstants.hpp"

struct ChunkWorldMapSection
{
    ChunkPosition chunkPosition;
    std::array<std::array<pl::Color, CHUNK_MAP_TILE_SIZE>, CHUNK_MAP_TILE_SIZE> colorGrid;
};

class WorldMap
{
public:
    WorldMap() = default;

    void setSize(int worldSize);

    void setChunkMapSection(const ChunkWorldMapSection& chunkMapSection);

    const pl::Texture& getTexture() const;

private:
    void initTexture();

    std::vector<float> mapTextureData;
    pl::Texture mapTexture;

    int worldSize;

};