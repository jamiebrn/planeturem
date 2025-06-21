#include "World/WorldMap.hpp"

void WorldMap::setSize(int worldSize)
{
    mapTextureData.clear();

    // Reserve 3 floats for each map tile
    mapTextureData.resize(CHUNK_MAP_TILE_SIZE * CHUNK_MAP_TILE_SIZE * worldSize * worldSize * 3, 0.0f);

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
            mapTextureData[startIdx + yIdxOffset + x * 3] = chunkMapSection.colorGrid[y][x].r / 255.0f;
            mapTextureData[startIdx + yIdxOffset + x * 3 + 1] = chunkMapSection.colorGrid[y][x].g / 255.0f;
            mapTextureData[startIdx + yIdxOffset + x * 3 + 2] = chunkMapSection.colorGrid[y][x].b / 255.0f;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, chunkMapSection.chunkPosition.x * CHUNK_MAP_TILE_SIZE,
            chunkMapSection.chunkPosition.y * CHUNK_MAP_TILE_SIZE + y, CHUNK_MAP_TILE_SIZE, 1,
            GL_RGB, GL_FLOAT, &mapTextureData[startIdx + yIdxOffset]);
    }
}

const pl::Texture& WorldMap::getTexture() const
{
    return mapTexture;
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
    }

    mapTexture.overwriteData(CHUNK_MAP_TILE_SIZE * worldSize, CHUNK_MAP_TILE_SIZE * worldSize, mapTextureData.data(), GL_RGB, GL_FLOAT);
}