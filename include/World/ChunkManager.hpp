#pragma once

#include <map>
#include <memory>
#include <vector>
#include <optional>

// #include <World/FastNoiseLite.h>
#include <World/FastNoise.h>

#include "Core/ResolutionHandler.hpp"
#include "Core/Camera.hpp"
#include "Core/CollisionRect.hpp"
#include "World/Chunk.hpp"
#include "World/ChunkPosition.hpp"
#include "Types/TileType.hpp"

// Forward declaration of chunk
class Chunk;

class ChunkManager
{
    // ChunkManager() = delete;

public:
    ChunkManager() = default;

    void updateChunks(const FastNoise& noise, int worldSize);

    void drawChunkTerrain(sf::RenderWindow& window, float time);
    void drawChunkWater(sf::RenderWindow& window, float time);

    void updateChunksObjects(float dt);

    std::optional<BuildableObject>& getChunkObject(ChunkPosition chunk, sf::Vector2i tile);
    // bool interactWithObject(sf::Vector2i selected_tile);
    TileType getChunkTileType(ChunkPosition chunk, sf::Vector2i tile) const;

    void setObject(ChunkPosition chunk, sf::Vector2i tile, unsigned int objectType, int worldSize);
    void deleteObject(ChunkPosition chunk, sf::Vector2i tile);

    // unsigned int getObjectTypeFromObjectReference(const ObjectReference& objectReference) const;
    void setObjectReference(const ChunkPosition& chunk, const ObjectReference& objectReference, sf::Vector2i tile);

    bool canPlaceObject(ChunkPosition chunk, sf::Vector2i tile, unsigned int objectType, int worldSize);

    std::vector<WorldObject*> getChunkObjects();
    std::vector<std::unique_ptr<CollisionRect>> getChunkCollisionRects();

    // inline std::map<ChunkPosition, std::unique_ptr<Chunk>>& getChunks() {return chunks;}

private:
    std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> storedChunks;
    std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> loadedChunks;

};