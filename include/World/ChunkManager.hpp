#pragma once

#include <map>
#include <memory>
#include <vector>

#include <World/FastNoiseLite.h>

#include "World/Chunk.hpp"
#include "World/ChunkPosition.hpp"
#include "Core/Camera.hpp"

class ChunkManager
{
    ChunkManager() = delete;

public:
    static void updateChunks(const FastNoiseLite& noise);
    static void drawChunkTerrain(sf::RenderWindow& window);

    static void updateChunksObjects(float dt);

    static WorldObject* getSelectedObject(sf::Vector2i selected_tile);
    static void setObject(sf::Vector2i selected_tile, ObjectType objectType);

    static std::vector<WorldObject*> getChunkObjects();

    // inline static std::map<ChunkPosition, std::unique_ptr<Chunk>>& getChunks() {return chunks;}

private:
    static std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> storedChunks;
    static std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> loadedChunks;

};