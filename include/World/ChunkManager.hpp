#pragma once

#include <map>
#include <memory>
#include <vector>

#include <World/FastNoiseLite.h>

#include "Core/ResolutionHandler.hpp"
#include "Core/Camera.hpp"
#include "Core/CollisionRect.hpp"
#include "World/Chunk.hpp"
#include "World/ChunkPosition.hpp"

// Forward declaration of chunk
class Chunk;

class ChunkManager
{
    ChunkManager() = delete;

public:
    static void updateChunks(const FastNoiseLite& noise);
    static void drawChunkTerrain(sf::RenderWindow& window);

    static void updateChunksObjects(float dt);

    static BuildableObject* getSelectedObject(ChunkPosition chunk, sf::Vector2i tile);
    // static bool interactWithObject(sf::Vector2i selected_tile);

    static void setObject(ChunkPosition chunk, sf::Vector2i tile, unsigned int objectType);
    static void deleteObject(ChunkPosition chunk, sf::Vector2i tile);

    static unsigned int getObjectTypeFromObjectReference(const ObjectReference& objectReference);
    static void setObjectReference(const ChunkPosition& chunk, const ObjectReference& objectReference, sf::Vector2i tile);

    static bool canPlaceObject(ChunkPosition chunk, sf::Vector2i tile, unsigned int objectType);

    static std::vector<WorldObject*> getChunkObjects();
    static std::vector<std::unique_ptr<CollisionRect>> getChunkCollisionRects();

    // inline static std::map<ChunkPosition, std::unique_ptr<Chunk>>& getChunks() {return chunks;}

private:
    static std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> storedChunks;
    static std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> loadedChunks;

};