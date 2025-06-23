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

#include "IO/CompressedData.hpp"

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

    inline int getWorldSize() const {return worldSize;}

    void setMapTextureData(const std::vector<uint8_t>& mapTextureData);
    const std::vector<uint8_t>& getMapTextureData() const;

    template <class Archive>
    void save(Archive& ar, const std::uint32_t version) const
    {
        CompressedData compressedMapData(std::vector<char>(mapTextureData.begin(), mapTextureData.end()));

        ar(compressedMapData);
    }

    template <class Archive>
    void load(Archive& ar, const std::uint32_t version)
    {
        CompressedData compressedMapData;

        ar(compressedMapData);

        std::vector<char> mapData = compressedMapData.decompress();
        mapTextureData = std::vector<uint8_t>(mapData.begin(), mapData.end());
    }

private:
    void initTexture();

    std::vector<uint8_t> mapTextureData;
    pl::Texture mapTexture;

    int worldSize;

};

CEREAL_CLASS_VERSION(WorldMap, 1);