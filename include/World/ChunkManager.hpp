#pragma once

#include <map>
#include <memory>
#include <vector>
#include <optional>

#include <World/FastNoise.h>

#include "Core/ResolutionHandler.hpp"
#include "Core/Camera.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/Shaders.hpp"
#include "World/Chunk.hpp"
#include "World/ChunkPosition.hpp"
#include "Entity/Entity.hpp"
#include "Types/TileType.hpp"

// Forward declaration of chunk
class Chunk;
class Entity;

class ChunkManager
{
    // ChunkManager() = delete;

public:
    ChunkManager() = default;

    // Load/unload chunks every frame
    void updateChunks(const FastNoise& noise, int worldSize);

    // Forces a reload of chunks, used when wrapping around world
    void reloadChunks();

    // Drawing functions for chunk terrain
    void drawChunkTerrain(sf::RenderTarget& window, float time);
    void drawChunkWater(sf::RenderTarget& window, float time);


    // -- Objects -- //
    // Update objects in all chunks
    void updateChunksObjects(float dt);

    // Get object at certain world position
    // Returns actual object if object reference is at position requested
    std::optional<BuildableObject>& getChunkObject(ChunkPosition chunk, sf::Vector2i tile);

    // Get tile type from loaded chunks (used in object placement/collisions)
    TileType getLoadedChunkTileType(ChunkPosition chunk, sf::Vector2i tile) const;

    // Get tile type from all generated chunks (used in chunk visual detail generation)
    TileType getChunkTileType(ChunkPosition chunk, sf::Vector2i tile);

    // Whether chunk has been generated: stored or loaded
    bool isChunkGenerated(ChunkPosition chunk);

    // Sets object in chunk at tile
    // Places object references if required
    void setObject(ChunkPosition chunk, sf::Vector2i tile, unsigned int objectType, int worldSize);

    // Deletes object in chunk at tile
    // Deletes object references if required
    void deleteObject(ChunkPosition chunk, sf::Vector2i tile);

    // Creates an object reference object in chunk at tile, with given data
    void setObjectReference(const ChunkPosition& chunk, const ObjectReference& objectReference, sf::Vector2i tile);

    // Tests whether an object can be placed in chunk at tile, taking into account object size etc
    bool canPlaceObject(ChunkPosition chunk, sf::Vector2i tile, unsigned int objectType, int worldSize, const CollisionRect& playerCollisionRect);

    // Tests whether an object can be destroyed, e.g. can't destroy bridge object if player or entity is on it
    bool canDestroyObject(ChunkPosition chunk, sf::Vector2i tile, int worldSize, const CollisionRect& playerCollisionRect);

    // Get all objects in loaded chunks (used for drawing)
    std::vector<WorldObject*> getChunkObjects();


    // -- Entities -- //
    // Update all entities in loaded chunks
    void updateChunksEntities(float dt, int worldSize);

    // Handle moving of entity from one chunk to another chunk
    void moveEntityToChunkFromChunk(std::unique_ptr<Entity> entity, ChunkPosition newChunk);

    // Get entity selected at cursor position (IN WORLD SPACE), if any
    Entity* getSelectedEntity(ChunkPosition chunk, sf::Vector2f cursorPos);

    // Get all entities in loaded chunks (used for drawing)
    std::vector<WorldObject*> getChunkEntities();


    // -- Collision -- //
    // Collision test functions for player, entity etc
    // against collision rects in all loaded chunks
    bool collisionRectChunkStaticCollisionX(CollisionRect& collisionRect, float dx);
    bool collisionRectChunkStaticCollisionY(CollisionRect& collisionRect, float dy);

    // Gets all collision rects from loaded chunks
    // DONT USE FOR GENERAL COLLISION CHECKING - use collisionRectChunkStaticCollision functions
    // to avoid copying pointers redundantly
    std::vector<CollisionRect*> getChunkCollisionRects();


    // Misc
    inline int getLoadedChunkCount() {return loadedChunks.size();}
    inline int getGeneratedChunkCount() {return loadedChunks.size() + storedChunks.size();}

    // Finds valid spawn position for player i.e. no water
    // Waterless area size checks for chunks +- waterlessAreaSize
    // e.g. size 1 will check 3x3 area, size 2 will check 5x5 etc
    sf::Vector2f findValidSpawnPosition(int waterlessAreaSize, const FastNoise& noise, int worldSize);

    // Gets levels of nearby crafting stations
    // Search area grows with one extra tile in each direction per 1 increase
    // E.g. 0 search area searches only player tile, 1 searches 3x3 area around player, 2 searches 5x5 etc
    std::unordered_map<std::string, int> getNearbyCraftingStationLevels(ChunkPosition playerChunk,
                                                                        sf::Vector2i playerTile,
                                                                        int searchArea,
                                                                        int worldSize);

    // Used to calculate chunk and tile positions from an offset value, from another chunk and tile
    // Correct for offsets < worldSize * CHUNK_TILE_SIZE
    static std::pair<ChunkPosition, sf::Vector2i> getChunkTileFromOffset(ChunkPosition chunk, sf::Vector2i tile, int xOffset, int yOffset, int worldSize);

private:
    void generateChunk(const ChunkPosition& chunkPosition,
                       const FastNoise& noise,
                       int worldSize,
                       bool putInLoaded = true,
                       std::optional<sf::Vector2f> positionOverride = std::nullopt);

private:
    std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> storedChunks;
    std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> loadedChunks;

};