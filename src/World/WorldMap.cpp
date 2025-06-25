#include "World/WorldMap.hpp"

void WorldMap::setSize(int worldSize)
{
    mapTextureData.clear();

    // Reserve 3 floats for each map tile
    mapTextureData.resize(CHUNK_MAP_TILE_SIZE * CHUNK_MAP_TILE_SIZE * worldSize * worldSize * 3, 0);

    this->worldSize = worldSize;

    initTexture();
}

void WorldMap::setChunkMapSection(const ChunkWorldMapSection& chunkMapSection)
{
    mapTexture.use();

    int startIdx = chunkMapSection.chunkPosition.x * CHUNK_MAP_TILE_SIZE * 3 + chunkMapSection.chunkPosition.y * CHUNK_MAP_TILE_SIZE * CHUNK_MAP_TILE_SIZE * worldSize * 3;

    for (int y = 0; y < CHUNK_MAP_TILE_SIZE; y++)
    {
        int yIdxOffset = y * CHUNK_MAP_TILE_SIZE * worldSize * 3;
        for (int x = 0; x < CHUNK_MAP_TILE_SIZE; x++)
        {
            mapTextureData[startIdx + yIdxOffset + x * 3] = static_cast<uint8_t>(chunkMapSection.colorGrid[y][x].r);
            mapTextureData[startIdx + yIdxOffset + x * 3 + 1] = static_cast<uint8_t>(chunkMapSection.colorGrid[y][x].g);
            mapTextureData[startIdx + yIdxOffset + x * 3 + 2] = static_cast<uint8_t>(chunkMapSection.colorGrid[y][x].b);
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, chunkMapSection.chunkPosition.x * CHUNK_MAP_TILE_SIZE,
            chunkMapSection.chunkPosition.y * CHUNK_MAP_TILE_SIZE + y, CHUNK_MAP_TILE_SIZE, 1,
            GL_RGB, GL_UNSIGNED_BYTE, &mapTextureData[startIdx + yIdxOffset]);
    }

    chunksDiscovered.insert(chunkMapSection.chunkPosition);
}

const pl::Texture& WorldMap::getTexture() const
{
    return mapTexture;
}

void WorldMap::setMapTextureData(const std::vector<uint8_t>& mapTextureData)
{
    this->mapTextureData = mapTextureData;
    initTexture();
}

const std::vector<uint8_t>& WorldMap::getMapTextureData() const
{
    return mapTextureData;
}

void WorldMap::initTexture()
{
    if (mapTexture.getID() == 0)
    {
        GLuint textureId;
        glGenTextures(1, &textureId);
        pl::Texture::bindTextureID(textureId, 0);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        mapTexture.setFromAllocated(textureId, CHUNK_MAP_TILE_SIZE * worldSize, CHUNK_MAP_TILE_SIZE * worldSize);
        mapTexture.setLinearFilter(false);
        mapTexture.setTextureRepeat(true);
    }

    mapTexture.overwriteData(CHUNK_MAP_TILE_SIZE * worldSize, CHUNK_MAP_TILE_SIZE * worldSize, mapTextureData.data(), GL_RGB, GL_UNSIGNED_BYTE);
}

bool WorldMap::isChunkDiscovered(ChunkPosition chunkPosition) const
{
    return chunksDiscovered.contains(chunkPosition);
}