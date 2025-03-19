#pragma once

#include <map>
#include <memory>
#include <vector>
#include <optional>
#include <set>
#include <chrono>

#include <World/FastNoise.h>

#include "Core/ResolutionHandler.hpp"
#include "Core/Camera.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/Shaders.hpp"
#include "Core/SpriteBatch.hpp"
#include "World/Chunk.hpp"
#include "World/ChunkPosition.hpp"
#include "World/TileMap.hpp"
#include "Entity/Entity.hpp"
#include "Entity/HitRect.hpp"
#include "Types/TileType.hpp"
#include "Player/InventoryData.hpp"
#include "Player/ItemPickup.hpp"

#include "World/Chunk.hpp"
#include "World/ChunkPOD.hpp"
#include "World/ChunkViewRange.hpp"
#include "World/PathfindingEngine.hpp"

#include "Data/typedefs.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"

#include "Network/PacketData/PacketDataWorld/PacketDataChunkDatas.hpp"
#include "Network/PacketData/PacketDataWorld/PacketDataEntities.hpp"

// Forward declaration
class Game;
class Entity;
class ProjectileManager;

class ChunkManager
{
    // ChunkManager() = delete;

public:
    ChunkManager() = default;

    void setSeed(int seed);
    int getSeed() const;

    void setPlanetType(PlanetType planetType);

    // Delete all chunks (used when switching planets)
    void deleteAllChunks();

    // Load chunks every frame
    // Returns true if any chunks loaded
    bool updateChunks(Game& game, std::optional<sf::Vector2f> localPlayerPos, const std::vector<ChunkViewRange>& chunkViewRanges,
            bool isClient = false, std::vector<ChunkPosition>* chunksToRequestFromHost = nullptr);
    
    bool unloadChunksOutOfView(const std::vector<ChunkViewRange>& chunkViewRanges);

    // Forces a reload of chunks, used when wrapping around world
    void reloadChunks(ChunkViewRange chunkViewRange);

    // Used when a chunk is required to have no structure generated to ensure no collision interference
    // E.g. when travelling to a new planet and rocket is placed, rocket may be placed inside structure (if there is one)
    void regenerateChunkWithoutStructure(ChunkPosition chunk, Game& game);

    // Drawing functions for chunk terrain
    void drawChunkTerrain(sf::RenderTarget& window, SpriteBatch& spriteBatch, const Camera& camera, float time);
    void drawChunkWater(sf::RenderTarget& window, const Camera& camera, float time);

    // Returns a pointer to the chunk with ChunkPosition key
    // Chunk can be in loaded chunks or stored chunks
    Chunk* getChunk(ChunkPosition chunk);

    // Whether chunk has been generated: stored or loaded
    bool isChunkGenerated(ChunkPosition chunk) const;

    const BiomeGenData* getChunkBiome(ChunkPosition chunk);


    // -- Tilemap -- //
    TileMap* getChunkTileMap(ChunkPosition chunk, int tileMap);

    // Returns tilemaps which have been modified
    // IF GRAPHIC UPDATE IS DISABLED, ENSURE TO CALL performChunkSetTileUpdate AFTER
    std::set<int> setChunkTile(ChunkPosition chunk, int tileMap, sf::Vector2i position, bool tileGraphicUpdate = true);

    // Sets tilemap tiles for the current tile for background, depending on adjacent tiles
    // Also sets adjacent tilemap tiles to have backing for the current tile
    // Returns set of tilemaps modified
    std::set<int> setBackgroundAdjacentTilesForTile(ChunkPosition chunk, int tileMap, sf::Vector2i position);

    // Updates chunks edge tiles, adjacent to a chunk
    void updateAdjacentChunkTiles(ChunkPosition chunk, int tileMap);

    // Updates chunk edge tiles adjacent to chunk, for each 4 adjacent chunks to a centre chunk
    // Does not update centre chunk
    // DO NOT CONFUSE WITH updateAdjacentChunkTiles
    void updateAdjacentChunkAdjacentChunkTiles(ChunkPosition centreChunk, int tileMap);

    // Called after setting a tile
    // Must be called if graphic update is disabled when setting tiles
    // Useful when placing many tiles, e.g. generating a chunk
    void performChunkSetTileUpdate(ChunkPosition chunk, std::set<int> tileMapsModified);

    // Get tile type from loaded chunks (used in object placement/collisions)
    int getLoadedChunkTileType(ChunkPosition chunk, sf::Vector2i tile) const;

