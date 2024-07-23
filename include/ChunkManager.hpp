#pragma once

#include <map>
#include <memory>
#include <vector>

#include <FastNoiseLite.h>

#include "Chunk.hpp"
#include "ChunkPosition.hpp"
#include "Camera.hpp"

class ChunkManager
{
    ChunkManager() = delete;

public:
    static void updateChunks(const FastNoiseLite& noise);
    static void drawChunkTerrain(sf::RenderWindow& window);

    static std::vector<WorldObject*> getChunkObjects();

    inline static std::map<ChunkPosition, std::unique_ptr<Chunk>>& getChunks() {return chunks;}

private:
    static std::map<ChunkPosition, std::unique_ptr<Chunk>> chunks;

};