    // Get tile type from all generated chunks (used in chunk visual detail generation)
    int getChunkTileType(ChunkPosition chunk, sf::Vector2i tile) const;

    // Gets tile type from all generated chunks
    // Falls back to generation prediction if chunk has not yet been generated
    int getChunkTileTypeOrPredicted(ChunkPosition chunk, sf::Vector2i tile);



    // -- Objects -- //
    // Update objects in all chunks
    void updateChunksObjects(Game& game, float dt);

    // Get object at certain world position
    // Returns actual object if object reference is at position requested
    template <class T = BuildableObject>
    T* getChunkObject(ChunkPosition chunk, sf::Vector2i tile);

    // Sets object in chunk at tile
    // Places object references if required
    void setObject(ChunkPosition chunk, sf::Vector2i tile, ObjectType objectType, Game& game);

    // Deletes object in chunk at tile
    // Deletes object references if required
    void deleteObject(ChunkPosition chunk, sf::Vector2i tile);

    // Deletes single object (does not delete object references)
    // Used in deleteObject function
    void deleteSingleObject(ChunkPosition chunk, sf::Vector2i tile);

    // Creates an object reference object in chunk at tile, with given data
    void setObjectReference(const ChunkPosition& chunk, const ObjectReference& objectReference, sf::Vector2i tile);

    // Tests whether an object can be placed in chunk at tile, taking into account object size etc
    bool canPlaceObject(ChunkPosition chunk, sf::Vector2i tile, ObjectType objectType, const CollisionRect& playerCollisionRect);

    // Tests whether an object can be destroyed, e.g. can't destroy bridge object if player or entity is on it
    bool canDestroyObject(ChunkPosition chunk, sf::Vector2i tile, const CollisionRect& playerCollisionRect);

    // Get all objects in loaded chunks (used for drawing)
    std::vector<WorldObject*> getChunkObjects(ChunkViewRange chunkViewRange);


    // -- Entities -- //
    // Update all entities in loaded chunks
    void updateChunksEntities(float dt, ProjectileManager& projectileManager, Game& game, bool networkUpdateOnly);

    // Damages any entities hit by any hit rect
    void testChunkEntityHitCollision(const std::vector<HitRect>& hitRects, Game& game, float gameTime);

    // Handle moving of entity from one chunk to another chunk
    void moveEntityToChunkFromChunk(std::unique_ptr<Entity> entity, ChunkPosition newChunk);

    // Get entity selected at cursor position (IN WORLD SPACE), if any
    Entity* getSelectedEntity(ChunkPosition chunk, sf::Vector2f cursorPos);

    // Get all entities in loaded chunks (used for drawing)
    std::vector<WorldObject*> getChunkEntities(ChunkViewRange chunkViewRange);

    int getChunkEntitySpawnCooldown(ChunkPosition chunk);

    void resetChunkEntitySpawnCooldown(ChunkPosition chunk);

    PacketDataEntities getEntityPacketDatas(ChunkViewRange chunkViewRange);
    void loadEntityPacketDatas(const PacketDataEntities& entityPacketDatas);


    // -- Item pickups -- //
    // Adds item pickup automatically to correct chunk
    // If chunk is not loaded / generated, item pickup will be discarded
    std::optional<ItemPickupReference> addItemPickup(const ItemPickup& itemPickup, std::optional<uint64_t> idOverride = std::nullopt);

    // Check chunks in 3x3 area around player for colliding item pickups
    // Returns first item pickup collided with (if any)
    // Call deleteItemPickup if successfully picked up
    std::optional<ItemPickupReference> getCollidingItemPickup(const CollisionRect& playerCollision, float gameTime);
    void deleteItemPickup(const ItemPickupReference& itemPickupReference);

    std::vector<WorldObject*> getItemPickups(ChunkViewRange chunkViewRange);


    // -- Collision -- //
    // Collision test functions for player, entity etc
    // against collision rects in all loaded chunks
    bool collisionRectChunkStaticCollisionX(CollisionRect& collisionRect, float dx) const;
    bool collisionRectChunkStaticCollisionY(CollisionRect& collisionRect, float dy) const;

    // Gets all collision rects from loaded chunks
    // DONT USE FOR GENERAL COLLISION CHECKING - use collisionRectChunkStaticCollision functions
    // to avoid copying pointers redundantly
    std::vector<CollisionRect*> getChunkCollisionRects();


    // -- Land -- //
    // Check whether land can be placed at position
    bool canPlaceLand(ChunkPosition chunk, sf::Vector2i tile);

    // Place land at position
    void placeLand(ChunkPosition chunk, sf::Vector2i tile);


    // -- Structures -- //

    // Returns chunk containing structure entered, if any
    std::optional<ChunkPosition> isPlayerInStructureEntrance(sf::Vector2f playerPos);


    // Save / load
    std::vector<ChunkPOD> getChunkPODs();
    void loadFromChunkPODs(const std::vector<ChunkPOD>& pods, Game& game);


    // -- Networking --

    // Get chunk data to send over network
    // Generates chunk minimally (as if loaded from POD and out of view) if does not exist
    PacketDataChunkDatas::ChunkData getChunkDataAndGenerate(ChunkPosition chunk, Game& game);

    void setChunkData(const PacketDataChunkDatas::ChunkData& chunkData, Game& game);
    

    // Misc
    inline int getLoadedChunkCount() const {return loadedChunks.size();}
    inline int getGeneratedChunkCount() const {return loadedChunks.size() + storedChunks.size();}
    inline int getWorldSize() const {return worldSize;}
    inline const FastNoise& getBiomeNoise() const {return biomeNoise;}
    inline const FastNoise& getHeightNoise() const {return heightNoise;}
    inline PlanetType getPlanetType() const {return planetType;}
    inline const PathfindingEngine& getPathfindingEngine() const {return pathfindingEngine;}

    // Finds valid spawn position for player i.e. no water
    // Waterless area size checks for chunks +- waterlessAreaSize
    // e.g. size 1 will check 3x3 area, size 2 will check 5x5 etc
    ChunkPosition findValidSpawnChunk(int waterlessAreaSize);

    // Gets levels of nearby crafting stations
    // Search area grows with one extra tile in each direction per 1 increase
    // E.g. 0 search area searches only player tile, 1 searches 3x3 area around player, 2 searches 5x5 etc
    std::unordered_map<std::string, int> getNearbyCraftingStationLevels(ChunkPosition playerChunk,
                                                                        sf::Vector2i playerTile,
                                                                        int searchArea);
    
    // Translate position relative to player position and world size, to make position closest possible to player
    // Provides planet / wraparound effect
    sf::Vector2f translatePositionAroundWorld(sf::Vector2f position, sf::Vector2f originPosition) const;

    // Used to calculate chunk and tile positions from an offset value, from another chunk and tile
    // Correct for offsets < worldSize * CHUNK_TILE_SIZE
    static std::pair<ChunkPosition, sf::Vector2i> getChunkTileFromOffset(ChunkPosition chunk, sf::Vector2i tile, int xOffset, int yOffset, int worldSize);

    // Returns rectangle area of size containing chunks in view
    // Used in lighting calculations
    static sf::Vector2i getChunksSizeInView(const Camera& camera);
    static sf::Vector2f topLeftChunkPosInView(const Camera& camera);

private:
    // Generates a chunk and stores it
    Chunk* generateChunk(const ChunkPosition& chunkPosition,
                       Game& game,
                       bool putInLoaded = true,
                       std::optional<sf::Vector2f> positionOverride = std::nullopt);

    void clearUnmodifiedStoredChunks();

private:
    std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> storedChunks;
    std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> loadedChunks;

    std::unordered_map<ChunkPosition, const BiomeGenData*> chunkBiomeCache;

    static constexpr int MAX_CHUNK_ENTITY_SPAWN_COOLDOWN = 60000;
    std::unordered_map<ChunkPosition, uint64_t> chunkLastEntitySpawnTime;

    FastNoise heightNoise;
    FastNoise biomeNoise;
    FastNoise riverNoise;

    int worldSize = 1;
    PlanetType planetType = 0;

    int seed = 0;

    PathfindingEngine pathfindingEngine;

};

template <class T>
inline T* ChunkManager::getChunkObject(ChunkPosition chunk, sf::Vector2i tile)
{
    Chunk* chunkPtr = getChunk(chunk);

    // Chunk does not exist
    if (!chunkPtr)
    {
        return nullptr;
    }
    
    // Get object from chunk
    // Get as BuildableObject as could be object reference
    BuildableObject* selectedObject = chunkPtr->getObject(tile);

    if (!selectedObject)
        return nullptr;

    // Test if object is object reference object, to then get the actual object
    if (selectedObject->isObjectReference())
    {
        const ObjectReference& objectReference = selectedObject->getObjectReference().value();
        return getChunkObject<T>(objectReference.chunk, objectReference.tile);
    }

    // Return casted object
    return chunkPtr->getObject<T>(tile);
}